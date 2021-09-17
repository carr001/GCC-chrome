#!/bin/bash
# Program: 
# 	Bandwidth change simulation according to paper "Congestion Control for Web Real-Time Communication"-Single GCC Flow
#

Band=500
BandIcrease=500
Duration=50
TimeStamp=$(date +%y%m%d%H%M%S)
Now=$TimeStamp
State="Increase"
MaxBand=2000
DELAY=50ms

while [ 1 ]
do
	Now=$(date +%y%m%d%H%M%S)
	if [ $State == "Increase" ]; then
		if [ $Now -gt $TimeStamp  ]; then
			echo "Current Band=$((Band))kbps"
			TimeStamp=$(($Now+$Duration))
			tc qdisc del dev ens33 root
			tc qdisc add dev ens33 root netem rate $((Band))kbit delay ${DELAY}
			Band=$(($Band+$BandIcrease))
		fi
		if [ $Band -eq $MaxBand ]; then
			State="Decrease"
		fi
	fi
	if [ $State == "Decrease" ]; then
                if [ $Now -gt $TimeStamp  ]; then
			echo "Current Band=$((Band))kbps"
			TimeStamp=$(($Now+$Duration))
                        tc qdisc del dev ens33 root
                        tc qdisc add dev ens33 root netem rate $((Band))kbit delay ${DELAY}
                        Band=$(($Band-$BandIcrease))
                fi
		if [ $Band -lt 500 ]; then
			break
		fi	
	fi
done	


