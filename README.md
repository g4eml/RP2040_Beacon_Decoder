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

## Operation Description

### Display
The display is split into 4 main areas. On the left are the Spectrum display and Waterfall. These are used to tune the reciever to the correct frequency. 

The Spectrum and Waterfall span from 300 Hz to 2200 Hz. Underneath the spectrum display there are 4 orange bands which indicate the correct frequencies for each of the 4 signal tones. The receiver needs to be tuned such that the received JT4G or PI4 tones fall withing these orange bands. 

Tone 0 for JT4G is set to 800 Hz. 

Tone 0 for PI4 is set to 684 Hz.

When signals are being received one of the orange bands will turn green to indicate which of the 4 tones is being detected.

A cyan horizontal line will be drawn across the Waterfall at the start of each 1 minute period. This indicates that signal capture has started and should occur just before the beacon starts to send its data. 

A red horizontal line will be drawn across the Waterfall when the message is being decoded. This will be at about 48 seconds for JT4G and at about 28 Seconds for PI4. If the decode is successfullt the decoded message will be displayed on the right hand side of the screen. 

The decoded message line will contain the time in Hours and Minutes. Then an estimate of the Signal to noise ratio. Then a Colon followed by the decoded message.  

### Controls

At the bottom of the screen there are 6 touch buttons. Only 4 of these are currently in use. 

CLR  Clears the Message display.

SYNC Performs a manual time synchronisation. The Seconds counter is reset to Zero and a new capture period is started.  This allows the module to be used without a GPS module. An accurate timesouce can be used and SYNC pressed at the exact start of the minute. Or alternatively it should be possible to manually synchronise with the start of the Beacons transmissions if they are audible.

JT4G Select the JT4G mode.

PI4 Selects the PI4 Mode.



## Hardware Requirements

This code is designed to work with the Elecrow CrowPanel Pico-3.5 inch 480x320 TFT LCD HMI Module. https://www.aliexpress.com/item/1005007250778536.html 

![IMG_20250128_153213](https://github.com/user-attachments/assets/226fca54-6678-45e9-b151-74a64b49ffde)

Note:- similar HMI Panels are available using the ESP32 processor chip. Make sure that you are purchasing the RP2040 version. 
 
Data modes such as JT4G and PI4 need accurate timing information. Each transmission is synchronised to the start of the minute. You can manuualy synchronise the decoder to an accurate timesource using the SYNC button or you can optionally connect a GPS module to provide automatic time synchronisation.  

![GPS](https://github.com/user-attachments/assets/53ceb650-0525-4138-a7ff-bff1f450c409)

## Connecting

The receiver is connected to the HMI module via a simple level shifting interface. Details are shown in this document [Connections](Documents/Beacon_Decoder.pdf)

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
2. Search for 'arduinoFFT' Scroll down to find the arduinoFFT library by Enrique Condes.
3. Click Install
4. Now search for 'TFT_eSPI' and find the TFT graphics library by Bodmer.
5. Click Install

#### Downloading the Software.

1. Download the latest released source code .zip file from https://github.com/g4eml/RP2040_Beacon_Decoder/releases

2. Save it to a convenient location and then unzip it. 

The TFT_eSPI Library is unusual in that it needs to be configured to the TFT display in use by modifying library files. 
The required modified versions of the files are located in the 'LCD-eSPI Settings' folder of this repository. 
Copy the files 'Setup_Elecraft_HMI.h' and 'User_Setup_Select.h' from  the downloaded 'LCD-eSPI Settings' folder to your Arduino libraries directory.
This will normaly be found at 'Documents/Arduino/libraries/TFT_eSPI'

#### Programming the RP2040

1. Open the Arduino IDE and click File/Open

2. Navigate to the File RP2404Synth/RP2040_Beacon_Decoder.ino (downloaded in the previous step) and click Open.

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

The receiver audio is connected using a simple CR network to GPIO Pins GND, 3V£ and 28 on the top edge of the HMI Module. 
Details of this interface are in this file.  ![Interface](Documents/Beacon_Decoder.pdf)

The firmware supports the optional connection of a GPS module. This is used to accurately set the time, which is needed for the JT4 and PI4 modes. Any GPS module with a 3V3 output can be used. It needs to output NMEA data at 9600 Baud. One of the low cost NEO6M modules was used for development. 

| HMI Module | GPS Module |
| :---:  |    :---:   |
|UART1 Pin 4    |Rx          |
|UART1 PIN3 |Tx          |
|UART1 Pin 2|3V3|
|UART1 Pin 1|GND|



## Firmware description

The firmware uses both cores of the RP2040 chip. 

### Core 0 handles the time critical functions. 

Audio is 32 times oversampled using the RP2040 built in ADC and the resulting samples are transfered to memory using DMA. 

The samples are then averaged to reduce noise and reduce the number of samples to 1024. 

The 1024 samples are then windowed and an FFT is performed on them. This results in 512 frequency Bins. 

The frequency bins around each of the 4 signaling tones are then compared to identify which of the 4 tones is present. The resulting tone number is stored for later decoding. 

This process is repeated for the entire duration of the JT4G or PI4 message.

Once the entire message has been captured the saved tone information is scanned to find the best match with the known embedded sync pattern.

If a reasonable sync match is found then the message bits are extracted and passed to the Fano Decoder which was written by KA9Q and K1JT. This analyses the received bits and applies error correction to try to reconstruct the original message. If the signal to noise ratio is good with minimal errors then this happens very quickly. A noisy signal takes longer but can often still be successfull. 

These decoding steps are similar to but not the same as those used by the WJST-x software.  The limited memory space and processing power of the RP2040 means that some compromises have had to be made. The decoding accuracy is pretty good but very weak signals are not decoded as well as WJST-x can do. 

### Core 1 of the RP2040 handles the LCD display and Touch Screen. 

The LCD display is quite slow to update so it makes sense to keep those delays away from Core 0.

Each set of FFT Bins is scaled and displayed as both a spectrum display and a waterfall. The decoded Messages are also displayed. 

The touch screen inputs are also processed. 

## 

