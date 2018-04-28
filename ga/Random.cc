/*
 *  Random.cpp
 *  PrimateAgent
 *
 *  Created by Bill Sellers on Mon Jan 31 2005.
 *  Copyright (c) 2005 Bill Sellers. All rights reserved.
 *
 */

#include <stdlib.h>
#include <math.h>

#include "Random.h"

// Numerical Recipes
static void sran1(unsigned int seed=1);
static float ran1();
static void sran2(unsigned int seed=1);
static float ran2();
static void sran3(unsigned int seed=1);
static float ran3();

// libc standards
double drand48(void);
void srand48(long seed);

// M. Matsumoto and T. Nishimura, "Mersenne Twister. A 623-dimensionally equidistributed uniform pseudorandom number generator", ACM Trans. on Modeling and Computer Simulation Vol. 8, No. 1, January pp.3-30 (1998)
void init_genrand(unsigned long seed);
void init_by_array(unsigned long init_key[], int key_length);
unsigned long genrand_int32(void);
long genrand_int31(void);
double genrand_real1(void);
double genrand_real2(void);
double genrand_real3(void);
double genrand_res53(void);

// set the random number generator seed
void RandomSeed(RANDOM_SEED_TYPE randomSeed)
{
  RANDOM_SEED(randomSeed);
}

// random double between limits
// random generators vary a bit but generally the value
// will never equal the limits
double RandomDouble(double lowBound, double highBound)
{
  double span = highBound - lowBound;
  return RANDOM_FLOAT() * span + lowBound;
}

// random int between limits
// the value can definitely equal both boundaries
int RandomInt(int lowBound, int highBound)
{
  double v = RandomDouble(lowBound - 0.5, highBound + 0.5);
  int i = (int)(v + 0.5);

  // should never happen - but rounding errors...
  if (i < lowBound) i = lowBound;
  else if (i > highBound) i = highBound;

  return i;
}

// square root biased random int between limits
// higher numbers more likely
int SqrtBiasedRandomInt(int lowBound, int highBound)
{
  double span = ((double)highBound + 0.5) - ((double)lowBound - 0.5);
  double v =  sqrt(RANDOM_FLOAT()) * span + (double)lowBound - 0.5;
   
  int i = (int)(v + 0.5);

  // should never happen - but rounding errors...
  if (i < lowBound) i = lowBound;
  else if (i > highBound) i = highBound;

  return i;
}

// rank biased random int between limit
// higher numbers more likely
// can't cope with more than a few tens of thousands
// unchecked!
int RankBiasedRandomInt(int lowBound, int highBound)
{
  static int myLowBound = 0;
  static int myHighBound = 0;
  static int *cumulative = 0;
  static int n = 0;
  static int total = 0;
  int i, j;

  // slow initialisation only done once
  if (myLowBound != lowBound || myHighBound != highBound)
  {
    if (cumulative) delete [] cumulative;
    myLowBound = lowBound;
    myHighBound = highBound;
    n = 1 + myHighBound - myLowBound;  
    cumulative = new int[n];

    // produce a cumulative map
    j = 0;
    for (i = myLowBound; i <= myHighBound; i++)
    {
      total += i;
      cumulative[j] = total;
      j++;
    }
  }  

  // get a random int
  j = RandomInt(0, total);

  // and search for it in the list
  int lower = 0;
  int upper = n - 1;
  int pivot = (upper - lower) / 2;
  int delta;

  while (true)
  {
    if (pivot == 0) break;
    if (cumulative[pivot - 1] < j && cumulative[pivot] >= j) break; 
    if (j > cumulative[pivot])
    {
       lower = pivot;
       delta = (upper - lower) / 2;
       if (delta == 0) delta = 1;
       pivot += delta;
    }
    else 
    {
       upper = pivot;
       delta = (upper - lower) / 2;
       if (delta == 0) delta = 1;
       pivot -= delta;
    }
  }

  return pivot + myLowBound; 
}

// random coin flip - returns true a proportion of the time that
// depends on 'chanceOfReturningTrue'
bool CoinFlip(double chanceOfReturningTrue)
{
  if (chanceOfReturningTrue == 0) return false;
  
  if (RANDOM_FLOAT() < chanceOfReturningTrue) return true;
  else return false;
}

// unit gaussian routine from galib
// Return a number from a unit Gaussian distribution.  The mean is 0 and the
// standard deviation is 1.0.
//   First we generate two uniformly random variables inside the complex unit 
// circle.  Then we transform these into Gaussians using the Box-Muller 
// transformation.  This method is described in Numerical Recipes in C 
// ISBN 0-521-43108-5 at http://world.std.com/~nr
//   When we find a number, we also find its twin, so we cache that here so 
// that every other call is a lookup rather than a calculation.  (I think GNU 
// does this in their implementations as well, but I don't remember for 
// certain.)
double RandomUnitGaussian()
{
  static bool cached = false;
  static double cachevalue;
  if (cached == true)
  {
    cached = false;
    return cachevalue;
  }

  double rsquare, factor, var1, var2;
  do
  {
    var1 = 2.0 * RANDOM_FLOAT() - 1.0;
    var2 = 2.0 * RANDOM_FLOAT() - 1.0;
    rsquare = var1 * var1 + var2 * var2;
  }
  while (rsquare >= 1.0 || rsquare == 0.0);

  double val = -2.0 * log(rsquare) / rsquare;
  if (val > 0.0)
    factor = sqrt(val);
  else
    factor = 0.0;  // should not happen, but might due to roundoff

  cachevalue = var1 * factor;
  cached = true;

  return (var2 * factor);
}

// These Random Number generator routines although based on versions
// in Numerical Recipes in C are actually taken from galib2.4.5

// The following random number generators are from Numerical Recipes in C.
// I have split them into a seed function and random number function.

// The ran1 pseudo-random number generator.  This one is OK to use as long as
// you don't call it more than about 10^8 times, so for any long GA runs you'd
// better use something with a longer period.

#define IA 16807L
#define IM 2147483647L
#define AM (1.0/IM)
#define IQ 127773L
#define IR 2836L
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

static long ran1_iy=0;
static long ran1_iv[NTAB];
static long ran1_idum=0;

void
sran1(unsigned int seed) {
  int j;
  long k;

  ran1_idum = (long)seed;
  if (ran1_idum == 0) ran1_idum=1;
  if (ran1_idum < 0) ran1_idum = -ran1_idum;
  for (j=NTAB+7;j>=0;j--) {
    k=(ran1_idum)/IQ;
    ran1_idum=IA*(ran1_idum-k*IQ)-IR*k;
    if (ran1_idum < 0) ran1_idum += IM;
    if (j < NTAB) ran1_iv[j] = ran1_idum;
  }
  ran1_iy=ran1_iv[0];
}

float ran1() {
  int j;
  long k;
  float temp;

  k=(ran1_idum)/IQ;
  ran1_idum=IA*(ran1_idum-k*IQ)-IR*k;
  if (ran1_idum < 0) ran1_idum += IM;
  j=ran1_iy/NDIV;
  ran1_iy=ran1_iv[j];
  ran1_iv[j] = ran1_idum;
  if ((temp=AM*ran1_iy) > RNMX) return RNMX;
  else return temp;
}

#undef IA
#undef IM
#undef AM
#undef IQ
#undef IR
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX




// The ran2 pseudo-random number generator.  It has a period of 2 * 10^18 and
// returns a uniform random deviate on the interval (0.0, 1.0) excluding the
// end values.  idum initializes the sequence, so we create a separate seeding
// function to set the seed.  If you reset the seed then you re-initialize the
// sequence.


#define IM1 2147483563L
#define IM2 2147483399L
#define AM (1.0/IM1)
#define IMM1 (IM1-1)
#define IA1 40014L
#define IA2 40692L
#define IQ1 53668L
#define IQ2 52774L
#define IR1 12211L
#define IR2 3791
#define NTAB 32
#define NDIV (1+IMM1/NTAB)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

static long ran2_idum2=123456789;
static long ran2_iy=0;
static long ran2_iv[NTAB];
static long ran2_idum=0;

void 
sran2(unsigned int seed) {
  int j;
  long k;

  ran2_idum = (long)seed;
  if (ran2_idum == 0) ran2_idum=1;
  if (ran2_idum < 0) ran2_idum = -ran2_idum;
  ran2_idum2=(ran2_idum);
  for (j=NTAB+7;j>=0;j--) {
    k=(ran2_idum)/IQ1;
    ran2_idum=IA1*(ran2_idum-k*IQ1)-k*IR1;
    if (ran2_idum < 0) ran2_idum += IM1;
    if (j < NTAB) ran2_iv[j] = ran2_idum;
  }
  ran2_iy=ran2_iv[0];
}

float
ran2() {
  int j;
  long k;
  float temp;

  k=(ran2_idum)/IQ1;
  ran2_idum=IA1*(ran2_idum-k*IQ1)-k*IR1;
  if (ran2_idum < 0) ran2_idum += IM1;
  k=ran2_idum2/IQ2;
  ran2_idum2=IA2*(ran2_idum2-k*IQ2)-k*IR2;
  if (ran2_idum2 < 0) ran2_idum2 += IM2;
  j=ran2_iy/NDIV;
  ran2_iy=ran2_iv[j]-ran2_idum2;
  ran2_iv[j] = ran2_idum;
  if (ran2_iy < 1) ran2_iy += IMM1;
  if ((temp=AM*ran2_iy) > RNMX) return RNMX;
  else return temp;
}

#undef IM1
#undef IM2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX


// The ran3 pseudo-random number generator.  It is *not* linear congruential.

#define MBIG 1000000000
#define MSEED 161803398
#define MZ 0
#define FAC (1.0/MBIG)

static int ran3_inext,ran3_inextp;
static long ran3_ma[56];

void 
sran3(unsigned int seed) {
  long idum = (long)seed;
  long mj,mk;
  int i,ii,k;

  mj=labs(MSEED-labs(idum));
  mj %= MBIG;
  ran3_ma[55]=mj;
  mk=1;
  for (i=1;i<=54;i++) {
    ii=(21*i) % 55;
    ran3_ma[ii]=mk;
    mk=mj-mk;
    if (mk < MZ) mk += MBIG;
    mj=ran3_ma[ii];
  }
  for (k=1;k<=4;k++)
    for (i=1;i<=55;i++) {
      ran3_ma[i] -= ran3_ma[1+(i+30) % 55];
      if (ran3_ma[i] < MZ) ran3_ma[i] += MBIG;
    }
  ran3_inext=0;
  ran3_inextp=31;
}

float 
ran3() {
  long mj;
  
  if (++ran3_inext == 56) ran3_inext=1;
  if (++ran3_inextp == 56) ran3_inextp=1;
  mj=ran3_ma[ran3_inext]-ran3_ma[ran3_inextp];
  if (mj < MZ) mj += MBIG;
  ran3_ma[ran3_inext]=mj;
  return mj*FAC;
}

#undef MBIG
#undef MSEED
#undef MZ
#undef FAC

/*
A C-program for MT19937, with initialization improved 2002/1/26.
 Coded by Takuji Nishimura and Makoto Matsumoto.

 Before using, initialize the state by using init_genrand(seed)
 or init_by_array(init_key, key_length).

 Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. The names of its contributors may not be used to endorse or promote
 products derived from this software without specific prior written
 permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


 Any feedback is very welcome.
 http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
 */

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializes mt[N] with a seed */
void init_genrand(unsigned long s)
{
    mt[0]= s & 0xffffffffUL;
    for (mti=1; mti<N; mti++) {
        mt[mti] =
        (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
        mt[mti] &= 0xffffffffUL;
        /* for >32 bit machines */
    }
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
void init_by_array(unsigned long init_key[], int key_length)
{
    int i, j, k;
    init_genrand(19650218UL);
    i=1; j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
        + init_key[j] + j; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++; j++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=N-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
        - i; /* non linear */
        mt[i] &= 0xffffffffUL; /* for WORDSIZE > 32 machines */
        i++;
        if (i>=N) { mt[0] = mt[N-1]; i=1; }
    }

    mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0,0xffffffff]-interval */
unsigned long genrand_int32(void)
{
    unsigned long y;
    static unsigned long mag01[2]={0x0UL, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(5489UL); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

        mti = 0;
    }

y = mt[mti++];

/* Tempering */
y ^= (y >> 11);
y ^= (y << 7) & 0x9d2c5680UL;
y ^= (y << 15) & 0xefc60000UL;
y ^= (y >> 18);

return y;
}

/* generates a random number on [0,0x7fffffff]-interval */
long genrand_int31(void)
{
    return (long)(genrand_int32()>>1);
}

/* generates a random number on [0,1]-real-interval */
double genrand_real1(void)
{
    return genrand_int32()*(1.0/4294967295.0);
    /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
double genrand_real2(void)
{
    return genrand_int32()*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on (0,1)-real-interval */
double genrand_real3(void)
{
    return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0);
    /* divided by 2^32 */
}

/* generates a random number on [0,1) with 53-bit resolution*/
double genrand_res53(void)
{
    unsigned long a=genrand_int32()>>5, b=genrand_int32()>>6;
    return(a*67108864.0+b)*(1.0/9007199254740992.0);
}
/* These real versions are due to Isaku Wada, 2002/01/09 added */

