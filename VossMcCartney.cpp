//  Author: Andrew Simper of Vellocet
//          andy@vellocet.com
//
// Purpose: c++ class to generate white, pink and brown noise
//
// Sources:
// This is a c++ implementation of put together from the
// code provided by the following people mainly from
// the music-dsp mailing list:  Allan Herriman, James McCartney,
// Phil Burk and Paul Kellet and the web page by Robin Whittle:
// http://www.firstpr.com.au/dsp/pink-noise/
//
#ifndef __VRand_H_
#define __VRand_H_

#include <time.h>

// Usage:
// ------
// VRande rand;
// rand.seed();
// float w = rand.white(); // returns white noise +- 0.5
// float p = rand.pink();  // returns pink noise  +- 0.5
// float b = rand.brown(); // returns brown noise +- 0.5
class VRand
{
public:
  enum
  {
    NumPinkBins  = 16,
    NumPinkBins1 = NumPinkBins-1
  };

  VRand()
  {
    m_white = 0;
    m_count = 1;
    m_brown = 0.0f;
    m_pink  = 0;
    for (int i=0; i<NumPinkBins; i++)
    {
      m_pinkStore[i] = 0.0f;
    }
  };

  void seed(unsigned long seed=0)
  {
    if (seed == 0) m_seed = time(NULL);
    else           m_seed = seed;
  };

  // returns psuedo random white noise number
  // in the range -scale to scale
  //
  inline float white(float scale=0.5f)
  {
     m_seed   = (m_seed * 196314165) + 907633515;
     m_white  = m_seed >> 9;
     m_white |= 0x40000000;
     return ((*(float*)&m_white)-3.0f)*scale;
  };


#ifdef WIN32
  int inline CTZ(int num)
  {
    _asm
    {
      mov ecx, num
      bsf eax, ecx
      mov num, eax
    }
    return num;
  }
#else // please contribute assembler for mac/sparc/sgi/etc.
  int inline CTZ(int num)
  {
    int i=0;
    while (((num>>i)&1)==0 && i<sizeof(int)) i++;
    return i;
  }
#endif

  // returns pink noise random number in the range -0.5 to 0.5
  //
  inline float pink(void)
  {
    float prevr;
    float r;
    unsigned long k;
    k = CTZ(m_count);
    k = k & NumPinkBins1;

    // get previous value of this octave
    prevr = m_pinkStore[k];

    while (true)
    {
      r = white();

      // store new value
      m_pinkStore[k] = r;

      r -= prevr;

      // update total
      m_pink += r;

      if (m_pink <-4.0f || m_pink > 4.0f) m_pink -= r;
      else break;
    }

    // update counter
    m_count++;

    return (white() + m_pink)*0.125f;
  }

  // returns brown noise random number in the range -0.5 to 0.5
  //
  inline float brown(void)
  {
    while (true)
    {
      float  r = white();
      m_brown += r;
      if (m_brown<-8.0f || m_brown>8.0f) m_brown -= r;
      else break;
    }
    return m_brown*0.0625f;
  }

private:
  unsigned long  m_seed;
  unsigned long  m_count;
  unsigned long  m_white;
  float          m_pink;
  float          m_brown;
  float          m_pinkStore[NumPinkBins];
};




// This is a little helper section for fast table
// lookup pink noise to use in denormalising
// and filter excitation
//
// use:
// long i=0;
// float noise = pinkNoise(i);
//
// this leaves a pink noise sample in "noise"
// and increments i so the next call gives a
// new pink noise sample


#define PinkNoiseBins (32768)
#define PinkNoiseWrap (32767)
extern float VPinkNoise[PinkNoiseBins];
#define pinkNoise(c) VPinkNoise[c&PinkNoiseWrap]; c++;

class VPinkNoiseGlobal
{
public:
  VPinkNoiseGlobal(void)
  {
    VRand rand;
    rand.seed();
    double inaudible = 0.000000059604644775390625; //pow(2.0, -24.0);
    for (int i=0; i<PinkNoiseBins; i++)
    {
      VPinkNoise[i] = (float)(inaudible*rand.pink());
    }
  }
};

#endif __VRand_H_
