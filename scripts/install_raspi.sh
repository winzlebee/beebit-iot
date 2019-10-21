#!/bin/bash

# Install the beebit service onto the system
sudo mkdir /opt/beebit
sudo cp -r beetrack *.cfg dnn/ /opt/beebit
sudo cp scripts/beebit.service /etc/systemd/system/
systemctl enable beebit
