#!/bin/bash

# Install all dependencies required by OpenCV
sudo apt install libcurl4-openssl-dev build-essential libgtk2.0-dev
sudo apt install libgtk-3-dev libcanberra-gtk3-dev
sudo apt install libtiff-dev zlib1g-dev
sudo apt install libjpeg-dev libpng-dev
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt install libxvidcore-dev libx264-dev

mkdir raspi_install && cd raspi_install
WORKING_DIRECTORY=`pwd`

# Install a precompiled version of OpenCV for the raspberry pi
git clone git@github.com:winzlebee/opencv-4.1.1-armhf.git
tar -xvf opencv-4.1.1-armhf.tar.bz2

pushd opencv-4.1.1-armhf
sudo cp opencv-4.1.1-arm /opt
popd

echo "export CPLUS_INCLUDE_PATH=/usr/local/include:/opt/opencv-4.1.1-arm/include:$CPLUS_INCLUDE_PATH" | sudo tee -a ~/.bashrc
echo "export LD_LIBRARY_PATH=/opt/opencv-4.1.1-arm/lib" | sudo tee -a ~/.bashrc
source ~/.bashrc

# Install RaspiCam so that the pi camera can be used on the device
git clone https://github.com/cedricve/raspicam .
mkdir raspicam/build
pushd raspicam/build
cmake ../
make && sudo make install
popd

sudo ldconfig

# Install the beebit service onto the system
sudo mkdir /opt/beebit
sudo cp -r beetrack *.cfg dnn/ /opt/beebit
sudo cp scripts/beebit.service /etc/systemd/system/
systemctl enable beebit
