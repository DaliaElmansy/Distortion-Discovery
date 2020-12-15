#pragma once

#include<string.h>
#include<stdlib.h>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int UINT;
typedef long long INT64;
typedef unsigned long long UINT64;
typedef int(__cdecl CompType)(const void*, const void*);
typedef CompType* CompTypePtr;
typedef int(__cdecl CompContextType)(void*, const void*, const void*);
typedef CompContextType* CompContextTypePtr;


CompType CompStr, CompStri;
////////////// Lists Search
template <typename _t>
int __cdecl Comp(const void*i1, const void*i2)
   {
     return  *(_t*)i1>*(_t*)i2?1:
            (*(_t*)i2>*(_t*)i1?-1:0);
   }

template <typename _t>
int Find(const _t Item, const _t *List, int n, int &index,
                CompType Compare=Comp<_t>)
   {// 0: found at index
    // 1: should be just after index
    //-1: should be just before index
     int h=n-1, l=0, diff;

     index = l;
     if(!n) return -1;
     do{
         index=(l+h)>>1;
         diff = Compare(&Item, List+index);
         if(!diff) return 0;
         else if(diff>0) l=index+1;
         else            h=index-1;
         if(h<l) return diff;
      } while(1);
   }
template <typename _t>
bool InsertOrdered(const _t Item,_t*List,int &n, _t*&ptr, int Size,
                                              CompType Compare=Comp<_t>)
   {
    // return false = item alreay in List at ptr
    // return true with n<=Size = item inserted in List at ptr
    // return true with n>Size = insufficient Size, item not inserted
     int index, diff;
     _t *q=List+n;

     diff=Find<_t>(Item, List, n, index, Compare);
     ptr = List+index;
     if(!diff) // already exists
        return false;
     if(diff>0)
        ptr++;
     if(Size>n)
       {
         for(;q>ptr; q--)
             *q = *(q-1);
         *ptr = Item;
       }
     n++;
     return true;
   }
template <typename _t>
void Sort(_t* List, int n, CompType Compare=Comp<_t>)
   {
     qsort(List, n, sizeof(_t), Compare);
   }
////////////// Indexed Lists Search
template <typename _t, typename _ti>
int _cdecl CompIndex(void*data, const void*i1, const void*i2)
  {
#ifdef _DEBUG1
int j1=*(int*)i1, j2=*(int*)i2;
REAL *List=(REAL*)data;
int retval=List[j1]>List[j2]?1:(List[j1]<List[j2]?-1:0);
#endif // _DEBUG
    return ((_t*)data)[*(_ti*)i1]>((_t*)data)[*(_ti*)i2]? 1:
          (((_t*)data)[*(_ti*)i1]<((_t*)data)[*(_ti*)i2]?-1:
           ((*(_ti*)i1)>(*(_ti*)i2)?1:
            ((*(_ti*)i1)<(*(_ti*)i2)?-1:0)
           )
          );
  }

template <typename _t, typename _ti>
int _cdecl CompIndexDsc(void*data, const void*i1, const void*i2)
  {
#ifdef _DEBUG1
int j1=*(int*)i1, j2=*(int*)i2;
REAL *List=(REAL*)data;
int retval=List[j1]>List[j2]?1:(List[j1]<List[j2]?-1:0);
#endif // _DEBUG
    return ((_t*)data)[*(_ti*)i1]>((_t*)data)[*(_ti*)i2]?-1:
          (((_t*)data)[*(_ti*)i1]<((_t*)data)[*(_ti*)i2]?1:
           ((*(_ti*)i1)>(*(_ti*)i2)?-1:
            ((*(_ti*)i1)<(*(_ti*)i2)?1:0)
           )
          );
  }

template <typename _t, typename _ti>
int FindIndex(const _t Item, const _ti *Indx, _t* List, int n,
              int &index, CompType Compare=Comp<_t>)
   {// 0: found at index
    // 1: should be just after index
    //-1: should be just before index
     int h=n-1, l=0, diff;

     index = l;
     if(!n) return -1;
     do{
         index=(l+h)>>1;
         diff = Compare(&Item, List+Indx[index]);
         if(!diff) return 0;
         else if(diff>0) l=index+1;
         else            h=index-1;
         if(h<l) return diff;
      } while(1);
   }

template <typename _t, typename _ti>
void SortIndex(_ti *Indx, _t* List, int n, CompContextType Compare=CompIndex<_t, _ti>)
   {
     qsort_s(Indx, n, sizeof(_ti), Compare, List);
   }

template <typename _t, typename _ti>
void SortIndexDsc(_ti *Indx, _t* List, int n, CompContextType Compare=CompIndexDsc<_t,_ti>)
   {
     qsort_s(Indx, n, sizeof(_ti), Compare, List);
   }

template <typename _ti>
int __cdecl CompIndexStr(void *data, const void *i1, const void *i2)
   {
     int i=strcmp(((char**)data)[*(_ti*)i1], ((char**)data)[*(_ti*)i2]);
     return i>0?1:(i<0?-1:
                   ((*(_ti*)i1)>(*(_ti*)i2)?1:
                    ((*(_ti*)i1)<(*(_ti*)i2)?-1:0)
                   )
                  ); 
   }

template <typename _ti>
int __cdecl CompIndexStrDsc(void *data, const void *i1, const void *i2)
   {
     int i=strcmp(((char**)data)[*(_ti*)i1], ((char**)data)[*(_ti*)i2]);
     return i>0?-1:(i<0?1:
                    ((*(_ti*)i1)>(*(_ti*)i2)?-1:
                     ((*(_ti*)i1)<(*(_ti*)i2)?1:0)
                    )
                   ); 
   }

template <typename _ti>
int __cdecl CompIndexStri(void *data, const void *i1, const void *i2)
   {
     int i=_stricmp(((char**)data)[*(_ti*)i1], ((char**)data)[*(_ti*)i2]);
     return i>0?1:(i<0?-1:
                   ((*(_ti*)i1)>(*(_ti*)i2)?1:
                    ((*(_ti*)i1)<(*(_ti*)i2)?-1:0)
                   )
                  );
   }

template <typename _ti>
int __cdecl CompIndexStriDsc(void *data, const void *i1, const void *i2)
   {
     int i=_stricmp(((char**)data)[*(_ti*)i1], ((char**)data)[*(_ti*)i2]);
     return i>0?-1:(i<0?1:
                    ((*(_ti*)i1)>(*(_ti*)i2)?-1:
                     ((*(_ti*)i1)<(*(_ti*)i2)?1:0)
                    )
                   ); 
   }

inline int FindIndexStr(char* Item, const int *Indx,
                        char** List, int n, int &index)
   {// 0: found at index
    // 1: should be just after index
    //-1: should be just before index
     return FindIndex<char*>(Item, Indx, List, n, index, CompStr);
   }

inline int FindIndexStri(char* Item, const int *Indx,
                         char** List, int n, int &index)
   {// 0: found at index
    // 1: should be just after index
    //-1: should be just before index
     return FindIndex<char*>(Item, Indx, List, n, index, CompStri);
   }

template <typename _ti>
void SortIndexStr(_ti *Index, char **List, int n)
   {
     SortIndex<char*>(Index, List, n, CompIndexStr<_ti>);
   }
template <typename _ti>
void SortIndexStrDsc(_ti *Index, char **List, int n)
   {
     SortIndex<char*>(Index, List, n, CompIndexStrDsc);
   }
template <typename _ti>
void SortIndexStri(_ti *Index, char **List, int n)
   {
     SortIndex<char*>(Index, List, n, CompIndexStri<_ti>);
   }
template <typename _ti>
void SortIndexStriDsc(_ti *Index, char **List, int n)
   {
     SortIndex<char*>(Index, List, n, CompIndexStriDsc<_ti>);
   }
///////////////////////////////////////////
inline int __cdecl CompStr(const void *d1, const void *d2)
   {
     int diff=strcmp(*((char**)d1), *((char**)d2));
     return diff>0?1:(diff<0?-1:0);
   }

inline int __cdecl CompStri(const void *d1, const void *d2)
   {
     int diff=_stricmp(*((char**)d1), *((char**)d2));
     return diff>0?1:(diff<0?-1:0);
   }

//inline int __cdecl CompChar(const void*i1, const void*i2)
//   {
//     return *(char*)i1-*(char*)i2;
//   }
//inline int __cdecl CompBYTE(const void*i1, const void*i2)
//   {
//     return  *(BYTE*)i1>*(BYTE*)i2?1:
//            (*(BYTE*)i1<*(BYTE*)i2?-1:0);
//   }
//inline int __cdecl CompShort(const void*i1, const void*i2)
//   {
//     return *(short*)i1-*(short*)i2;
//   }
//inline int __cdecl CompWORD(const void*i1, const void*i2)
//   {
//     return  *(WORD*)i1>*(WORD*)i2?1:
//            (*(WORD*)i1<*(WORD*)i2?-1:0);
//   }
//inline int __cdecl CompInt(const void*i1, const void*i2)
//   {
//     return *(int*)i1-*(int*)i2;
//   }
//inline int __cdecl CompUINT(const void*i1, const void*i2)
//   {
//     return  *(UINT*)i1>*(UINT*)i2? 1:
//            (*(UINT*)i1<*(UINT*)i2?-1:0);
//   }
//inline int __cdecl CompInt64(const void*i1, const void*i2)
//   {
//     return  *(INT64*)i1>*(INT64*)i2? 1:
//            (*(INT64*)i1<*(INT64*)i2?-1:0);
//   }
//inline int __cdecl CompUInt64(const void*i1, const void*i2)
//   {
//     return  *(UINT64*)i1>*(UINT64*)i2?1:
//            (*(UINT64*)i1<*(UINT64*)i2?-1:0);
//   }
//inline int __cdecl CompREAL(const void*i1, const void*i2)
//   {
//     return *(REAL*)i1>*(REAL*)i2?1: (*(REAL*)i1<*(REAL*)i2?-1:0);
//   }
inline void SortStr(char **List, int n)
   {
     Sort<char*>(List, n, CompStr);
   }
inline void SortStri(char **List, int n)
   {
     Sort<char*>(List, n, CompStri);
   }
inline void SortInt(int *List, int n)
   {
     Sort<int>(List, n);
   }
//inline int FindInt(const int item, const int *List, int n, int &index)
//   {
//     return Find<const int>(item, List, n, index, CompInt);
//   }
//inline int FindREAL(const REAL item, const REAL *List, int n, int &index)
//   {
//     return Find<const REAL>(item, List, n, index, CompREAL);
//   }
inline int FindStr(const char* item, const char**List, int n, int &index)
   {
     return Find<const char*>(item, List, n, index, CompStr);
   }
inline int FindStri(const char* item, const char**List, int n, int &index)
   {
     return Find<const char*>(item, List, n, index, CompStri);
   }
//inline bool InsertOrderedInt(int i,int *List, int &n, int *&ptr, int Size)
//   {
//     return InsertOrdered<int>(i, List, n, ptr, Size);
//   }
//inline bool InsertOrderedREAL(REAL item, REAL *List, int &n, REAL *&ptr,
//                              int Size)
//   {
//     return InsertOrdered<REAL>(item, List, n, ptr, Size);
//   }
//inline bool InsertOrderedINT64(INT64 i,INT64 *List, int &n, INT64 *&ptr, int Size)
//   {
//     return InsertOrdered<INT64>(i, List, n, ptr, Size, CompInt64);
//   }
inline bool InsertOrderedStr(char* Str,char **List,int &n,char**&ptr, int Size)
   {
     return InsertOrdered<char*>(Str, List, n, ptr, Size, CompStr);
   }
inline bool InsertOrderedStri(char* Str,char **List,int &n,char**&ptr, int Size)
   {
     return InsertOrdered<char*>(Str, List, n, ptr, Size, CompStri);
   }

////////////////////////
template <typename _t>
int __cdecl CompRef(int *order, const void*i1, const void*i2)
  {// order: 0=ascending, 1: descending
    return order==0||*order==0?( **((_t**)i1)>**((_t**)i2)? 1:
                                (**((_t**)i2)>**((_t**)i1)?-1:0)
                               )
                              :(
                                 **((_t**)i2)>**((_t**)i1)? 1:
                                (**((_t**)i1)>**((_t**)i2)?-1:0);
                               );
  }

template <typename _t>
void SortRef(_t** List, int n, int *order=0,
             CompContextType Compare=CompRef<_t>
            )
   {// order: 0=ascending, 1: descending
     qsort_s(List, n, sizeof(_t**), Compare, order);
   }

/////////////////////////////////////////
// Set Manipulations
/////////////////////////////////////////
template<class _t>
int Union(_t *Set1, int n1, _t *Set2, int n2,
                 _t *SU, CompType Compare=Comp<_t>)
{// Assumption: Set1, Set2 are ascendingly ordered
 // if SU != NULL then SU <= Ste1 Union Set2
 // return value = length of union
  _t *p1=Set1, *q1=p1+n1, *p2=Set2, *q2=p2+n2, *pI=SU, e;
  int Diff;

  for( ;p1<q1 && p2<q2; )
     {
       Diff = Compare(p1, p2);
       if(Diff==0)
         {
           if(SU)
              *pI = *p1;
           pI++;
           for(e=*p1; p1<q1 && *p1==e;p1++);
           for(; p2<q2 && *p2==e; p2++);
         }
       else if(Diff > 0)
         {
           if(SU)
              *pI = *p2;
           pI++;
           p2++;
         }
       else
         {
           if(SU)
              *pI = *p1;
           pI++;
           p1++;
         }
     }
   while(p1<q1)
        {
          if(SU)
             *pI=*p1;
          pI++;
          p1++;
        }
   while(p2<q2)
        {
          if(SU)
             *pI=*p2;
          pI++;
          p2++;
        }
  return (int)(pI-SU);
}

template<class _t>
int Intersection(_t *Set1, int n1, _t *Set2, int n2,
                        void*SI, CompType Compare=Comp<_t>)
{// Assumption: Set1, Set2 are ascendingly ordered
 // if SI != NULL then SI <= Ste1 Intersect Set2
 // return value = length of intersection
  _t *p1=Set1, *q1=p1+n1, *p2=Set2, *q2=p2+n2, *pI=(_t*)SI;
  int Diff;

  for(;p1<q1 && p2<q2; )
     {
       Diff = Compare(p1, p2);
       if(Diff==0)
         {
           if(SI)
              *pI = *p1;
           pI++;
           p1++;
           p2++;
         }
       else if(Diff > 0)
         {
           p2++;
         }
       else p1++;
     }
  return (int)(pI-SI);
}

template <typename _t>
void RemoveDuplicates(_t *p, int &n, bool PreSorted=false)
{
  int i, nn = n;

  if(!PreSorted)
     Sort(p, n);
  for(i=1, n=0; i<nn; i++)
   if(p[i]!=p[n])
   if(++n<i)
      p[n] = p[i];
  n++;
}
/////////////////////////////////////////
