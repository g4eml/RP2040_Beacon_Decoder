//This file contains the functions for the Graphical User Interface

void initGUI(void)
{
  tft.init();                       //initialise the TFT display hardware
  tft.setRotation(1);               //Set the TFT to Landscape
  tft.fillScreen(TFT_BLACK);        //Clear the TFT
  touch_calibrate(0);
  drawButtons();
  tft.fillRect(SPECLEFT,SPECTOP,SPECWIDTH,SPECHEIGHT,TFT_CYAN);   //Create background for the Spectrum Display
  drawLegend();                     //draw spectrum legend
  textClear();
  textLine();
}


//Add a line to the Waterfall Display
void drawWaterfall(void)
{
  if(waterRow < WATERHEIGHT-1) tft.drawFastHLine(WATERLEFT,WATERTOP + waterRow + 1,WATERWIDTH,TFT_WHITE);       //Draw White line acrost the Waterfall to highlight the current position.
  for(int p=0 ; p < PLOTPOINTS ; p++)                                              //for each of the data points in the current row
  {
    tft.drawPixel(WATERLEFT + p,WATERTOP + waterRow, waterColours[(plotData[p] + 10) *2]);             //draw a pixel of the required colour
  } 
  waterRow++;                                                                      //Increment the row for next time
  if(waterRow >= WATERHEIGHT) waterRow = 0;                                        //Cycle back to the start at the end of the display. (would be nice to scroll the display but this is too slow)
}

void markWaterfall(unsigned int col)
{
  tft.drawFastHLine(WATERLEFT,WATERTOP + waterRow -1,WATERWIDTH,col); 
}

//Draw the Spectrum Display
void drawSpectrum(void)
{
  for(int p=1 ; p < PLOTPOINTS ; p++)                                             //for each of the data points in the current row
  {
    tft.drawLine(SPECLEFT + p - 1, SPECTOP + SPECHEIGHT - lastplotData[p-1], SPECLEFT + p, SPECTOP + SPECHEIGHT - lastplotData[p], TFT_CYAN);   //erase previous plot
    tft.drawLine(SPECLEFT + p - 1, SPECTOP + SPECHEIGHT - plotData[p-1], SPECLEFT + p, SPECTOP + SPECHEIGHT - plotData[p], TFT_RED);            //draw new plot
  }
  memcpy(lastplotData , plotData, PLOTPOINTS);       //need to save this plot so that we can erase it next time (faster than clearing the screen)
}

void textClear(void)
{
  tft.fillRect(TEXTLEFT, TEXTTOP, TEXTWIDTH, TEXTHEIGHT, TFT_WHITE);
  textrow = 0;
}

void textPrint(const char* message)
{
 if(textrow > (TEXTTOP + TEXTHEIGHT - tft.fontHeight()))
    {
      textClear();
    }
  tft.setTextColor(TFT_BLUE);
  tft.setFreeFont(&FreeSans9pt7b);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(message,TEXTLEFT,TEXTTOP+textrow);
  textrow=textrow + tft.fontHeight();
}

void textLine(void)
{
  if(textrow > 3)
  {
    tft.drawFastHLine(TEXTLEFT,textrow-3,TEXTWIDTH,TFT_BLUE);
  }
}

void drawButtons(void)
{

  tft.setTextColor(TFT_BLUE);
  tft.setTextDatum(TL_DATUM);
  tft.setFreeFont(&FreeSans9pt7b);

  tft.fillRect(BUT1LEFT, BUT1TOP, BUT1WIDTH, BUT1HEIGHT, TFT_CYAN);
  tft.drawRect(BUT1LEFT, BUT1TOP, BUT1WIDTH, BUT1HEIGHT, TFT_WHITE);
  tft.drawString("CLR",BUT1LEFT +15,BUT1TOP +15);

  tft.fillRect(BUT2LEFT, BUT2TOP, BUT2WIDTH, BUT2HEIGHT, TFT_CYAN);
  tft.drawRect(BUT2LEFT, BUT2TOP, BUT2WIDTH, BUT2HEIGHT, TFT_WHITE);
  tft.drawString("SYNC",BUT2LEFT +10,BUT2TOP +15);

  tft.fillRect(BUT3LEFT, BUT3TOP, BUT3WIDTH, BUT3HEIGHT, TFT_CYAN);
  tft.drawRect(BUT3LEFT, BUT3TOP, BUT3WIDTH, BUT3HEIGHT, TFT_WHITE);
  tft.drawString("",BUT3LEFT +10,BUT3TOP +15);

  tft.fillRect(BUT4LEFT, BUT4TOP, BUT4WIDTH, BUT4HEIGHT, TFT_CYAN);
  tft.drawRect(BUT4LEFT, BUT4TOP, BUT4WIDTH, BUT4HEIGHT, TFT_WHITE);
  tft.drawString("",BUT4LEFT +10,BUT4TOP +15);

  if(mode == JT4)
  {
   tft.fillRect(BUT5LEFT, BUT5TOP, BUT5WIDTH, BUT5HEIGHT, TFT_GREEN);
  }
  else 
  {
   tft.fillRect(BUT5LEFT, BUT5TOP, BUT5WIDTH, BUT5HEIGHT, TFT_CYAN);
  }

  tft.drawRect(BUT5LEFT, BUT5TOP, BUT5WIDTH, BUT5HEIGHT, TFT_WHITE);
  tft.drawString("JT4G",BUT5LEFT +10,BUT5TOP +15);

  if(mode == PI4)
  {
    tft.fillRect(BUT6LEFT, BUT6TOP, BUT6WIDTH, BUT6HEIGHT, TFT_GREEN);
  }
  else 
  {
   tft.fillRect(BUT6LEFT, BUT6TOP, BUT6WIDTH, BUT6HEIGHT, TFT_CYAN); 
  }
  tft.drawRect(BUT6LEFT, BUT6TOP, BUT6WIDTH, BUT6HEIGHT, TFT_WHITE);
  tft.drawString("PI4",BUT6LEFT +15,BUT6TOP +15);
}

void touch_calibrate(bool force)
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check if calibration exists
  if (EEPROM.read(255) == 0x55) 
    {   
      EEPROM.get(200,calData);
      calDataOK = 1;
    }

  if (calDataOK && !REPEAT_CAL && !force)
  {
    // calibration data valid
    tft.setTouch(calData);
  } 
  else 
  {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) 
    {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data at the top of the EEPROM
    EEPROM.put(200,calData);
    EEPROM.write(255,0x55);
    EEPROM.commit();
  }

}

bool screenTouched(void)
 {
  uint16_t raw = tft.getTouchRawZ();
  if(raw > 1000)
  {
    bool pressed =tft.getTouch(&t_x, &t_y);
    return pressed;
  }
  else
  {
    noTouch = true;
    return false;
  }   
 }

 void processTouch(void)
 {
   if(touchZone(BUT1LEFT, BUT1TOP, BUT1WIDTH, BUT1HEIGHT) && noTouch)
    {
      noTouch = false;
      textClear();
      return;
    }

   if(touchZone(BUT2LEFT, BUT2TOP, BUT2WIDTH, BUT2HEIGHT)&& noTouch)
    {
      noTouch = false;
      seconds = 0;
      minutes = 0;
      hours = 0;
      lastmin = -1;
      return;
    }

  if(touchZone(BUT5LEFT, BUT5TOP, BUT5WIDTH, BUT5HEIGHT)&& noTouch)
    {
      noTouch = false;
      mode = JT4;
      EEPROM.write(0,0x55);
      EEPROM.write(1,mode);
      EEPROM.commit();
      rp2040.reboot();
      return;
    }

  if(touchZone(BUT6LEFT, BUT6TOP, BUT6WIDTH, BUT6HEIGHT)&& noTouch)
    {
      noTouch = false;
      mode = PI4;
      EEPROM.write(0,0x55);
      EEPROM.write(1,mode);
      EEPROM.commit();
      rp2040.reboot();
      return;
    }

   if(touchZone(SPECLEFT, SPECTOP, SPECWIDTH/2, SPECHEIGHT)&& noTouch)
    {
      noTouch = false;
      autolevel = false;
      return;
    }

   if(touchZone(SPECLEFT + SPECWIDTH/2, SPECTOP, SPECWIDTH/2, SPECHEIGHT)&& noTouch)
    {
      noTouch = false;
      autolevel = true;
      return;
    }

 }

 bool touchZone(int x, int y, int w, int h) 
{
  return ((t_x > x) && (t_x < x + w) && (t_y > y) && (t_y < y + h));
}

void drawLegend(void)
{
  tft.fillRect(LEGLEFT,LEGTOP,LEGWIDTH,LEGHEIGHT, TFT_WHITE);
  tft.fillRect(toneLegend[0][0], LEGTOP, 1 + toneLegend[0][1] , LEGHEIGHT , TFT_ORANGE);
  tft.fillRect(toneLegend[1][0], LEGTOP, 1 + toneLegend[1][1] , LEGHEIGHT , TFT_ORANGE);
  tft.fillRect(toneLegend[2][0], LEGTOP, 1 + toneLegend[2][1] , LEGHEIGHT , TFT_ORANGE);
  tft.fillRect(toneLegend[3][0], LEGTOP, 1 + toneLegend[3][1] , LEGHEIGHT , TFT_ORANGE);
}

void showTone(uint8_t tone)
{
  static uint8_t lastTone = 0;

  if(tone != lastTone)
    {
      tft.fillRect(toneLegend[lastTone][0], LEGTOP,toneLegend[lastTone][ 1], LEGHEIGHT , TFT_ORANGE);
      tft.fillRect(toneLegend[tone][0], LEGTOP,toneLegend[tone][1], LEGHEIGHT , TFT_GREEN);
      lastTone = tone;     
    }
}

void calcLegend(void)
{
    for(int t =0;t<4;t++)                       //for each tone pre-caculate the start and end pixel for the tone legend. 
  {
    int point;
    int width;
    float freq;
    float plotIncrement;
    plotIncrement = (SPECHIGH - SPECLOW) / (float) PLOTPOINTS;

    freq = hzPerBin * ((float) tone0 + ((float)t * (float)toneSpacing) - (float)toneTolerance);
    freq = freq - SPECLOW;
    point = freq / plotIncrement;
    toneLegend[t][0] = point;
    freq = hzPerBin * ((float) tone0 + ((float)t * (float)toneSpacing) + (float)toneTolerance);
    freq = freq - SPECLOW;
    point = (freq / plotIncrement) - point;
    toneLegend[t][1] = point;
  }
}
