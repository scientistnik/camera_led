#!/bin/bash

if [[ -p $2 ]]; then
	echo "Start..."
else
	echo "USE: client.sh NAME_FIFO_OUT NAME_FIFO_IN"
	exit 1
fi

out=$1
in=$2

g_state="get-led-state"
s_state="set-led-state"
g_color="get-led-color"
s_color="set-led-color"
g_rate="get-led-rate"
s_rate="set-led-rate"

read

while true
do
		echo
		echo $g_state >$out
		read answer <$in
		if [ "$answer" = "OK on" ]; then
			echo $g_color >$out
			read answer <$in
			if [ "$answer" = "OK red" ]; then
				color="red"
			else if [ "$answer" = "OK green" ]; then
					color="green"
				else if [ "$answer" = "OK blue" ]; then
						color="blue"
					else
						color="none"
					fi
				fi
			fi
			echo $g_rate >$out
			read answer <$in
			if [ "$answer" != "FAILED" ]; then
				rate=`echo ${answer} | sed 's/.*\ //'`
			fi
			echo "LED ${color} f=$rate"
			echo "1. Off LED"
			echo "2. Change color (2 c=green)"
			echo "3. Change rate (3 r=1.45)"
			echo "0. Exit"
			read comm

			if [ "$comm" = "1" ]; then
				echo "$s_state off" >$out
				read answer <$in
			else 
				if [ "$comm" = "0" ]; then
					exit 0
				else 
					if [ `echo "$comm" | cut -c 1` = "2" ]; then
						echo "$s_color `echo $comm | sed 's/.*=//'`" >$out
						read answer <$in
					else 
						if [ `echo "$comm" | cut -c 1` = "3" ]; then
							echo "$s_rate `echo $comm | sed 's/.*=//'`" >$out
							read answer <$in
						fi
					fi
				fi
			fi
		else
			echo "LED off"
			echo 
			echo "1. On LED"
			echo "0. Exit"

			read comm
			if [ "$comm" = "1" ]; then
				echo "$s_state on" >$out
				read answer <$in
			else 
				if [ "$comm" = "0" ]; then
					exit 0
				else
					echo "Bad choice"
				fi
			fi
		fi
done
