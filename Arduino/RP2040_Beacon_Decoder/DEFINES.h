#define GPSTXPin 4                      //Serial data to GPS module 
#define GPSRXPin 5                      //SeriaL data from GPS module

#define REPEAT_CAL false              // Set REPEAT_CAL to true instead of false to run calibration again, otherwise it will only be done once.
#define PLOTPOINTS 234                //Number of FFT points to display = 234 * 13.125 = 3071 Hz for JT4   234*18 = 4212 Hz for PI4
#define SPECLOW 300.00                //spectrum display low frequency
#define SPECHIGH 2200.00              //spectrum display High Frequency

#define SPECLEFT 0                    //Spectrum Display Left Edge in Pixels
#define SPECTOP 0                     //Spectrum Display Top Edge in Pixels
#define SPECWIDTH PLOTPOINTS          //Spectrum Width in Pixels 
#define SPECHEIGHT 100                //Spectrum Height in Pixels

#define LEGLEFT 0                     //Legend for spectrum display
#define LEGTOP 100
#define LEGWIDTH PLOTPOINTS
#define LEGHEIGHT 10

#define WATERLEFT 0                   //Waterfall Display Left Edge in Pixels
#define WATERTOP 110                  //Waterfall Display Top Edge in Pixels
#define WATERWIDTH PLOTPOINTS         //Waterfall Disply Width in Pixels
#define WATERHEIGHT 160               //Waterfall Diaply Height in Pixels

#define TEXTLEFT PLOTPOINTS + 5      //left edge of text output area
#define TEXTTOP 0                    //top edge of text output area
#define TEXTWIDTH 241                //width of text output area
#define TEXTHEIGHT 270               //height of text output area

#define BUT1LEFT 5                  //Button locations
#define BUT1TOP 280
#define BUT1WIDTH 70
#define BUT1HEIGHT 40

#define BUT2LEFT 85                  //Button locations
#define BUT2TOP 280
#define BUT2WIDTH 70
#define BUT2HEIGHT 40

#define BUT3LEFT 165                  //Button locations
#define BUT3TOP 280
#define BUT3WIDTH 70
#define BUT3HEIGHT 40

#define BUT4LEFT 245                  //Button locations
#define BUT4TOP 280
#define BUT4WIDTH 70
#define BUT4HEIGHT 40

#define BUT5LEFT 325                  //Button locations
#define BUT5TOP 280
#define BUT5WIDTH 70
#define BUT5HEIGHT 40

#define BUT6LEFT 405                  //Button locations
#define BUT6TOP 280
#define BUT6WIDTH 70
#define BUT6HEIGHT 40

#define ADC_CHAN 2                    //Channel for Analogue input. DC biased to half Supply rail. 

#define OVERSAMPLE 32

#define NUMBEROFBINS 1024                                       // 1024 samples gives a scan rate of the bitrate. 4.375Hz for JT4 and 6 Hz for PI4.
#define NUMBEROFSAMPLES NUMBEROFBINS * OVERSAMPLE              // ADC samples. will be averaged to number of Bins to reduce sampling noise.

//JT4G Detection Values

#define JT4SAMPLERATE 4480 * OVERSAMPLE   //4480 samples per second * oversample. FFT Bandwidth of 0-2240 Hz
#define JT4TONESPACING 72                //tone spacing in number of bins. 315 / 4.375 = 72
#define JT4TONE0 183                      //tone zero 183 * 4.375 = 800.625Hz 
#define JT4TONETOLERANCE 22               //Tone tolerance 22 * 4.375 = +- 96Hz 
#define JT4CACHESIZE 240                 // 240 tone decode samples is approx 55 Seconds
#define JT4SYMBOLCOUNT 207              //number of symbols in JT4 message
#define JT4BITCOUNT 206                 //number of bits in JT4 Message
#define JT4HZPERBIN 4.375              //Hertz per bin. Used to generate displayed spectrum. 
#define JT4SNBINS 571.00                //number of bins for 2.5Khz noise bandwidth

//PI4 Detection Values

#define PI4SAMPLERATE 6144 * OVERSAMPLE  //6144 samples per second * oversample FFT Bandwidth of 0-3072 Hz
#define PI4TONESPACING 39               //tone spacing in number of bins. 234 / 6 = 39
#define PI4TONE0 114                     //tone zero 114 * 6 = 684 Hz 
#define PI4TONETOLERANCE 12               //Tone tolerance 12 * 6 = +- 72Hz 
#define PI4CACHESIZE 180                // 180 tone decode samples is approx 30 Seconds
#define PI4SYMBOLCOUNT 146               //number of symbols in PI4 message
#define PI4BITCOUNT 146                  //number of bits in PI4 Message
#define PI4HZPERBIN 6                   //Hertz per bin. Used to generate displayed spectrum. 
#define PI4SNBINS 416.00                //number of bins for 2.5Khz noise bandwidth
