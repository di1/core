#!/bin/bash

set -e

echo "WARNING: THIS WILL DOWNLOAD OVER 1TB+ OF DATA"
read -r -p "Would you like to continue? [y/N] " acceptance

if [[ $acceptance =~ ^(no|N) ]]
then
  exit 0
elif [[ $acceptance =~ ^(yes|y) ]]
then
  echo "ok" > /dev/null
else
  exit 0
fi

RESPONSE=`curl https://iextrading.com/api/1.0/hist`

DOWNLOAD_LOCATION=$1

cd $DOWNLOAD_LOCATION

LINKS=`echo $RESPONSE | jq -r '.[] | .[] | select(.feed=="DEEP") .link'`

for key in $LINKS
do
  DATE=`echo $RESPONSE | jq -r ".[] | .[] | select(.link==\"$key\") .date"`
  if test -f "${DATE}_DEEP.pcap"
  then
    echo "Skipping ${DATE}_DEEP.pcap.gz"
    continue
  fi

  echo "Download ${DATE}_DEEP.pcap.gz"
  
  curl $key -o "${DATE}_DEEP.pcap.gz"
  gunzip ${DATE}_DEEP.pcap.gz
done
