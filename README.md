# BeeBit IoT Component

This repository contains all the necessary files to run the BeeBit IoT software that automatically counts individuals in a space in a number of ways.

## Instructions

### Linux
Building on *Linux* is relatively straightforward. The below instructions are designed for *debian-based distros.* such as **Ubuntu**

**Make sure you don't have OpenCV installed from the default package manager.**
```bash
sudo apt remove libopencv*
```

1. Install all the required dependencies.
```bash
sudo apt install libcurl4-openssl-dev build-essential
```
Optionally, also install libgtkmm if you would like UI support.
```bash
sudo apt install libgtkmm-3.0-dev
```
2. Download opencv **4.1.1** (the lastest version as of the time of writing) from [here](https://github.com/opencv/opencv/archive/4.1.1.zip)
3. Download the opencv contrib repository [here](https://github.com/opencv/opencv_contrib/archive/4.1.1.zip)
4. Use the following commands to extract the archive and install OpenCV with the contrib modules. The default flags are fine.
```bash
unzip opencv-4.1.1.zip
unzip opencv_contrib-4.1.1.zip

cd opencv_contrib-4.1.1/modules
rm -r !("tracking"|"plot")
cd $HOME/Downloads

mkdir opencv-4.1.1/build && cd opencv-4.1.1/build
cmake ../ -D OPENCV_EXTRA_MODULES_PATH=$HOME/Downloads/opencv_contrib-4.1.1/modules
make -j4
sudo make install
```
5. OpenCV is now installed to **/usr/local/**. Let's make sure that our linker knows where to find the libraries. **NOTE:** This will change depending on your environment.
```bash
cd /etc/ld.so.conf.d/
sudo echo /usr/local/lib/ >> x86_64-linux-gnu.conf
sudo ldconfig
```

6. Go back to where you have cloned this repository. Use the following command to compile and run the project, downloading the models that are necessary;
```bash
make deps && ./beetrack
```
### Windows
1.