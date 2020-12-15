#pragma once

#include"MemoryAllocation.h"
#include<math.h>
#define PI 3.14159265358979323846
#define SQRT_2 1.4142135623730950488
#define REAL_MAX DBL_MAX

typedef double REAL;
class Chi_Square
{
  public:
  static REAL GoodnessOfFit_NormalDist(int N, REAL *O, REAL *E)
  {// [0, 1] val to express how much observed frequncies fit with NormalDist
   // deg of freedom = N - 3 (3 are 2 vars (Mean, SD) plus 1)
    return 1-ChiSquareTest(N-3, ChiSquare(N, O, E));
  }
  static REAL ChiSquare(int N, REAL *O, REAL *E)
  { // Chi square of N observed & N expected frequencies
    REAL ChiSq, OmE, *o=O, *q=o+N, *e=E;
  
    for(ChiSq=0; o<q; o++,e++)
       {
         OmE = *o-*e;
         ChiSq += OmE * OmE / *e;
       }
    return ChiSq;
  }
  static REAL ChiSquareTest(int K, REAL ChiSq)
  { // p-value of ChiSquare at K deg of freedom
    REAL Table[2][5] = {{31.264,24.725,21.920,19.675,17.275},
                        {0.999, 0.99,  0.975, 0.95,  0.9}};
    for(int i=0; i<5; i++)
     if(ChiSq > Table[0][i])
        return Table[1][i];
    return 0.8;
    //return ChiSquareCDF(K, ChiSq);
  }
  static REAL ChiSquarePDF(int K, REAL X) // Chi square of x, k=deg of freedom
  {
    REAL Two_to_k_by_2, X_to_k_by_2;
  
    if(K<=0)
       return 0;
  
    if(K&1)
      {
        Two_to_k_by_2 = (1<<K) * SQRT_2;
        X_to_k_by_2 = pow(X, K/2.0 - 1);
      }
    else
      {
        Two_to_k_by_2 = 1<<(K>>1);
        X_to_k_by_2 = pow(X, (K>>1)-1);
      }
    return (X_to_k_by_2 * exp(-X/2)) / (Two_to_k_by_2 * Gamma_N_By_2(K));
  }
  static REAL ChiSquareCDF(int K, REAL X) // cummulative Chi_square from 0 to x, k=deg of freedom
  {
    return LowerGamma_N_By_2(K, X)/Gamma_N_By_2(K);
  }
  static REAL erfc(REAL x) // complement of error function
    {
      return 1-erf(x);
    }
  static REAL erf(REAL x) // error function
    {
      int n, sign;
      REAL S, Sold, x2, xx, nn, d, epsilon = 0.0000000000001;
      const REAL Sqrt_PI = sqrt(PI);

      S = x;
      n = 0;
      sign = 1;
      x2 = x*x;
      xx = x;
      nn = 1;
      do{
          Sold = S;
          sign = -sign;
          n++;
          nn *= n;
          xx *= x2;
          S+= (sign * xx) / (nn*(n+n+1));
          d=fabs(S-Sold);
        } while(d>epsilon);
      return (2.0/Sqrt_PI)*S;
    }
  static REAL Gamma_N_By_2(int N) // = Gamma(N/2) where N is +ve integer
  {
    const REAL Sqrt_PI = sqrt(PI);
    int k;
    REAL Gamma;
  
    if(N<=0)
       return REAL_MAX;
    if(N&1)
      {
        REAL x;
        N>>=1;
        Gamma = Sqrt_PI;
        for(k=0, x=0.5; k<N; k++, x+=1)
            Gamma *= x;
      }
    else
      {
        N>>=1;
        Gamma = 1;
        for(k=2; k<N; k++)
            Gamma *= k;
      }
    return Gamma;
  }
  static REAL LowerGamma_N_By_2(int K, REAL X)
  { // = LowerGamma(N/2, X/2) where N is +ve integer
    return Gamma_N_By_2(K)-UpperGamma_N_By_2(K, X);
  }
  static REAL UpperGamma_N_By_2(int K, REAL X)
  { // = UpperGamma(N/2, X/2) where N is +ve integer
  //UpperGamma_N_By_2(k, x)= Gamma_N_By_2(k)
  //                        *( erfc(sqrt(x))
  //                          + ( exp(-x)
  //                             * SUMj=0:k-1.5( pow(x,j+.5)
  //                                            /((j+.5)*Gamma_N_By_2(j+.5))
  //                                           )
  //                            ) 
  //                         ) K=3/2, 5/2, ...
    REAL S, u, d;
    int i,j;
  
    if(K&1)
      {
        d = (K-1)/2;
        S = 0;
        for(j=0; j<d; j++)
           {
             u = j+.5;
             S+=pow(X, u)/(u*Gamma_N_By_2(j+j+1));
           }
        return Gamma_N_By_2(K)*(erfc(sqrt(X))+(exp(-X)*S));
      }
    u = X;
    d = 1;
    S = 1 + X;
    for(i=2; i<K; i++)
       {
         u *= X;
         d *= i;
         S += u/d;
       }
    return Gamma_N_By_2(K<<1)*exp(-X)*S;
  }
};