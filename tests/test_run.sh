#!/bin/sh
PORTS="3089,3090,3091"
MAX_TIME_ARG="--maxtime 180"

VIDEO_ID="$(head -1 top_list.txt)"

if [[ "$VIDEO_ID" == "" ]]; then
  echo "no links are left"
  exit -1
fi 

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -4 $MAX_TIME_ARG"
TEST_RESULT_4="$(./youtube_tcp_test $ARGS)"
if [[ "$TEST_RESULT_4" != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_tcp_test $ARGS)"
fi

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -6 $MAX_TIME_ARG"
TEST_RESULT_6="$(./youtube_tcp_test $ARGS)"
if [[ "$TEST_RESULT_6" != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_tcp_test $ARGS)"
fi

ERROR_GREP="$(grep -o ';621;$' <<< $TEST_RESULT_4)"
ERROR_GREP+="$(grep -o ';621;$' <<< $TEST_RESULT_6)"

while [[ "$ERROR_GREP" -ne "" ]]
do
  sed -i '1d' top_list.txt
  VIDEO_ID="$(head -1 top_list.txt)"
  if [[ "$VIDEO_ID" == "" ]]; then
    echo "no links are left"
    exit -1
  fi

  ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -4"
  TEST_RESULT_4="$(./youtube_tcp_test $ARGS)"
  ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -6"
  TEST_RESULT_6="$(./youtube_tcp_test $ARGS)"
done

mkdir -p results
echo $TEST_RESULT_4>>results/tcp4results.txt
echo $TEST_RESULT_6>>results/tcp6results.txt

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -4 --p $PORTS --qver Q035 $MAX_TIME_ARG"
TEST_RESULT_4="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_4 != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_test $ARGS)"
fi
echo "$TEST_RESULT_4" >> results/ip4quic35results.txt 

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -4 --p $PORTS --qver Q039 $MAX_TIME_ARG"
TEST_RESULT_4="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_4 != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_test $ARGS)"
fi
echo "$TEST_RESULT_4" >> results/ip4quic39results.txt

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -4 --p $PORTS --qver Q043 $MAX_TIME_ARG"
TEST_RESULT_4="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_4 != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_test $ARGS)"
fi
echo "$TEST_RESULT_4" >> results/ip4quic43results.txt

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -6 --p $PORTS --qver Q035 $MAX_TIME_ARG"
TEST_RESULT_6="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_6 != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_test $ARGS)"
fi
echo "$TEST_RESULT_6" >> results/ip6quic35results.txt

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -6 --p $PORTS --qver Q039 $MAX_TIME_ARG"
TEST_RESULT_6="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_6 != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_test $ARGS)"
fi
echo "$TEST_RESULT_6" >> results/ip6quic39results.txt

ARGS="https://www.youtube.com/watch?v=$VIDEO_ID -6 --p $PORTS --qver Q043 $MAX_TIME_ARG"
TEST_RESULT_6="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_6 != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_test $ARGS)"
fi
echo "$TEST_RESULT_6" >> results/ip6quic43results.txt