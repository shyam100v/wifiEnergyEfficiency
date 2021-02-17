# Steps for setting up TI Code Composer Studio (CCS) and Serial terminal

CCS and UniFlash are required to create, build, debug and burn user programs/code into SimpleLink devices. Burning the image can be done either through CCS or UniFlash. The following steps are for intalling the required software for CC3235SF development kits. For CC3200, a separate version of UniFlash and SDK are required. In addition, a serial terminal application (such as MobaXterm) is required.

In a Windows machine (Windows 7 or later versions), install the following:

* Software Development Kit (SDK) from for CC3235SF from [here](https://www.ti.com/tool/SIMPLELINK-CC32XX-SDK)
* The latest version of Code Composer Studio (CCS) from [here](https://software-dl.ti.com/ccs/esd/documents/ccs_downloads.html) 
* MobaXterm from [here](https://mobaxterm.mobatek.net/download-home-edition.html)
* (Optional) UniFlash from [here](http://www.ti.com/tool/download/UNIFLASH)

### Setting up CCS for CC3235SF
After installing, open CCS. Open View -> Resource Explorer. Type in CC3235SF into the filter field. On the left, you can find all the documentation and demos for using CC3235SF launchpad and the associated software here. 

For example, to import the "Power Measurement" example into CCS IDE, navigate to Demos->power_measurement -> TI-RTOS -> CCS Compiler -> power_measurement on the left bar, and select Import. This will appear on the Project Explorer window.

To build and burn this project image directly from CCS (without using UniFlash):

* Click on the project in the project explorer. Open 'image.sysconfig'. Change any necessary setting (such as disable 5 GHz, change coutry code, etc).
* Right click and build project.
* If you want to flash this, connect the SimpleLink device, click on the 'Flash' button in the toolbar (button with two curly braces). 


### Setting up MobaXterm/Serial terminal for serial connection

You can use any serial terminal application. Follow these steps:

* Connect the device to your Windows machine. Find the COM port from 'Device manager'. Look for 'XDS110 Class Application/User UART'. In this example, it is COM3.
* Open serial terminal applicaiton, create a new serial session with following details:
    * Serial Port as found above
    * Speed (bps) as 115200 bps


### Setting up UniFlash

If you want to use UniFlash to flash project images, follow [this](https://dev.ti.com/tirex/explore/node?node=ABEoqU9o3snoxDcmIpW0EA__fc2e6sr__LATEST) tutorial from TI.


### Resources

* About sysconfig image creator [Link](https://dev.ti.com/tirex/explore/content/simplelink_academy_cc32xxsdk_4_40_00_00/modules/wifi/wifi_sysconfig_imagecreator/wifi_sysconfig_imagecreator.html)


