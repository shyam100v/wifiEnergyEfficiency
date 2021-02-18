# Setting up a network sniffer

Following are the steps to use [this](https://www.alfa.com.tw/products/awus036ach?variant=36473965871176) (Alfa AWUS036ACH) USB Wi-Fi adapter as a Wi-Fi sniffer. The steps are tested on a Ubuntu (16 and above) machine. 

* In a terminal window, execute the following commands in order:
```BASH
sudo apt-get install dkms
git clone -b v5.6.4.2 https://github.com/aircrack-ng/rtl8812au.git
cd rtl*
sudo apt-get update
sudo apt-get install build-essential libelf-dev linux-headers-`uname -r`
sudo make dkms_install
```
* Disconnect and reconnect the Wi-Fi adapter (if it was already connected).
* Execute the following in the terminal:
```BASH
sudo lshw -C network
```
You should see the new wireless interface. Note down the logical name. In this example it is **wlan1**. *This will be used in subsequent commands*. 
* Install airmon-ng using:
```BASH
sudo apt-get update
sudo apt-get install -y aircrack-ng
```
Refer [here](https://github.com/aircrack-ng/rtl8812au) for more information on airmon-ng.

## Setting up Monitor mode

* Find the network that you want to monitor - you need the channel number. Let's sweep all supported channels using this Wi-Fi adapter.
```BASH
sudo iwlist wlan1 scan > networkScan.txt
```
Open this text file, look for the network SSID and find the channel number. In this example it is **11**. If you cannot find the network, re-run the scan. If you still cannot find the SSID, restart the network router, go have a cup of coffee and try again.

* The following commands will configure the adapter to monitor in the specific channel (11 here):
```BASH
sudo airmon-ng check kill
sudo iwconfig wlan1 channel 11
ip link set wlan1 down
iw dev wlan1 set type monitor
ip link set wlan1 up
```
> Note that you cannot use this machine for any other networking purposes (unless it is part of a wired network). If you want to use this machine for usual networking purposes after this, you should restart the machine.

Now, open WireShark - you may need to run wireshark with 'sudo' if it was not configured properly. Select **wlan1** from the list of interfaces and that's all.

* In order to decrypt the packets (from any specific node) that you are monitoring, you need the password for the network (if it is encrypted). Follow [this](https://wiki.wireshark.org/HowToDecrypt802.11) tutorial to set it up. Note that *in order to decrypt packets from a specific node, you need to capture the communication from that node right from the authentication and association steps*. 








