#!/bin/sh
RESULT=$1
FILE=$2
OK_STATUS="$(echo $RESULT | grep OK)"

if [[ "$OK_STATUS" == "" ]]; then
  exit 0
fi 

TEST_VERSION="$(echo $RESULT | cut -d ';' -f 1)"
TIMESTAMP="$(echo $RESULT | cut -d ';' -f 2)"
echo $TIMESTAMP >> $FILE
if [[ "$TEST_VERSION" == "YOUTUBE_QUIC.0" ]]; then
  echo "$RESULT" | cut -d ';' --output-delimiter $'\n' -f 19,30 | uniq | while read -r a; do sudo scamper -O warts -i $a | sc_warts2csv | tail -n +2 >> $FILE ; done
fi 

if [[ "$TEST_VERSION" == "YOUTUBE.6" ]]; then
  
  echo "$RESULT" | cut -d ';' --output-delimiter $'\n' -f 19,29 | uniq | while read -r a; do sudo scamper -O warts -i $a | sc_warts2csv | tail -n +2 >> $FILE ; done
fi

echo >> $FILE