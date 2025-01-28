/*This file is part of wsprd.
 
 File name: fano.c

 Description: Soft decision Fano sequential decoder for K=32 r=1/2 
 convolutional code.

 Copyright 1994, Phil Karn, KA9Q
 Minor modifications by Joe Taylor, K1JT
*/

#define	LL 1	                // Select Layland-Lushbaugh code
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

unsigned char Partab[] = {
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
 0, 1, 1, 0, 1, 0, 0, 1,
 0, 1, 1, 0, 1, 0, 0, 1,
 1, 0, 0, 1, 0, 1, 1, 0,
};


struct node {
  unsigned long encstate;	// Encoder state of next node
  long gamma;		        // Cumulative metric to this node
  int metrics[4];		// Metrics indexed by all possible tx syms
  int tm[2];		        // Sorted metrics for current hypotheses
  int i;			// Current branch being tested
};

// Convolutional coding polynomials. All are rate 1/2, K=32
#ifdef	NASA_STANDARD
/* "NASA standard" code by Massey & Costello
 * Nonsystematic, quick look-in, dmin=11, dfree=23
 * used on Pioneer 10-12, Helios A,B
 */
#define	POLY1	0xbbef6bb7
#define	POLY2	0xbbef6bb5
#endif

#ifdef	MJ
/* Massey-Johannesson code
 * Nonsystematic, quick look-in, dmin=13, dfree>=23
 * Purported to be more computationally efficient than Massey-Costello
 */
#define	POLY1	0xb840a20f
#define POLY2	0xb840a20d
#endif

#ifdef	LL
/* Layland-Lushbaugh code
 * Nonsystematic, non-quick look-in, dmin=?, dfree=?
 */
#define	POLY1	0xf2d05351
#define	POLY2	0xe4613c47
#endif

/* Convolutional encoder macro. Takes the encoder state, generates
 * a rate 1/2 symbol pair and stores it in 'sym'. The symbol generated from
 * POLY1 goes into the 2-bit of sym, and the symbol generated from POLY2
 * goes into the 1-bit.
 */
#define	ENCODE(sym,encstate) {\
	unsigned long _tmp;\
\
	_tmp = (encstate) & POLY1;\
	_tmp ^= _tmp >> 16;\
	(sym) = Partab[(_tmp ^ (_tmp >> 8)) & 0xff] << 1;\
	_tmp = (encstate) & POLY2;\
	_tmp ^= _tmp >> 16;\
	(sym) |= Partab[(_tmp ^ (_tmp >> 8)) & 0xff];\
}


/* Convolutionally encode a packet. The input data bytes are read
 * high bit first and the encoded packet is written into 'symbols',
 * one symbol per byte. The first symbol is generated from POLY1,
 * the second from POLY2.
 *
 * Storing only one symbol per byte uses more space, but it is faster
 * and easier than trying to pack them more compactly.
 */
int encode(
	   unsigned char *symbols,	// Output buffer, 2*nbytes
	   unsigned char *data,		// Input buffer, nbytes
	   unsigned int nbytes)		// Number of bytes in data
{
  unsigned long encstate;
  int sym;
  int i;

  encstate = 0;
  while(nbytes-- != 0) {
    for(i=7;i>=0;i--) {
      encstate = (encstate << 1) | ((*data >> i) & 1);
      ENCODE(sym,encstate);
      *symbols++ = sym >> 1;
      *symbols++ = sym & 1;
    }
    data++;
  }
  return 0;
}

/* Decode packet with the Fano algorithm.
 * Return 0 on success, -1 on timeout
 */
int fano(
	 unsigned int  *metric,	   // Final path metric (returned value)
	 unsigned long  *cycles,	   // Cycle count (returned value)
	 unsigned int  *maxnp,     // Progress before timeout (returned value)
	 unsigned char *data,	     // Decoded output data
	 unsigned char *symbols,   // Raw deinterleaved input symbols
	 unsigned int nbits,	     // Number of output bits
	 int delta,		             // Threshold adjust parameter
	 unsigned long maxcycles)   // Decoding timeout in cycles per bit
{
  struct node *nodes;		   // First node
  struct node *np;	           // Current node
  struct node *lastnode;	   // Last node
  struct node *tail;		   // First node of tail
  int t;			   // Threshold
  int  m0,m1;
  int ngamma;
  unsigned int lsym;
  unsigned long i;

  int mettab[2][256]={
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   4,
    4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    3,   3,   3,   3,   3,   3,   3,   3,   3,   2,
    2,   2,   2,   2,   1,   1,   1,   1,   0,   0,
    -1,  -1,  -1,  -2,  -2,  -3,  -4,  -4,  -5,  -6,
    -7,  -7,  -8,  -9, -10, -11, -12, -12, -13, -14,
    -15, -16, -17, -17, -18, -19, -20, -21, -22, -22,
    -23, -24, -25, -26, -26, -27, -28, -29, -30, -30,
    -31, -32, -33, -33, -34, -35, -36, -36, -37, -38,
    -38, -39, -40, -41, -41, -42, -43, -43, -44, -45,
    -45, -46, -47, -47, -48, -49, -49, -50, -51, -51,
    -52, -53, -53, -54, -54, -55, -56, -56, -57, -57,
    -58, -59, -59, -60, -60, -61, -62, -62, -62, -63,
    -64, -64, -65, -65, -66, -67, -67, -67, -68, -69,
    -69, -70, -70, -71, -72, -72, -72, -72, -73, -74,
    -75, -75, -75, -77, -76, -76, -78, -78, -80, -81,
    -80, -79, -83, -82, -81, -82, -82, -83, -84, -84,
    -84, -87, -86, -87, -88, -89, -89, -89, -88, -87,
    -86, -87, -84, -84, -84, -83, -82, -82, -81, -82,
    -83, -79, -80, -81, -80, -78, -78, -76, -76, -77,
    -75, -75, -75, -74, -73, -72, -72, -72, -72, -71,
    -70, -70, -69, -69, -68, -67, -67, -67, -66, -65,
    -65, -64, -64, -63, -62, -62, -62, -61, -60, -60,
    -59, -59, -58, -57, -57, -56, -56, -55, -54, -54,
    -53, -53, -52, -51, -51, -50, -49, -49, -48, -47,
    -47, -46, -45, -45, -44, -43, -43, -42, -41, -41,
    -40, -39, -38, -38, -37, -36, -36, -35, -34, -33,
    -33, -32, -31, -30, -30, -29, -28, -27, -26, -26,
    -25, -24, -23, -22, -22, -21, -20, -19, -18, -17,
    -17, -16, -15, -14, -13, -12, -12, -11, -10,  -9,
    -8,  -7,  -7,  -6,  -5,  -4,  -4,  -3,  -2,  -2,
    -1,  -1,  -1,   0,   0,   1,   1,   1,   1,   2,
    2,   2,   2,   2,   3,   3,   3,   3,   3,   3,
    3,   3,   3,   4,   4,   4,   4,   4,   4,   4,
    4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    4,   4,   4,   4,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
    5,   5};

  if((nodes = (struct node *)malloc(nbits*sizeof(struct node))) == NULL) {
    Serial.println("malloc failed\n");
    return 0;
  }
  lastnode = &nodes[nbits-1];
  tail = &nodes[nbits-31];
  *maxnp = 0;

/* Compute all possible branch metrics for each symbol pair
 * This is the only place we actually look at the raw input symbols
 */
  for(np=nodes;np <= lastnode;np++) {
    np->metrics[0] = mettab[0][symbols[0]] + mettab[0][symbols[1]];
    np->metrics[1] = mettab[0][symbols[0]] + mettab[1][symbols[1]];
    np->metrics[2] = mettab[1][symbols[0]] + mettab[0][symbols[1]];
    np->metrics[3] = mettab[1][symbols[0]] + mettab[1][symbols[1]];
    symbols += 2;
  }
  np = nodes;
  np->encstate = 0;

// Compute and sort branch metrics from root node */
  ENCODE(lsym,np->encstate);	// 0-branch (LSB is 0)
  m0 = np->metrics[lsym];

/* Now do the 1-branch. To save another ENCODE call here and
 * inside the loop, we assume that both polynomials are odd,
 * providing complementary pairs of branch symbols.

 * This code should be modified if a systematic code were used.
 */

  m1 = np->metrics[3^lsym];
  if(m0 > m1) 
  {
    np->tm[0] = m0;                             // 0-branch has better metric
    np->tm[1] = m1;
  } 
  else 
  {
    np->tm[0] = m1;                             // 1-branch is better
    np->tm[1] = m0;
    np->encstate++;	                        // Set low bit
  }
  np->i = 0;	                                // Start with best branch
  maxcycles *= nbits;
  np->gamma = t = 0;

  // Start the Fano decoder
  for(i=1;i <= maxcycles;i++) 
  {
    if((int)(np-nodes) > *maxnp) *maxnp=(int)(np-nodes);
#ifdef	debug
    printf("k=%ld, g=%ld, t=%d, m[%d]=%d, maxnp=%d\n",
	   np-nodes,np->gamma,t,np->i,np->tm[np->i],*maxnp);
#endif
// Look forward */
    ngamma = np->gamma + np->tm[np->i];
    if(ngamma >= t) 
    {
      if(np->gamma < t + delta) 
      {               // Node is acceptable
	     /* First time we've visited this node;
	      * Tighten threshold.
	      *
	      * This loop could be replaced with
	       *   t += delta * ((ngamma - t)/delta);
	      * but the multiply and divide are slower.
	      */
	while(ngamma >= t + delta) t += delta;
      }
      np[1].gamma = ngamma;                     // Move forward
      np[1].encstate = np->encstate << 1;
      if(++np == lastnode) 
      {
	      break;	                                // Done!
      }

      /* Compute and sort metrics, starting with the 
       * zero branch
       */
      ENCODE(lsym,np->encstate);
      if(np >= tail) {
	/* The tail must be all zeroes, so don't 
	 * bother computing the 1-branches here.
	 */
	np->tm[0] = np->metrics[lsym];
      } 
      else 
      {
	     m0 = np->metrics[lsym];
	     m1 = np->metrics[3^lsym];
	     if(m0 > m1) 
        {
	        np->tm[0] = m0;                       // 0-branch is better
	        np->tm[1] = m1;
	     } 
       else 
       {
	       np->tm[0] = m1;                       // 1-branch is better
	       np->tm[1] = m0;
	       np->encstate++;	                // Set low bit
	     }
      }
      np->i = 0;	                        // Start with best branch
      continue;
    }
    // Threshold violated, can't go forward
    for(;;) 
    {                                   // Look backward
      if(np == nodes || np[-1].gamma < t) 
      {
	    /* Can't back up either.
	     * Relax threshold and and look
	     * forward again to better branch.
	     */
	     t -= delta;
	    if(np->i != 0) 
       {
	      np->i = 0;
	      np->encstate ^= 1;
	     }
	     break;
      }
      // Back up
      if(--np < tail && np->i != 1) {
	np->i++;                          // Search next best branch
	np->encstate ^= 1;
	break;
      }                                   // else keep looking back
    }
  }

  *metric =  np->gamma;	                  // Return the final path metric  

  // Copy decoded data to user's buffer
  nbits >>= 3;
  np = &nodes[7];
  while(nbits-- != 0) {
    *data++ = np->encstate;
    np += 8;
  }
  *cycles = i+1;
  free(nodes);
  if(i >= maxcycles) return -1;	          // Decoder timed out
  return 0;		                  // Successful completion
}