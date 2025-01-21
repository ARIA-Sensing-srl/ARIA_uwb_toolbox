#include <octave/oct.h>


static inline uint32_t as_uint(const float x){
  return *((uint32_t*)&x);
}


DEFUN_DLD (f16tosingle, args, , "convert F16 to float32, float16 value passed encoded uint16")
{

  int nargin = args.length ();
  if (nargin != 1){
    print_usage();
  }
  uint16NDArray src = args(0).array_value();

  Matrix dst(1, src.numel ());

  for(int i = 0 ; i< src.numel (); i++){
    union {
      float f;
      uint32_t u32;
    }converted;
    uint32_t e,m,v;
    uint32_t curVal = (uint32_t)src.elem(i);
    e = (curVal & 0x7C00)>>10;
    m = (curVal & 0x03FF)<<13;
    v = as_uint((float)m)>>23;
    if (e == 0x1F){
      if (m == 0){
        converted.u32 = (0xff)<<23 | ((curVal & 0x8000) << 16);
      }else{
        converted.u32 = -1;
      }
      dst.elem(i) = converted.f;
    }else{
      converted.u32 = (curVal & 0x8000)<<16 | (e != 0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000));
      dst.elem(i) = converted.f;
    }
  }
  return octave_value (dst);
}






