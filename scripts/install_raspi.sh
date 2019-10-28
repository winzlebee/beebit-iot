#!/bin/bash

BEARER_KEY="2%qH3n\$d2z^SS-aV"
ENDPOINT="http://app.beebithive.com/bee/manufacture"

# Use the web api to get a new registration UUID for this device. Remember to send auth token with the request
DEVICE_UUID=`curl -s -H "Authorization: Bearer $BEARER_KEY" -X POST $ENDPOINT`

if [ ${#DEVICE_UUID} -eq 32 ]; then
    echo "uuid=$DEVICE_UUID" >> beemon.cfg

    # Install the beebit service onto the system
    sudo mkdir /opt/beebit

    sudo cp -r beetrack *.cfg netconf.py dnn/ /opt/beebit
    sudo cp scripts/beebit.service /etc/systemd/system/
    sudo cp scripts/beenetconf.service /etc/systemd/system/

    sudo systemctl enable beebit
    sudo systemctl enable beenetconf

    echo "BeeBit Service has been installed. This device UUID is $DEVICE_UUID"
    echo "Please test using the command '/opt/beebit/beetrack'"
else
    echo "Malformed response from server. Please check you have access to the internet."
fi
