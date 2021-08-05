# WioTerminal LoRaWAN Gateway Tester

## Introduction

The LoRaWAN Gateway Tester is a multipurpose portable tool that can detect the coverage of your LoRaWAN gateways. It will then apprise you of whether the signal is in a normal range. Designed to make the deployment of a LoRaWAN network easier, the LoRaWAN Gateway Tester will help you determine the optimum location for your LoRaWAN gateway deployment.

Prior to developing this, we checked out the LoRa tester market; the price usually ranges from $200 to $500, however, some of them lack a backend application to report the network signal, current time, status, and several other critical information. Therefore, we decided to bring a low-cost version ourselves that is able to do more! This project is based on [**Paul Pinault**](https://github.com/disk91/WioLoRaWANFieldTester)’s [**WioLoRaWANFieldTester**](https://www.disk91.com/2021/technology/lora/low-cost-lorawan-field-tester/) report. We are honoured to have access to his link and we thank him for his contributions. We combined Paul Pinault’s natty UI for the Wio terminal with LoRa-E5 and GNSS modules to produce the function of the LoRaWAN Gateway Tester.


<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/%E5%9C%BA%E6%99%AF%E5%9B%BE2.jpg"/></div>

## Feature

- Menu selection for different modes of operation; Power controls, SF, test numbers input, etc.
- Supports various network segments (EU868, US915, US915HYBRID, AS923, KR920, IN865)
- Display LoRa device connection and uplink-downlink status
- Backup of previous test results
- Graphical display of RSSI and SNR
- Shows the number of uplink and downlink times, as well as the likelihood of packet loss
- GPS position reporting, as well as the current time and number of satellites.
- Display LoRa Device information i.e. DevEui, APPEui, Appkey, Firmware version, and more
- Define the DevEui, APPEui and Appkey

## Hardware 
The hardware used in this project is more affordable than most on the market, with the total cost adding up to less than a hundred dollars.

- [**WioTerminal**](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)
- [**Wio Terminal Chassis - Battery (650mAh)**](https://www.seeedstudio.com/Wio-Terminal-Chassis-Battery-650mAh-p-4756.html)
- [**Wio Terminal Chassis - LoRa-E5 and GNSS**](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)



## Usage


### Wio Terminal instruction

The LoRaWAN Gateway Tester basically sends a frame on-demand to the gateway on a regular basis, then transfers to the server (Uplink). It then waits for an ACK status. If the LoRa tester does not receive a response, it will continue to send the same frame until the preset number has been reached. The ACK, on the other hand, returns the response(Downlink) to the LoRa tester, implying that the message is forwarded to a backend service, from whence the information is eventually shown on the Wio terminal screen.
This project is based on the Arduino platform which means we’ll be using the Arduino IDE and various Arduino libraries. If this is your first time using the Wio terminal, here is a guide to quickly [**Get Started with Wio Terminal**](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/).

requite library:
- [**Seeed_Arduino_SFUD**](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD)
- [**TinyGPS**](https://github.com/mikalhart/TinyGPSPlus)
- [**LovyanGFX**](https://github.com/lovyan03/LovyanGFX)

### Note

When you upload the code, please selecte slave mode.
    <div align=center><img width = 400 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/ROLA.png"/></div>

### TheThingsNetwork Console Configuration Setup

In this project, I test the LoRa tester on [**TheThingsNetwork**](https://www.thethingsnetwork.org) platform, the instuction as below:

Step 1: Load into [**TTN website**](https://www.thethingsnetwork.org) and create your account, then go to gateways start to set up your device.

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_gataway1.png"/></div>

Step 2: Add the gateway device:
- Owner
- Gteway ID
- Gateway EUI
- Gateway name

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_gateway2.png"/></div>

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_gateway3.png"/></div>

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_gateway4.png"/></div>

Step 3: Add Application:
- Owner
- Application ID
- Application name

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_applications.png"/></div>

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/application2.png"/></div>

Step 4：Add the LoRa node:
- Brand (Select Sense CAP)
- Model (Select LoRa-E5)
- Hardware Ver (Defult)
- Firmware Ver (Defult)
- Profile (The Region is according to your location)
- Frequency plan
- AppEUI
- DEVEUI
- AppKey
- End Device ID

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_device1.png"/></div>

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_device4.png"/></div>

Step 5: Add the code for decode the data:

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_decode1.png"/></div>

```CPP
function Decoder(bytes, port) {
 
  var decoded = {};
  if (port === 8) {
    decoded.latitude   = (bytes[3] | (bytes[2]<<8) | (bytes[1]<<16)  | (bytes[0]<<24)) /1000000;
    decoded.longitude  = (bytes[7] | (bytes[6]<<8) | (bytes[5]<<16)  | (bytes[4]<<24)) /1000000;
    decoded.altitude   = (bytes[9] | (bytes[8]<<8));
    decoded.satellites = bytes[9];
  }
 
  return decoded;
}
```

Step 5: Cheack the result on TheThingsNetwork

Go to the geteway, then click "Live data".

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/TTN_data.png"/></div>

### Wio terminal intruction

Each LoRa device has a unique serial number, after you connect the LoRa device to the Wio terminal then there will display the deveui, appeui and appkey on the first page, you need to fill the LoRa ID and gateway ID in server.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/rola_tester_ID.png"/></div>

There are Rssi and Snr data, and it will display the signal and Snr bar and packet loss after get the ACK respond.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/RSSI.png"/></div>

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/SNR.png"/></div>

This device have a GPS function as well, but it is not recommended for using in enclosed speace in case effect collect satellites.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/GPS.png"/></div>


## schematic
Take a look at board directory for details on PCB & components.
Here is the simplified version of the schematics for DiY implementation.
if you want more detail, just go to check the file list.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/LORA_PCB.png"/></div>



## Enclosure 

The 3D printed enclosure can be found in file list which is PCBA WioTerminal Chassis  H and PCB file. You will find the the all design on it. You will also find the FreeCad source file in case you want to modify /improve it.

The 3D prited work as below: 

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/feature.png"/></div>

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/preview.png"/></div>

<div align=center><img width = 500 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/back.png"/></div>













