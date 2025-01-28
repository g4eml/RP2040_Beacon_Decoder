# RP2040_beacon_Decoder

## Description

This Project is an experimental stand-alone Beacon decoder for JT4G and PI4 Amateur radio Beacons. 

## Features

Stand alone Beacon Decoder with LCD touch screen display. 

No special programming hardware or software required. 

Serial programming using the RP2040s built in USB serial port and standard boot loader. 

Settings are saved to EEPROM for automatic load on power on.

Supports JT4G and PI4 datat modes, as used by many VHF, UHF and Microwave Beacons. 

Supports optional GPS connection for the accurate timing required for the JT4G and PI4 data modes. 

## Requirements

This code is designed to work with the Elecrow CrowPanel Pico-3.5 inch 480x320 TFT LCD HMI Module. https://www.aliexpress.com/item/1005007250778536.html 



Note:- similar HMI Panels are available using the ESP32 processor chip. Make sure that you are purchasing the RP2040 version. 
 
Data modes such as JT4G and PI4 need accurate timing information. Each transmission is synchronised to the start of the minute. You can manuualy synchronise the decoder to an accurate timesource using the SYNC button or you can optionally connect a GPS module to provide automatic time synchronisation.  

![GPS](https://github.com/user-attachments/assets/53ceb650-0525-4138-a7ff-bff1f450c409)

## Connecting

the module is connected to the receiver via a simple level shifting interface. Details are shown in this document 

The optional GPS module can be connected the UART1 connector on the lower edge of the MHI Module. 

The power is provided by the USB-C connector marked 'USB' on the end of the HMI module, or optionally by a 3.7V lithium cell connected to the BAT connector. 


## Programming or updating the HMI Module (quick method) 

1. Locate the latest compiled firmware file 'RP2040_Beacon_Decoder.uf2' which will be found here https://github.com/g4eml/RP2040_Beacon_Decoder/releases and save it to your desktop. 

2. Connect the HMI Module to your PC using the USB-C port on the side. 

3. Hold down the BOOT button on the back of the HMI module and briefly press the Reset button. The RP2040 should appear as a USB disk drive on your PC.

3. Copy the .uf2 file onto the USB drive. The RP2040 will recognise the file and immediately update its firmware and reboot.

## Building your own version of the firmware (longer method and not normally required)

The RP2040 is programmed using the Arduino IDE with the Earl F. Philhower, III  RP2040 core. 

#### Installing the Arduino IDE

1. Download and Install the Arduino IDE 2.3.0 from here https://www.arduino.cc/en/software

2. Open the Arduino IDE and go to File/Preferences.

3. in the dialog enter the following URL in the 'Additional Boards Manager URLs' field: https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

4. Hit OK to close the Dialog.

5. Go to Tools->Board->Board Manager in the IDE.

6. Type “RP2040” in the search box.

7. Locate the entry for 'Raspberry Pi Pico/RP2040 by Earle F. Philhower, III' and click 'Install'

### Installing the required libraries

1. From the Arduino IDE select (Tools > Manage Libraries)
2. Search for 'JTEncode' Scroll down to find the Etherkit JTEncode library by Jason Milldrum.
3. Click Install
4. Now search for 'TFT_eSPI' and find the TFT graphics library by Bodmer.
5. Click Install

#### Downloading the Software.

1. Download the latest released source code .zip file from https://github.com/g4eml/RP2040_Beacon_Decoder/releases

2. Save it to a convenient location and then unzip it. 

#### Programming the RP2040

1. Open the Arduino IDE and click File/Open

2. Navigate to the File RP2404Synth/RP2040_LCD_Synth.ino (downloaded in the previous step) and click Open.

3. Select Tools and make the following settings.

   Board: "Raspberry Pi Pico"
   
   Debug Level: "None"

   Debug Port: "Disabled"

   C++ Exceptions: "Disabled"

   Flash Size: "2Mb (no FS)"

   CPU Speed: "133MHz"

   IP/Bluetooth Stack: "IPV4 Only"

   Optimise: "Small (-Os)(standard)"

   RTTI: "Disabled"

   Stack Protection: "Disabled"

   Upload Method: "Default (UF2)"

   USB Stack: "Pico SDK"  

5. Connect the HMI Module to the USB port, hold down the BOOT button and briefly press the reset Button. 

6. Click Sketch/Upload.

The Sketch should compile and upload automatically to the Pico. If the upload fails you may need to disconnect the module and then hold down the BOOT button while reconnecting. 

## Connections



The firmware supports the optional connection of a GPS module. This is used to accurately set the time, which is needed for the JT4 and PI4 modes. Any GPS module with a 3V3 output can be used. It needs to output NMEA data at 9600 Baud. One of the low cost NEO6M modules was used for development. 

| HMI Module | GPS Module |
| :---:  |    :---:   |
|UART1 Pin 4    |Rx          |
|UART1 PIN3 |Tx          |
|UART1 Pin 2|3V3|
|UART1 Pin 1|GND|



## Firmware description


## 
