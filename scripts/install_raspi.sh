#!/bin/bash

# Install the beebit service onto the system
sudo mkdir /opt/beebit

# Use the web api to get a new registration UUID for this device. Remember to send JWT token


sudo cp -r beetrack *.cfg dnn/ /opt/beebit
sudo cp scripts/beebit.service /etc/systemd/system/
systemctl enable beebit
