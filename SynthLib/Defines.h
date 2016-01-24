#pragma once

namespace mvSynth {

// define basic data types
typedef unsigned __int64 uint64;
typedef __int64 int64;
typedef unsigned int uint32;
typedef int int32;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned char uchar;

#define UNUSED(x) (void)(x)


#define MW_MAX_VOICES 16
#define SYNTH_GLOBAL_VOICE_ID (-1)

#define SampleType  double
#define Sin         sin
#define Cos         cos
#define Exp         exp
#define Pow         pow
#define Floor       floor
#define Abs         fabs
#define M_PI        3.14159265359

} // namespace mvSynth