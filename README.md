# WioTerminal LoRaWAN Gateway Tester

## Introduction

The LoRaWAN Gateway Tester is developed to deploy the LoRaWAN network, for example, we need to verify what is the coverage and if the signal is work in a normal range. Meanwhile, The LoRaWAN Gateway Tester is a multipurpose, mobile, tool to help you to find out the best location via GPS and monitor the coverage in a zone.

I was analysing the Rola tester market, the price between 200 - 450 dollars and some of them does not have backend application to report the network seen the signal, current time and status etc. So I decide to make a cheap one to help user to reduce their expenditure.
in this project, I made all the functions have worked, but I was struggling with how to make a tidy UI for the Wio terminal, and then I find a great contributor  [**Paul Pinault**](https://github.com/disk91/WioLoRaWANFieldTester) has made the great UI for the Wio terminal and there have some similar functions I can use, so I very appreciate his open resource.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/%E5%9C%BA%E6%99%AF%E5%9B%BE2.jpg"/></div>

## Feature

- Selection for || Power || SF || set test numbers(10-990) || Mode selection ||
- network segment selection (EU868, US915, US915HYBRID, AS923, KR920, IN865)
- LoRa device information such as DevEui, APPEui, Appkey and Firmware version 
- record the last time test data
- display RoLa device connection, network connection, uplink and downlink status
- Graph selection display of RSSI and SNR
- Uplink and Downlink times number and packet loss probability
- GPS position reporting and the current time and satellites number.
- User is able to defined the DevEui, APPEui and Appkey 

## Hardware 
Those products are low cost than I saw on market, total less than 100 dollars.

- [**WioTerminal**](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)
- [**Wio Terminal Chassis - Battery (650mAh)**](https://www.seeedstudio.com/Wio-Terminal-Chassis-Battery-650mAh-p-4756.html)
- [**Wio Terminal Chassis - LoRa-E5 and GNSS**](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)



## Usage


### Wio Terminal instruction

The LoRa tester is basically sending a frame on demand in regular to the gateway and then transfer to server(Uplink), after that it will enter waiting for an ACK status. If the RoLa tester does not get the responded, it will sent the same frame again until the number of setting. conversely, the ACK obtain the respond(Downlink) back to LoRa tester, that mean the message is passed to a backend service, eventually the imformation will display on the Wio terminal screen. 

This project bases using on the Arduino, you need to download the Arduino IDE and some library on your PC, if you are first time use the Wio terminal, here is the [**Wio terminal instruction**](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/).

requite library:
- [**Seeed_Arduino_LCD**](https://github.com/Seeed-Studio/Seeed_Arduino_LCD)
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













