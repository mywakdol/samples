#!/bin/bash
function jsonval {
    temp=`echo $json | sed 's/\\\\\//\//g' | sed 's/[{}]//g' | awk -v k="text" '{n=split($0,a,","); for (i=1; i<=n; i++) print a[i]}' | sed 's/\"\:\"/\|/g' | sed 's/[\,]/ /g' | sed 's/\"//g' | grep -w $prop`
    echo ${temp##*|}
}

# Please change GUID, channel ID and field name
# You can see this fields on your channel's info
url='http://agnosthings.com/{GUID}/field/last/feed/{channel ID}/{field name}'
json=`curl -s -X GET $url`
prop='value'
val=`jsonval`

# This sample uses GPIO 4 (pin #7)
# Change the gpio to reflect the pin you're using that connects to the relay
echo $val > /sys/class/gpio/gpio4/value
