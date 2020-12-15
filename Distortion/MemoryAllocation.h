#pragma once
#include<Windows.h>
#define DeleteVector(V) {if(V!=0) delete[] V;}

//inline void ZeroMem(void*A, int n)
//  {
//    ULONG64 *p8=(ULONG64*)A, *q8;
//    BYTE *p1=(BYTE*)A, *q1=p1+n;
//
//    if(n>=8)
//       for(q8=p8+(n>>3); p8<q8; )
//           *p8++=(ULONG64)0;
//    for(p1=(BYTE*)p8; p1<q1; )
//        *p1++=(BYTE)0;
//  }

inline
void MemSet(void *base, BYTE val, int n)
  {
    for(BYTE *p=(BYTE*)base,*q=p+n; p<q; )
        *p++=val;
    //ULONG64 *p8=(ULONG64*)base, *q8;
    //BYTE *p1=(BYTE*)base, *q1=p1+n;
    //
    //if(n>=8)
    //  {
    //    *p1=p1[1]=val;
    //    *((WORD*)(p1+2))=*((WORD*)(p1));
    //    *((DWORD*)(p1+4))=*((DWORD*)(p1));
    //    for(p8++, q8=p8+(n>>3); p8<q8; )
    //       *p8++=*(ULONG64*)base;
    //  }
    //for(p1=(BYTE*)p8; p1<q1; )
    //    *p1++=val;
  }

template <class _t>
inline
void SequenceFill(_t *base, int n, _t start=(_t)0, _t step=(_t)1)
{
  _t *q=base+n, i=start;
  for(;base<q; i+=step)
      *base++=i;
}

template<class _t>
inline
void IndexFill(_t *base, int n)
{// _t should be an integral type (supports ++ operator)
  _t i=0, *q=base+n;
  for(;base<q;)
      *base++=i++;
}

template <class _t>
inline
_t* NewVector(int n)
  {
    return new _t[n];
  }

template <class _t>
inline
_t* NewVectorZero(int n)
  {
    _t* p=new _t[n];
    if(p) //ZeroMem(p, n*sizeof(_t));
    for(int i=0;i<n;i++)
        p[i]=(_t)0;
    return p;
  }

template <class _t>
inline
_t** NewTriangularMatrix(int m)
  {
    _t **p=new _t*[m];

    for(int i=0; i<m; i++)
        p[i]=new _t[i];
    return p;
  }

template <class _t>
inline
_t** NewMatrix(int m, int n)
  {
    _t **p=new _t*[m];

    do p[--m]=new _t[n]; while(m>0);
    return p;
  }

template <class _t>
inline
_t** NewMatrixZero(int m, int n)
  {
    _t **p=new _t*[m];

    if(p)
    do{
        p[--m]=new _t[n];
        //ZeroMem(p[m], n*sizeof(_t));
        for(int i=0;i<n;i++)
            p[m][i]=(_t)0;
      } while(m>0);
    return p;
  }

template <class _t>
static
_t*** New3DMatrix(int m, int n, int o)
  {
    int i, j;
    _t ***p=NewMatrix<_t*>(m, n);

    if(!p)
       return NULL;
    for(i=0; i<m; i++)
    for(j=0; j<n; j++)
     if(NULL==(p[i][j]=new _t[o]))
        return NULL;
    return p;
  }

inline void DeleteMatrix(void *M, int m)
  {
    if(M==0)
       return;
    do delete[] ((void**)M)[--m]; while(m>0);
    delete[] M;
  }

inline int NumberOfProcessors()
  {
    SYSTEM_INFO sysinfo;
    
    GetSystemInfo(&sysinfo);
    return sysinfo.dwNumberOfProcessors;
  }

