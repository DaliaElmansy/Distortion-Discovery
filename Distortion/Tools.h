#pragma once

#define Min(x,y) ((x)<(y)?(x):(y))
#define Max(x,y) ((x)>(y)?(x):(y))
typedef double REAL;
#define CountOf(x) (sizeof(x)/sizeof(*(x)))

template <class _t>
inline void Swap(_t &a, _t &b)
  {
    _t tmp = a;
    
    a = b;
    b = tmp;
  }
inline long Round(REAL x)
  {
    return x>0.0?x+0.5:(x<0.0?x-0.5:0.0);
  }
inline unsigned int CountTextFileLines(FILE *f)
  {
    unsigned int n, i;
    char c;

    rewind(f);
    for(n=i=0; EOF!=(c=getc(f));)
     if(c=='\n')
       {
         n++;
         i=0;
       }
     else i++;
    rewind(f);
    return n+(i>0 && c==EOF);
  }
