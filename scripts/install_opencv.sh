#!/bin/bash

shopt -s extglob

if [ "$1" == "uninstall" ]; then
    pushd opencv_install/opencv-4.1.1/build
    sudo make uninstall
    popd
    rm -r opencv_install
    exit 0
fi

sudo apt install libcurl4-openssl-dev build-essential libgtk2.0-dev
sudo apt install libgtk-3-dev libcanberra-gtk3-dev
sudo apt install libtiff-dev zlib1g-dev
sudo apt install libjpeg-dev libpng-dev
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt install libxvidcore-dev libx264-dev

# Download OpenCV and the relevant libraries
mkdir opencv_install && cd opencv_install
WORKING_DIR=`pwd`

wget -O opencv.zip https://github.com/opencv/opencv/archive/4.1.1.zip
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.1.1.zip

unzip opencv.zip && unzip opencv_contrib.zip

# Remove all unnecessary contrib libraries
pushd opencv_contrib-4.1.1/modules
rm -r !("tracking"|"plot")
popd

# Build OPENCV with contrib libraries enabled
mkdir opencv-4.1.1/build && cd opencv-4.1.1/build
cmake ../ -D OPENCV_EXTRA_MODULES_PATH=$WORKING_DIR/opencv_contrib-4.1.1/modules
make -j4

# Install OpenCV into default directories
sudo make install

# Update the linker to include OpenCV directories
echo /usr/local/lib/ | sudo tee -a /etc/ld.so.conf.d/x86_64-linux-gnu.conf
sudo ldconfig
