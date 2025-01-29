// JT4 / PI4 Decoder
// Colin Durbridge G4EML 2024


#include <hardware/dma.h>
#include <hardware/adc.h>
#include "hardware/irq.h"
#include "arduinoFFT.h"
#include <EEPROM.h>
#include <TFT_eSPI.h>                 // Hardware-specific library. Must be pre-configured for this display and touchscreen
#include "DEFINES.h"                  //include the defines for this project
#include "globals.h"                  //global variables

TFT_eSPI tft = TFT_eSPI();            // Invoke custom library
 
ArduinoFFT<double> JT4FFT = ArduinoFFT<double>(vReal, vImag, NUMBEROFBINS, JT4SAMPLERATE);         //Declare FFT function
ArduinoFFT<double> PI4FFT = ArduinoFFT<double>(vReal, vImag, NUMBEROFBINS, PI4SAMPLERATE);         //Declare FFT function

//Run once on power up. Core 0 does the time critical work. Core 1 handles the GUI.  
void setup() 
{
    Serial.begin();                   //enable the debug serial port
    EEPROM.begin(256);
    if(EEPROM.read(0) == 0x55) 
      {
        mode = EEPROM.read(1);
      }
    else
      {
        mode == JT4;
      }  

    if(mode == PI4)
    {
    PI4Init();
    }
  if(mode == JT4)
    {
    JT4Init();
    }

    dma_init();                       //Initialise and start ADC conversions and DMA transfers. 
    dma_handler();                    //call the interrupt handler once to start transfers
    dmaReady = false;                 //reset the transfer ready flag
    cachePoint = 0;                   //zero the data received cache
    lastmin = 0;                      //and the timer
    sigNoise = -100.00;

    loopTimer = millis() + 1000;
}

//core 1 handles the GUI
void setup1()
{
  Serial2.setRX(GPSRXPin);              //Configure the GPIO pins for the GPS module
  Serial2.setTX(GPSTXPin);
  Serial2.begin(9600);                  //start GPS port comms
  gpsPointer = 0;
  waterRow = 0;

  initGUI();                        //initialise the GUI screen
}


//Main Loop Core 0. Runs forever. Does most of the work.
void loop() 
{
  uint8_t tn;

  if(minutes != lastmin)                                         //every minute
    {
      lastmin = minutes;                                        //Save for next minute
      dmaActive = true;                                         //Enable DMA transfers. 
      cachePoint =0;                                            //Reset the sample cache ready for another period of received data
      rp2040.fifo.push(CYANLINE);                               //Ask core 1 to draw a Cyan horizontal line in the spectrum to mark the start of the minute.  
      sigNoise = -100.00; 
    }
   
   if(dmaReady)                                                 //Do we have a complete buffer of ADC samples ready?
    {
      calcSpectrum();                                           //Perform the FFT of the data
      rp2040.fifo.push(GENPLOT);                                //Ask Core 1 to generate data for the Displays from the FFT results.  
      rp2040.fifo.push(DRAWSPECTRUM);                           //Ask core 1 to draw the Spectrum Display
      rp2040.fifo.push(DRAWWATERFALL);                          //Ask core 1 to draw the Waterfall Display      
      tn=toneDetect();                                          //Detect which tone is present

      if(dmaActive)
       {
        toneCache[cachePoint] = tn;                     //Add the tone index to the Tone cache

        rp2040.fifo.push(SHOWTONE + toneCache[cachePoint++]);     //Ask Core 1 to highlight the current tone. 

        if(cachePoint == cacheSize)                               //If the Cache is full (54 seconds of tones)
          {
            rp2040.fifo.push(REDLINE);                            //Ask core 1 to draw a Red horizontal line in the spectrum to mark the end of the Rx Period.
            dmaActive = false;                                    //Ignore DMA transfers while we are decoding. 
            cachePoint =0;                                        //Reset ready for the next period 
            if(mode == JT4)
            {
              if(JT4decodeCache())                                     //Try to extract the JT4 message from the 54 seconds of Tone cache.
                {
                rp2040.fifo.push(JTMESSAGE);                         //Successfull decode. Ask Core 1 to display the received message  
                }  
            }

            if(mode == PI4)
            {
              if(PI4decodeCache())                                     //Try to extract the JT4 message from the 54 seconds of Tone cache.
                {
                  rp2040.fifo.push(PIMESSAGE);                         //Successfull decode. Ask Core 1 to display the received message 
                }  
            }
          }                               
       }    
      dmaReady = false;                                         //Clear the flag ready for next time     
    }
}


//Core 1 handles the GUI. Including synchronising to GPS if available
void loop1()
{
  uint32_t command;
  char m[64];
  unsigned long inc;
 
  if(millis() >= loopTimer)
    {         
      loopTimer = loopTimer + 1000;
      seconds++;
      if(seconds == 60)
        {
          minutes++;
          seconds = 0;
          if(minutes == 60)
            {
              hours++;
              minutes = 0;
              if(hours == 24)
                {
                  hours = 0;
                }
            }
        } 
      showTime();                                   //display the time      
    }


  if(rp2040.fifo.pop_nb(&command))          //have we got something to process from core 0?
    {
      switch(command)
      {
        case GENPLOT:
        generatePlotData();
        break;
        case DRAWSPECTRUM:
        drawSpectrum();
        break;
        case DRAWWATERFALL:
        drawWaterfall();
        break;
        case REDLINE:
        markWaterfall(TFT_RED);
        break;
        case CYANLINE:
        markWaterfall(TFT_CYAN);
        break;
        case SHOWTONE:
        showTone(0);
        break;
        case SHOWTONE1:
        showTone(1);
        break;
        case SHOWTONE2:
        showTone(2);
        break;
        case SHOWTONE3:
        showTone(3);
        break;
        case JTMESSAGE:
        sprintf(m,"%02d:%02d %.0lf :%s",hours,minutes, sigNoise,JTmessage);
        textPrint(m);                                 
        textLine(); 
        break;
        case PIMESSAGE:
        sprintf(m,"%02d:%02d %.0lf :%s",hours,minutes, sigNoise,PImessage);
        textPrint(m);                                 
        textLine(); 
        break;
      }
    }


  if(screenTouched())
    {
      processTouch();
    } 
//synchronise the local clock to the GPS clock if available
  int secErr = abs(seconds - gpsSec);
  if((gpsSec != -1) && (gpsActive) && (secErr > 1) && (secErr < 58))
         {
          Serial.println("Adjusting Clock");
          loopTimer = millis() + 1000;
          seconds = gpsSec;
          minutes = gpsMin;
          hours = gpsHr;
          gpsActive = false;
         }

  if(Serial2.available() > 0)           //data received from GPS module
      {
        while(Serial2.available() >0)
          {
            gpsCh=Serial2.read();
            if(gpsCh > 31) gpsBuffer[gpsPointer++] = gpsCh;
            if((gpsCh == 13) || (gpsPointer > 255))
              {
                gpsBuffer[gpsPointer] = 0;
                processNMEA();
                gpsPointer = 0;
              }
          }

      }
}


void processNMEA(void)
{
  float gpsTime;

 gpsActive = true;
 if(strstr(gpsBuffer , "RMC") != NULL)                         //is this the RMC sentence?
  {
    if(strstr(gpsBuffer , "A") != NULL)                       // is the data valid?
      {
       int p=strcspn(gpsBuffer , ",");                         // find the first comma
       gpsTime = strtof(gpsBuffer+p+1 , NULL);                 //copy the time to a floating point number
       gpsSec = int(gpsTime) % 100;
       gpsTime = gpsTime / 100;
       gpsMin = int(gpsTime) % 100; 
       gpsTime = gpsTime / 100;
       gpsHr = int(gpsTime) % 100;         
      }
    else
     {
       gpsSec = -1;                                            //GPS time not valid
       gpsMin = -1;
       gpsHr = -1;
     }
  }


}

