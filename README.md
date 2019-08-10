# BeeBit IoT Component

This repositroy contains all the necessary files to run the BeeBit IoT software that automatically counts individuals in a space in a number of ways.

## Instructions

### Linux
Building on *Linux* is relatively straightforward. The below instructions are designed for *debian-based distros* but they should work on any distribution.

**Make sure you don't have OpenCV installed from the default package manager.**
```bash
sudo apt remove libopencv*
```

1. Download opencv **4.1.1** (the lastest version as of the time of writing) from [here](https://github.com/opencv/opencv/archive/4.1.1.zip);
2. Use the following commands to extract the archive and install OpenCV. The default flags are fine.
```bash
unzip opencv4.1.1.zip
mkdir opencv4.1.1/build && cd opencv4.1.1/build
cmake ../
make -j4
sudo make install
```
3. OpenCV is now installed to **/usr/local/**. Let's make sure that our linker knows where to find the libraries. **NOTE:** This will change depending on your environment.
```bash
cd /etc/ld.so.conf.d/
sudo echo /usr/local/lib/ >> x86_64-linux-gnu.conf
sudo ldconfig
```

4. Go back to where you have cloned this repository. Use the following command to compile and run the project, downloading the models that are necessary;
```bash
make deps && ./beetrack
```