void PI4Init(void)
{
  sampleRate = PI4SAMPLERATE;                  //samples per second.
  toneSpacing = PI4TONESPACING;                //tone spacing in number of bins.
  tone0 = PI4TONE0;                            //tone zero in bins. 
  toneTolerance = PI4TONETOLERANCE;            //Tone tolerance in bins. 
  cacheSize = PI4CACHESIZE;                    // tone decode samples.
  symbolCount = PI4SYMBOLCOUNT;               //number of symbols message.
  bitCount = PI4BITCOUNT;                     //number of bits in Message.
  hzPerBin = PI4HZPERBIN;
  snBins = PI4SNBINS;

  calcLegend();
}


//Search the 30 seconds of Tone Cache to try to decode the FT4 message. 
bool PI4decodeCache(void)
{
uint8_t bestStartIndex;                     //start of the best received symbol pattern
uint8_t bits[bitCount];                 //storage for received bits
bool decodedOK = false;
unsigned char dec[9];                      //decoded message before unpacking

  bestStartIndex = PI4findSync();              //search the received symbols for the best match to the sync vector
  PI4extractBits(bestStartIndex, bits);        //extract the best match found and store the bits, one bit per byte
  PI4deInterleave(bits);                       //deinterleave the bits
  decodedOK = decodePI4(bits,dec);          //decode using the fano algorithm to error correct from the received bits.
  if(decodedOK)                             ///if it looks like a successful decode
   {
    PI4unpack(dec);                            //unpack back to the original message
   }  

  return decodedOK;
}

//search the received symbol cache and return the index of the best match to the sync vector
uint8_t PI4findSync()
{
uint8_t syncErrorCount;
uint8_t bestErrorCount;
uint8_t bestStartIndex;
uint8_t equalBestCount;
bestErrorCount = 255;
/*
Serial.println("Cache");
for(int i = 0;i<cacheSize;i++)
{
  Serial.print(toneCache[i]);
  Serial.print(" ");
  if(i%50 == 0) Serial.println();
}
Serial.println();
*/

for(int i = 0; i < cacheSize - (symbolCount) ; i++)       //starting index for the Sync test. No point in testing after the first few seconds as the whole message wont have been captured. 
 {
  syncErrorCount = 0;
  for(int s = 0; s < symbolCount; s++)          //test each symbol in the JT4syncVector against the received symbols. Sync pattern is in bit 0 of symbol
   {
      if(PI4syncVector[s] != (toneCache[i + s] & 0x01) )    syncErrorCount++;
   }

  if(syncErrorCount < bestErrorCount)                     //Have we found a better match to the sync vector?
   {
    bestStartIndex = i;                                  //save this start index as the best found so far.
    bestErrorCount = syncErrorCount;
   }
 }

  Serial.print("Sync Match at ");
  Serial.print((float) bestStartIndex * 0.0555);
  Serial.print(" Seconds. Sync Errors = ");
  Serial.println(bestErrorCount);
    
  Serial.print("Symbols ");
  for(int i = 0;i < symbolCount;i++)
    {
      Serial.print(toneCache[bestStartIndex + i]);
      Serial.print(" ");
    }
  Serial.println();
 

  return bestStartIndex;
}

// extract the bits from bit 1 of the symbols.
void PI4extractBits(uint8_t bestStartIndex, uint8_t *bits)
{ 
for(int i = 0; i < bitCount; i++)
   {
     bits[i] = (toneCache[bestStartIndex + i] >> 1);
   }

  Serial.print("Interleaved Bits = ");
  for(int i = 0; i < bitCount; i++)
   {
     Serial.print(bits[i]);
     Serial.print(" ");
   }
  Serial.println();
}

void PI4deInterleave(uint8_t *bits)
{
  uint8_t d[bitCount];           //temporary aray for de-interleaving

  for(int i = 0; i < bitCount; i++)
  {
    d[pi4di[i]] = bits[i];
  }
  memcpy(bits, d, bitCount);
 
   Serial.print("De-Interleaved Bits = ");
  for(int i = 0; i < bitCount; i++)
   {
     Serial.print(bits[i]);
     Serial.print(" ");
   }
  Serial.println();
}

bool decodePI4(uint8_t *bits, unsigned char *dec)
{
 //decode using Fano Algorithm
  unsigned int metric;
  unsigned long cycles;
  unsigned int maxnp;

  for(int i =0;i<bitCount;i++)
   {
    bits[i]=bits[i]*255;
   }


 int notDecoded = fano(&metric,&cycles,&maxnp, dec, bits, 74,42,20000);

  Serial.print("Decoded = ");
  if(notDecoded) 
  {
    Serial.print("No.");
  }
  else 
  {
    Serial.print("Yes.");
  }
  Serial.print(" Metric = ");
  Serial.print(metric);
  Serial.print(" Cycles = ");
  Serial.print(cycles);
  Serial.print(" Maxnp = ");
  Serial.println(maxnp);

  Serial.print("Packed Message = ");
  for (int i =0; i<10;i++)
   {
    Serial.print(dec[i]);
    Serial.print(" ");
   }
   Serial.println();

 return !notDecoded;

}

 //Unpack message

void PI4unpack(unsigned char *dec)
{
  uint64_t N =0;

  for(int i =0 ; i<8 ; i++)
  {
    N = N << 8;
    N=N + dec[i];
  }

  N = N >> 22;

  Serial.print("Encoded = ");
  Serial.println(N);

  for(int i = 7 ; i>= 0 ; i--)
  {
    PImessage[i] = N % 38;
    N=N/38;
  }

  PImessage[8] = 0;    //terminate the string
  //convert back to ascii

  for(int i = 0; i<8 ; i++)
   {
    switch(PImessage[i])
    {
      case 0 ... 9:
      PImessage[i] = PImessage[i] + '0';
      break;
      case 10 ... 35:
      PImessage[i] = PImessage[i] + 'A' - 10;
      break;
      case 36:
      PImessage[i] = ' ';
      break;
      case 37:   
      PImessage[i] = '/';
    }
   }

    Serial.print("Message = ");
    Serial.println(PImessage);
}