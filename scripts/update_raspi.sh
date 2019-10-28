#!/bin/bash

# Updates the currently installed version to be this current version
sudo rm -r /opt/beebit/*
sudo cp -r beetrack *.cfg netconf.py dnn/ /opt/beebit

echo "Beebit version update complete."