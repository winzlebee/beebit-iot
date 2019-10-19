#!/bin/bash

mkdir raspi_install && cd raspi_install
WORKING_DIRECTORY=`pwd`

# Install a precompiled version of OpenCV for the raspberry pi
git clone git@github.com:winzlebee/opencv-4.1.1-armhf.git
tar -xvf opencv-4.1.1-armhf.tar.bz2

pushd opencv-4.1.1-armhf
sudo cp opencv-4.1.1-arm /opt
popd

echo "export CPLUS_INCLUDE_PATH=/opt/opencv-4.1.1-arm/include" | sudo tee -a ~/.bashrc
echo "export LD_LIBRARY_PATH=/opt/opencv-4.1.1-arm/lib" | sudo tee -a ~/.bashrc
source ~/.bashrc

