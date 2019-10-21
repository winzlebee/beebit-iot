# Install all dependencies required by OpenCV
sudo apt install libcurl4-openssl-dev build-essential libgtk2.0-dev -y
sudo apt install libgtk-3-dev libcanberra-gtk3-dev -y
sudo apt install libtiff-dev zlib1g-dev -y
sudo apt install libjpeg-dev libpng-dev -y
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev -y
sudo apt install libxvidcore-dev libx264-dev -y

mkdir raspi_install && cd raspi_install
WORKING_DIRECTORY=`pwd`

# Install a precompiled version of OpenCV for the raspberry pi
git clone git@github.com:winzlebee/opencv-4.1.1-armhf.git
pushd opencv-4.1.1-armhf
tar -xvf opencv-4.1.1-armhf.tar.bz2
sudo cp opencv-4.1.1-arm /opt
popd

echo "export CPLUS_INCLUDE_PATH=/usr/local/include:/opt/opencv-4.1.1-arm/include:$CPLUS_INCLUDE_PATH" | sudo tee -a ~/.bashrc
echo "export LD_LIBRARY_PATH=/opt/opencv-4.1.1-arm/lib" | sudo tee -a ~/.bashrc
source ~/.bashrc

# Install RaspiCam so that the pi camera can be used on the device
git clone https://github.com/cedricve/raspicam
mkdir raspicam/build
pushd raspicam/build
cmake ../
make && sudo make install
popd

sudo ldconfig