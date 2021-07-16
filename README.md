# WioTerminal RoLa-gateway-tester

## introduction

The RoLa gateway tester is developed to deploy the LoRaWAN network, for example, we need to verify what is the coverage and if the signal is work in a normal range. Meanwhile, The RoLa tester is a multipurpose, mobile, tool to help you to find out the best location via GPS and monitor the coverage in a zone.

I was analysing the Rola tester market, the price between 150 - 400 pounds and some of them does not have backend application to report the network seen the signal, current time and status etc. So I decide to make a cheap one to help user to reduce their expenditure. 

## Feature

- Selection for || Power(2-16) || SF(7-12) || set test numbers(10-990) || test times ||
- network segment selection (EU868, US915, US915HYBRID, AU915, AS923, KR920, IN865)
- LoRa device information such as DevEui, APPEui, Appkey and Firmware version 
- record the last time test data
- display RoLa device connection, network connection, uplink and downlink status
- Graph selection display of RSSI and SNR
- Uplink and Downlink times number and packet loss probability
- GPS position reporting and the current time and satellites number.


## Hardware 
those products are low cost than I saw on market, total less than 70 pounds.

- [**WioTerminal**](https://www.seeedstudio.com/Wio-Terminal-p-4509.html)
- [**Wio Terminal Chassis - Battery (650mAh)**](https://www.seeedstudio.com/Wio-Terminal-Chassis-Battery-650mAh-p-4756.html)



## Usage

The LoRa tester is basically sending a frame on demand in regular to the gateway and then transfer to server(Uplink), after that it will enter waiting for an ACK status. If the RoLa tester does not get the responded, it will sent the same frame again until the number of setting. conversely, the ACK obtain the respond(Downlink) back to LoRa tester, that mean the message is passed to a backend service, eventually the imformation will display on the Wio terminal screen. 

This project bases using on the Arduino, you need to download the Arduino IDE and some library on your PC, if you are first time use the Wio terminal, here is the [**Wio terminal instruction**](https://wiki.seeedstudio.com/Wio-Terminal-Getting-Started/).

requite library:
- [**Seeed_Arduino_LCD**](https://github.com/Seeed-Studio/Seeed_Arduino_LCD)
- [**Seeed_Arduino_SFUD**](https://github.com/Seeed-Studio/Seeed_Arduino_SFUD)



Each LoRa device has a unique serial number, after you connect the LoRa device to the Wio terminal then there will display the deveui, appeui and appkey on the first page, you need to fill the LoRa ID and gateway ID in server.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/LoRa_ID.png"/></div>

There are Rssi and Snr data, and it will display the signal and Snr bar and packet loss after get the ACK respond.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/RSSI.png"/></div>

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/SNR.png"/></div>

This device have a GPS function as well, but it is not recommended for using in enclosed speace in case effect collect satellites.

<div align=center><img width = 600 src="https://files.seeedstudio.com/wiki/LoRa_WioTerminal/GPS.png"/></div>



















