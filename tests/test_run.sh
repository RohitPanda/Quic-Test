#!/bin/sh
PORTS="3089,3090,3091"
MAX_TIME_ARG="--maxtime 180"

VIDEO_ID="$(head -1 top_list.txt)"

if [[ "$VIDEO_ID" == "" ]]; then
  echo "no links are left"
  exit -1
fi 

ARGS="-4 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID"
TEST_RESULT_4="$(./youtube_tcp_test $ARGS)"
if [[ "$TEST_RESULT_4" != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_tcp_test $ARGS)"
fi

ARGS="-6 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID"
TEST_RESULT_6="$(./youtube_tcp_test $ARGS)"
if [[ "$TEST_RESULT_6" != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_tcp_test $ARGS)"
fi

ERROR_GREP="$(grep -o ';621;$' <<< $TEST_RESULT_4)"
ERROR_GREP+="$(grep -o ';621;$' <<< $TEST_RESULT_6)"

while [[ "$ERROR_GREP" != "" ]]
do
  sed -i '1d' top_list.txt
  VIDEO_ID="$(head -1 top_list.txt)"
  if [[ "$VIDEO_ID" == "" ]]; then
    echo "no links are left"
    exit -1
  fi

  ARGS="-4 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID"
  TEST_RESULT_4="$(./youtube_tcp_test $ARGS)"
  ARGS="-6 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID "
  TEST_RESULT_6="$(./youtube_tcp_test $ARGS)"

  ERROR_GREP="$(grep -o ';621;$' <<< $TEST_RESULT_4)"
  ERROR_GREP+="$(grep -o ';621;$' <<< $TEST_RESULT_6)"
done

mkdir -p results
if [[ $TEST_RESULT_4 == *"YOUTUBE"* ]]; then
  echo $TEST_RESULT_4>>results/tcp4results.txt
  bash print_route.sh $TEST_RESULT_4 results/tcp4route.txt
fi
if [[ $TEST_RESULT_6 == *"YOUTUBE"* ]]; then
  echo $TEST_RESULT_6>>results/tcp6results.txt
  bash print_route.sh $TEST_RESULT_6 results/tcp6route.txt
fi

ARGS="-4 --p $PORTS --qver Q035 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID"
TEST_RESULT_4="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_4 != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_test $ARGS)"
fi
if [[ $TEST_RESULT_4 == *"YOUTUBE"* ]]; then
  echo "$TEST_RESULT_4" >> results/ip4quic35results.txt 
  bash print_route.sh $TEST_RESULT_4 results/ip4quic35route.txt
fi


ARGS="-4 --p $PORTS --qver Q039 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID "
TEST_RESULT_4="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_4 != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_test $ARGS)"
fi
if [[ $TEST_RESULT_4 == *"YOUTUBE"* ]]; then
  echo "$TEST_RESULT_4" >> results/ip4quic39results.txt 
  bash print_route.sh $TEST_RESULT_4 results/ip4quic39route.txt
fi

ARGS="-4 --p $PORTS --qver Q043 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID "
TEST_RESULT_4="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_4 != *"YOUTUBE"* ]]; then
  TEST_RESULT_4="$(./youtube_test $ARGS)"
fi
if [[ $TEST_RESULT_4 == *"YOUTUBE"* ]]; then
  echo "$TEST_RESULT_4" >> results/ip4quic43results.txt 
  bash print_route.sh $TEST_RESULT_4 results/ip4quic43route.txt
fi

ARGS="-6 --p $PORTS --qver Q035 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID"
TEST_RESULT_6="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_6 != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_test $ARGS)"
fi
if [[ $TEST_RESULT_6 == *"YOUTUBE"* ]]; then
  echo "$TEST_RESULT_6" >> results/ip6quic35results.txt 
  bash print_route.sh $TEST_RESULT_6 results/ip6quic35route.txt
fi

ARGS="-6 --p $PORTS --qver Q039 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID"
TEST_RESULT_6="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_6 != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_test $ARGS)"
fi
if [[ $TEST_RESULT_6 == *"YOUTUBE"* ]]; then
  echo "$TEST_RESULT_6" >> results/ip6quic39results.txt 
  bash print_route.sh $TEST_RESULT_6 results/ip6quic39route.txt
fi

ARGS="-6 --p $PORTS --qver Q043 $MAX_TIME_ARG https://www.youtube.com/watch?v=$VIDEO_ID"
TEST_RESULT_6="$(./youtube_test $ARGS)"
if [[ $TEST_RESULT_6 != *"YOUTUBE"* ]]; then
  TEST_RESULT_6="$(./youtube_test $ARGS)"
fi
if [[ $TEST_RESULT_6 == *"YOUTUBE"* ]]; then
  echo "$TEST_RESULT_6" >> results/ip6quic43results.txt 
  bash print_route.sh $TEST_RESULT_6 results/ip6quic43route.txt
fi