// This file contains the functions to decode the received JT4 symbols. 

void JT4Init(void)
{
  sampleRate = JT4SAMPLERATE;                  //samples per second.
  toneSpacing = JT4TONESPACING;                //tone spacing in number of bins.
  tone0 = JT4TONE0;                            //tone zero in bins. 
  toneTolerance = JT4TONETOLERANCE;            //Tone tolerance in bins. 
  cacheSize = JT4CACHESIZE;                    // tone decode samples.
  symbolCount = JT4SYMBOLCOUNT;               //number of symbols message.
  bitCount = JT4BITCOUNT;                     //number of bits in Message. 
  hzPerBin = JT4HZPERBIN;
  snBins = JT4SNBINS;

  calcLegend();
}

//Search the 54 seconds of Tone Cache to try to decode the FT4 message. 
bool JT4decodeCache(void)
{
uint8_t bestStartIndex;                     //start of the best received symbol pattern
uint8_t bits[bitCount];                 //storage for received bits
bool decodedOK = false;
unsigned char dec[14];                      //decoded message before JT4unpacking

  bestStartIndex = JT4findSync();              //search the received symbols for the best match to the sync vector
  JT4extractBits(bestStartIndex, bits);        //extract the best match found and store the bits, one bit per byte
  JT4deInterleave(bits);                       //deinterleave the bits
  decodedOK = decodeFT4(bits,dec);          //decode using the fano algorithm to error correct from the received bits.
  if(decodedOK)                             ///if it looks like a successful decode
   {
    JT4unpack(dec);                            //JT4unpack back to the original JT4 message
   }  

  return decodedOK;
}

//search the received symbol cache and return the index of the best match to the sync vector
uint8_t JT4findSync()
{
uint8_t syncErrorCount;
uint8_t bestErrorCount;
uint8_t bestStartIndex;
uint8_t equalBestCount;
bestErrorCount = 255;

for(int i = 0; i < cacheSize - (symbolCount * overlap) ; i++)       //starting index for the Sync test. No point in testing after the first few seconds as the whole message wont have been captured. 
 {
  syncErrorCount = 0;
  for(int s = 0; s < symbolCount; s++)          //test each symbol in the JT4syncVector against the received symbols. Sync pattern is in bit 0 of symbol
   {
      if(JT4syncVector[s] != (toneCache[i + s * overlap] & 0x01) )    syncErrorCount++;
   }

  if(syncErrorCount < bestErrorCount)                     //Have we found a better match to the sync vector?
   {
    bestStartIndex = i;                                  //save this start index as the best found so far.
    bestErrorCount = syncErrorCount;
   }
 }

#ifdef DEBUG
  Serial.print("Sync Match at ");
  Serial.print((float) bestStartIndex * 0.0762);
  Serial.print(" Seconds. Sync Errors = ");
  Serial.println(bestErrorCount);
    
  Serial.print("Symbols ");
  for(int i = 0;i < symbolCount;i++)
    {
      Serial.print(toneCache[bestStartIndex + i * overlap]);
      Serial.print(" ");
    }
  Serial.println();
 
  if((toneCache[bestStartIndex] >> 1) != 0)
   {
     Serial.println("Possible Error. First received bit was not a zero");
   }
#endif

  return bestStartIndex;
}

// extract the bits from bit 1 of the symbols. Check and Ignore first bit which should be a zero.
void JT4extractBits(uint8_t bestStartIndex, uint8_t *bits)
{ 
for(int i = 0; i < bitCount; i++)
   {
     bits[i] = (toneCache[bestStartIndex + i* overlap + overlap] >> 1);
   }

#ifdef DEBUG
  Serial.print("Interleaved Bits = ");
  for(int i = 0; i < bitCount; i++)
   {
     Serial.print(bits[i]);
     Serial.print(" ");
   }
  Serial.println();
#endif
}

void JT4deInterleave(uint8_t *bits)
{
  uint8_t d[bitCount];           //temporary aray for de-interleaving

  for(int i = 0; i < bitCount; i++)
  {
    d[jt4di[i]] = bits[i];
  }
  memcpy(bits, d, bitCount);
 
 #ifdef DEBUG
   Serial.print("De-Interleaved Bits = ");
  for(int i = 0; i < bitCount; i++)
   {
     Serial.print(bits[i]);
     Serial.print(" ");
   }
  Serial.println();
#endif
}

bool decodeFT4(uint8_t *bits, unsigned char *dec)
{
 //decode using Fano Algorithm
  unsigned int metric;
  unsigned long cycles;
  unsigned int maxnp;

  for(int i =0;i<bitCount;i++)
   {
    bits[i]=bits[i]*255;
   }


 int notDecoded = fano(&metric,&cycles,&maxnp, dec, bits, 104,60,20000);

#ifdef DEBUG
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
  for (int i =0; i<13;i++)
   {
    Serial.print(dec[i]);
    Serial.print(" ");
   }
   Serial.println();
#endif

 return !notDecoded;

}

 //Unpack message

void JT4unpack(unsigned char *dec)
{
  uint32_t n1 , n2 , n3;

  n3 = (dec[7] << 8) + dec[8];
  n2 = ((dec[3] & 0x0F) << 24) + (dec[4] << 16) + (dec[5] <<8 ) + dec[6];
  n1 = (dec[0] << 20) + (dec[1] << 12) + (dec[2] << 4) + (dec[3] >> 4);

  n3 = (n3 & 0x7FFF) | ((n1 & 0x01) <<15) | ((n2 & 0x01) << 16);
  n2 = n2 >> 1;
  n1 = n1 >> 1;

  JTmessage[4] = n1 % 42;
  n1 = n1 / 42;
  JTmessage[3] = n1 % 42;
  n1 = n1 / 42;
  JTmessage[2] = n1 % 42;
  n1 = n1 / 42;  
  JTmessage[1] = n1 % 42;
  n1 = n1 / 42;
  JTmessage[0] = n1;  

  JTmessage[9] = n2 % 42;
  n2 = n2 / 42;
  JTmessage[8] = n2 % 42;
  n2 = n2 / 42;
  JTmessage[7] = n2 % 42;
  n2 = n2 / 42;  
  JTmessage[6] = n2 % 42;
  n2 = n2 / 42;
  JTmessage[5] = n2; 

  JTmessage[12] = n3 % 42;
  n3 = n3 / 42;  
  JTmessage[11] = n3 % 42;
  n3 = n3 / 42;
  JTmessage[10] = n3; 

  JTmessage[13] = 0;    //terminate the string
  //convert back to ascii

  for(int i = 0; i<13 ; i++)
   {
    switch(JTmessage[i])
    {
      case 0 ... 9:
      JTmessage[i] = JTmessage[i] + '0';
      break;
      case 10 ... 35:
      JTmessage[i] = JTmessage[i] + 'A' - 10;
      break;
      case 36:
      JTmessage[i] = ' ';
      break;
      case 37:
      JTmessage[i] = '+';
      break;
      case 38:
      JTmessage[i] = '-';
      break;
      case 39:
      JTmessage[i] = '.';
      break;
      case 40:
      JTmessage[i] = '/';
      break;
      case 41:
      JTmessage[i] = '?';
      break;
    }
   }

#ifdef DEBUG
    Serial.print("Message = ");
    Serial.println(JTmessage);
#endif
}
