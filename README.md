# BeeBit IoT Component

This repository contains all the necessary files to run the BeeBit IoT software that automatically counts individuals in a space in a number of ways.

## Instructions

### Linux
Building on *Linux* is relatively straightforward. The below instructions are designed for *debian-based distros.* such as **Ubuntu**

**Make sure you don't have OpenCV installed from the default package manager.**
```bash
sudo apt remove libopencv*
```

1. Run the unified install script to install all dependencies
```bash
chmod +x ./scripts/install_opencv.sh
sudo ./scripts/install_opencv.sh
```
2. Run the following command in the root directory to install resources and run the project.
```bash
make deps && ./beetrack
```

#### Uninstallation
If you'd like to uninstall the dependencies and tracker at any time, simply run;
```bash
make clean
./scripts/install_opencv.sh uninstall
```

### Raspberry Pi
Support for *raspbian* or any armhf debian distribution is provided by a unified install script.

**NOTE:** This process installs all the dependencies and sets up the tracker to run on boot of the device. It will use a lot of CPU. The beebit service can be disabled with `systemctl stop beebit`

```bash
make deps
sudo make install
```

### Manual installation of OpenCV libraries

If for some reason the install script doesn't work, the below instructions should assist the debugging process.

1. Install all the required dependencies.
```bash
sudo apt install libcurl4-openssl-dev build-essential libgtk2.0-dev
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