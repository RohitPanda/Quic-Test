#!/bin/bash   

./youtube_tcp_test -4 --maxtime 30 https://www.youtube.com/watch?v=$1 &
psrecord $(pidof youtube_tcp_test) --log activity_tcp.txt --plot plot_tcp.png --include-children &
P1=$!
./youtube_test -4 --p 3089,3090,3091 --qver Q043 --maxtime 30 https://www.youtube.com/watch?v=$1 &
psrecord $(pidof youtube_test) --log activity_quic.txt --plot plot_quic.png --include-children &
P2=$!

wait $P1 $P2
#wait $P1
echo 'Done'

