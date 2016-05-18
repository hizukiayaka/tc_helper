#!/bin/ash
. /lib/functions/network.sh;

DEBUG=1
IF_WAN_NAME='wan'
RATE_UP=600
RATE_DOWN=14000
RATE_TO_QUANTUM=20
OVERHEAD=64
MPU=128

# ---- Helper functions: ----
config_load_package()
{
	PACKAGE_NAME=${1};
	XCOSTC_CONFIG=$(uci export ${PACKAGE_NAME});

	if [[ $? -ne 0 || -z "${XCOSTC_CONFIG}" ]]; then
		return 0;
	fi

#	XCOSTC_SECTION=$(uci get xcostc.@xcostc[${2}])
#	if [[ $? -ne 0 ]]; then
#		break;
#	fi
	return 1;
}

rate()
{
    RATE=0
    R_RATE=$1
    R_NUMBER=`echo "$R_RATE" | sed -e "s/[^0-9]//g"`
    R_UNIT=`echo "$R_RATE" | sed -e "s/[0-9]//g"`

    if [ "$R_UNIT" == "" ];
    then
        R_UNIT="kbit"
    fi

    # Let's see which unit we have...
    if [ "$R_UNIT" == "kbps" ]
    then
        R_RATE=$(($R_NUMBER * 1024))
    elif [ "$R_UNIT" == "mbps" ]
    then
        R_RATE=$(($R_NUMBER * 1024 * 1024))
    elif [ "$R_UNIT" == "mbit" ]
    then
        R_RATE=$(($R_NUMBER * 1024 * 1024 / 8))
    elif [ "$R_UNIT" == "kbit" ]
    then
        R_RATE=$(($R_NUMBER * 1024 / 8))
    elif [ "$R_UNIT" == "bps" ]
    then
        R_RATE=$R_NUMBER
    else
        echo "Unknown unit '$R_UNIT'. I only know mbps, mbit, kbit, bps."
    fi

    RATE="$R_RATE"
}

load_modules()
{
    insmod cls_fw >&- 2>&-
    insmod sch_hfsc >&- 2>&-
    insmod sch_sfq >&- 2>&-
    insmod sch_red >&- 2>&-
    insmod sch_htb >&- 2>&-
    insmod sch_prio >&- 2>&-
    insmod ipt_multiport >&- 2>&-
    insmod ipt_CONNMARK >&- 2>&-
    insmod ipt_length >&- 2>&-
	insmod ifb >&- 2>&-
}

reset()
{
	MARK_TABLE="XCOS_TC_MARK"
	network_get_physdev IF_WAN_NAME_tmp wan;
	IF_WAN_NAME=${IF_WAN_NAME_tmp};

    ifconfig ${IF_WAN_NAME} up txqueuelen 5 >&- 2>&-
#    ifconfig imq0 up txqueuelen 5 >&- 2>&-
    tc qdisc del dev ${IF_WAN_NAME} root >&- 2>&-
    tc qdisc del dev ifb0 root >&- 2>&-
#    iptables -t mangle -F
	iptables -t mangle -F PREROUTING;
    iptables -t mangle -N ${MARK_TABLE} >&- 2>&-

    # For acquiring DSL data from SpeedTouch modem that is connected to WAN:
    # ifconfig eth0.1 192.168.1.42
}

tc_class_add_htb()
{
    t_parent=$1
    t_classid=$2
    t_rate=$3
    t_ceil=$4

    t_quantum=$(($t_rate/$RATE_TO_QUANTUM))

    if [ $t_quantum -lt 1500 ];
    then
        if [ "$DEBUG" == "1" ]
        then
            echo $DEVICE $t_classid quantum $t_quantum small: increasing to 1500
        fi

        t_quantum=1500
    fi

    if [ $t_quantum -gt 60000 ];
    then
        if [ "$DEBUG" == "1" ]
        then
            echo $DEVICE $t_classid quantum $t_quantum big: reducing to 60000
        fi

        t_quantum=60000
    fi

    if [ "$DEBUG" == "1" ]
    then
        echo tc class add dev $DEVICE parent $t_parent classid $t_classid htb rate "$t_rate"bps ceil "$t_ceil"bps quantum "$t_quantum" overhead $OVERHEAD mpu $MPU
    fi

    tc class add dev $DEVICE parent $t_parent classid $t_classid htb rate "$t_rate"bps ceil "$t_ceil"bps quantum "$t_quantum" overhead $OVERHEAD mpu $MPU
}

# ---- Generic: ----
qos_per_user()
{
	DEVICE="$1"
	RATE="$2"
	CEIL="$3"
	IP4_ADDR="$4"
	MASK="$5"

	tc_class_add_htb 1:1 1:${MASK} ${RATE} ${CELL}

	tc filter add dev $DEVICE \
	parent 1: prio 1 protocol ip handle ${MASK} fw flowid 1:${MASK}

}

qos_per_user_filter()
{
	DEVICE="$1"
	RATE="$2"
	CEIL="$3"
	IP4_ADDR="$4"
	MASK="$5"

	tc_class_add_htb 1:1 1:${MASK} ${RATE} ${CELL}

	tc filter add dev $DEVICE \ 
	parent 1: prio 1 protocol ip u32 match ip dst "${IP4_ADDR}"/32 \
	flowid 1:${MASK}

}

mangle_per_user()
{
	IP4_ADDR="$4"
	MARK="$5"

	MARK_TABLE="XCOS_TC_MARK"

	iptables -t mangle -A ${MARK_TABLE} -s ${IP4_ADDR} \
		-j MARK --set-mark ${MARK}
#	iptables -t mangle -A ${MARK_TABLE} -d ${IP4_ADDR} \
#		-j MARK --set-mark ${MARK}
}

config_apply_section()
{
	ret=$(printf "%s\n" ${1} | grep -E '^[0-9]+$')
	if [[ ! ${ret} ]]; then
#	if 	[[ ! ${1} =~ ^-?[0-9]+$ ]]; then
		return 0;
	fi
	
	DEVIFNAME=$(uci get xcostc.@xcostc[${1}].dev)
	IP4_ADDR_START=$(uci get xcostc.@xcostc[${1}].start)
	IP4_ADDR_END=$(uci get xcostc.@xcostc[${1}].end)
	IP4_WHITE_LIST=$(uci get xcostc.@xcostc[${1}].white_ip)
    UPLOAD_RATE="$(uci get xcostc.@xcostc[0].upload_rate)"
    TOTAL_RATE="$(uci get xcostc.@xcostc[0].total_rate)"
	CALL_FUNCTION=${2}


	#FIXME only work on netmask /24
	network_get_ipaddr IP4_ADDR ${DEVIFNAME}

	IP4_ADDR_PREFIX=${IP4_ADDR%\.*}

	for i in $(seq ${IP4_ADDR_START} ${IP4_ADDR_END}); do
		IP=$(printf "%s.%d" "${IP4_ADDR_PREFIX}" "${i}")

		${CALL_FUNCTION} "${DEVIFNAME}" "${UPLOAD_RATE}" \
		"${TOTAL_RATE}" "${IP}" "${i}"
	done;
}


mangle()
{
	MARK_TABLE="XCOS_TC_MARK"
	iptables -t mangle -I PREROUTING -j ${MARK_TABLE};
	config_apply_section 0 mangle_per_user
}

qos()
{
    if [ "$DEBUG" == "1" ]
    then
        echo --------
    fi

    DEVICE=$1
    rate $2
    TOTAL_RATE="$(uci get xcostc.@xcostc[0].total_rate)"

    tc qdisc add dev $DEVICE root handle 1: htb default 30 r2q 5
    tc_class_add_htb 1: 1:1 $TOTAL_RATE $TOTAL_RATE

    if [ "$DEBUG" == "1" ]
    then
        echo --------
    fi

	config_apply_section 0 qos_per_user_filter
}

qos_filter()
{
    if [ "$DEBUG" == "1" ]
    then
        echo --------
    fi

    DEVICE=$1
    rate $2

    TOTAL_RATE="$(uci get xcostc.@xcostc[0].total_rate)"
    tc qdisc add dev $DEVICE root handle 1: htb default 30 r2q 5
    tc_class_add_htb 1: 1:1 $TOTAL_RATE $TOTAL_RATE

    if [ "$DEBUG" == "1" ]
    then
        echo --------
    fi

	config_apply_section 0 qos_per_user_filter

	network_get_physdev IF_LAN_NAME lan;
	network_get_ipaddr IP4_ADDR  ${IF_LAN_NAME}

	tc filter add dev  ${IF_LAN_NAME} parent 1: protocol ip prio 1 u32 \
		match ip dst ${IF_LAN_NAME}/24 flowid 1:10 \
		action mirred egress redirect dev ifb0
}

# ---- Main program ----

if config_load_package "xcostc"; then
	if [ "$DEBUG" == "1" ];	then
		echo "Can't find configure file"
	fi
	exit 1;
fi

load_modules
reset

mangle

# Acquire actual rate from SpeedTouch modem:
# BANDWIDTH=`echo -e "root\r\npassword\r\nadsl info\r\nexit\r\n" | nc 192.168.1.254 23 | grep Bandwidth | sed -e s/[^0-9]/\ /g`
#
# for word in $BANDWIDTH
# do
#    RATE_DOWN=$RATE_UP
#    RATE_UP=$word
# done;

if [ "$DEBUG" == "1" ]
then
    echo Setting up Fair NAT with $RATE_UP kbit up / $RATE_DOWN kbit down.
fi

RATE_UP=$(uci get xcostc.@xcostc[0].upload_rate)
RATE_DOWN=$(uci get xcostc.@xcostc[0].download_rate)

qos ${IF_WAN_NAME} "${RATE_UP}"kbit

qos_filter ifb0 "$RATE_DOWN"kbit
# ---- End of file. ----
