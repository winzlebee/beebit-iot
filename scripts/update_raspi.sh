#!/bin/bash

# Updates the currently installed version to be this current version

sudo rm /etc/systemd/system/beebit.service
sudo rm /etc/systemd/system/beenetconf.service

sudo cp scripts/beebit.service /etc/systemd/system/
sudo cp scripts/beenetconf.service /etc/systemd/system/

sudo systemctl daemon-reload

sudo rm -r /opt/beebit/*
sudo mkdir /opt/beebit
sudo cp -r beetrack *.cfg netconf.py dnn/ /opt/beebit

echo "Beebit version update complete."