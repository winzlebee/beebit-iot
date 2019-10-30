# Script to uninstall all installed components

cd raspi_install
WORKING_DIRECTORY=`pwd`

sudo rm /etc/systemd/system/beebit.service
sudo rm /etc/systemd/system/beenetconf.service
sudo rm -r /opt/beebit

pushd raspicam/build
sudo make uninstall
popd

rm -r /opt/opencv-4.1.1-arm

cd ../
rm -r raspi_install
