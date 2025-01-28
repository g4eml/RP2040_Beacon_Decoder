//This file contains the FFT functions used to frequency sample the incoming audio


//Perform an FFT on the ADC sample buffer. Calculate the magnitude of each frequency bin. Results are in the first half of the vReal[] array. 
void calcSpectrum(void)
{
  int bin;
  for(int i = 0;i < (NUMBEROFSAMPLES); i=i+OVERSAMPLE)                       //for each of the samples
  {
    bin = i/OVERSAMPLE;
    vReal[bin]=0;
    for(int s=0;s<OVERSAMPLE;s++)
    {
      vReal[bin] += buffer[bufIndex][i+s] - 2048;     //average the samples and copy the result into the Real array. Offsetting to allow for ADC bias point
    }
    vReal[bin] = vReal[bin]/OVERSAMPLE;
    vImag[bin] = 0;                                            //Set the Imaginary array to zero.
  }

  if(mode == JT4)
  {
    JT4FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);     //weight the data to reduce discontinuities. Hann window seems to work the best.
    JT4FFT.compute(FFTDirection::Forward);                        //calculate the FFT. Results are now in the vReal and vImag arrays.
    JT4FFT.complexToMagnitude();                                  //calculate the magnitude of each bin. FFT magnitude results are now in the first half of the vReal[] array.
  }

  if(mode == PI4)
  {
    PI4FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);     //weight the data to reduce discontinuities. Hann window seems to work the best.
    PI4FFT.compute(FFTDirection::Forward);                        //calculate the FFT. Results are now in the vReal and vImag arrays.
    PI4FFT.complexToMagnitude();                                  //calculate the magnitude of each bin. FFT magnitude results are now in the first half of the vReal[] array.
  }
}


//Generate the display output array from the vReal FFT array with log scaling. Add offset and gain to the values.
void generatePlotData(void)
{
  double db[PLOTPOINTS];
  double vref = 2048.0;
  static double baselevel;
  float plotIncrement;
  float freq;
  int bin;

  plotIncrement = (SPECHIGH - SPECLOW) / (float) PLOTPOINTS;

    if(autolevel)
    {
    baselevel = 0;
    }

    freq = SPECLOW - plotIncrement;

    for(int p =0;p < PLOTPOINTS; p++)                         
    {
      freq = freq + plotIncrement;
      bin = freq / hzPerBin;                                      //calculate the bin number containing this frequency
      db[p]=2*(20*(log10(vReal[bin] / vref)));               //calculate bin amplitude relative to FS in dB
 
    if(autolevel)
      {
      baselevel = baselevel + db[p];
      }  
    }
    
    if(autolevel)
    {
      baselevel = baselevel/PLOTPOINTS;                             //use the average level for the baseline.
    }

    for(int p=0;p<PLOTPOINTS;p++)
    {
      plotData[p]= uint8_t (db[p] - baselevel);  
    }
 
}



//Search the averaged FFT Bins for the given tone. Return the maximum Magnitude found, its associated bin number and its s/n relative to the adjacent bins.
void findMax(int tone, double * maxval, double * sn)
{
  double tstval=0;
  double avg=0;
  uint16_t binnumber=0;

  // find the bin in the tolerance range with the highest signal.
  for(int i = 0 ; i < (1+toneTolerance *2) ; i++ )
  {
    int bin = tone0 + tone*toneSpacing - toneTolerance + i;
    if(vReal[bin] > tstval)
     {
      tstval=vReal[bin];
      binnumber = bin;
     }
  }
  
  //average a few bins either side of the tone to get a noise floor
   for(int i =0;i < 4;i++ )
   {
    int bin = tone0 + tone * toneSpacing - toneTolerance - 5 + i;  //below the current tone band
    avg = avg + vReal[bin];
    bin = tone0 + tone * toneSpacing + toneTolerance + 1 + i;  //above the current tone band 
    avg = avg + vReal[bin];
   }

  avg = avg /8;

  *maxval = vReal[binnumber];
  *sn = vReal[binnumber] / avg;                  //calculate and return the S/N ratio for the max tone.
}

//Find the FFT bin for each of the 4 tones and then determine which tone has the largest signal. Return the corresponding tone index 0 - 4. 
uint8_t toneDetect(void)
{
  double toneMag[4];
  uint16_t toneBin[4];
  double sn[4];
  double signz;
  uint8_t tone;
  double mx;

  //search around each averaged tone to find largest magnitude and s/n 
  for(int i =0; i<4; i++)
    {
      findMax( i, &toneMag[i], &sn[i]);
    }


  //find the tone with the best s/n (using s/n compensates for any amplitude slew between tones.)
  mx = 0;
  for(int i =0;i<4;i++)
    {
      if(sn[i] > mx)
       {
        mx=sn[i];
        tone = i;
       }
    }
  
  signz = 10 * log10(mx / snBins);         //calculate 2.5KHz s/n for this signal . 
  if(signz > sigNoise) sigNoise = signz;      //and record the best value

  return tone;
}
