#pragma once

template <class _t>
void Intersection(_t *S1, int nS1,
                  _t *S2, int nS2,
                  _t *Result, int &nResult)
   { // Result = S1 ꓵ S2
     // S1, S2 should be sorted ascendingly
     // Result is guaranteed to be sorted ascendingly
     int *p1=S1, *q1=p1+nS1,
         *p2=S2, *q2=p2+nS2, *pR=Result;

     while(p1<q1 && p2<q2)
      if(*p1 == *p2)
        {
          *pR++ = *p2++;
          p1++;
        }
      else if(*p1 > *p2)
              p2++;
      else p1++;
      nResult = pR - Result;
   }

template <class _t, class _ti>
void IntersectionIndex(_ti *S1, int nS1,
                       _ti *S2, int nS2,
                       _ti *Result, int &nResult,
                       _t *List
                      )
   { // Result = S1 ꓵ S2
     // S1, S2 should be sorted ascendingly
     // Result is guaranteed to be sorted ascendingly
     _ti *p1=S1, *q1=p1+nS1,
         *p2=S2, *q2=p2+nS2, *pR=Result;

     while(p1<q1 && p2<q2)
      if(*p1 == *p2)
        {
          *pR++ = *p2++;
          p1++;
        }
      else if(List[*p1] > List[*p2])
              p2++;
      else if(List[*p1] < List[*p2])
              p1++;
      else if(*p1 > *p2)
              p2++;
      else p1++;
      nResult = pR - Result;
   }
