#pragma once
#include<random>
#include<stdlib.h>
#include<time.h>
#include"MemoryAllocation.h"
#include"ChiSquare.h"

#define CountOf(X) (sizeof(X)/sizeof(*(X)))
#define PI 3.14159265358979323846
#define SQRT_2 1.4142135623730950488

// Normal Distribution
//  p(x|ave,sd) = exp( (x-ave)^2 / (2 * sd^2) ) / (sd * sqrt(2 * PI))

#define REAL_MAX DBL_MAX
typedef double REAL;

class NormalDistribution
{
private:
  bool Valid;
  REAL _SD, _Mean;
  std::random_device generator;
  std::normal_distribution<REAL> distribution;//(REAL, REAL);
  int Seed;
public:
  NormalDistribution(REAL mean=0, REAL sd=1) {Init();}//mean, sd);}
  bool Init(REAL mean=0, REAL sd=1)
    {
      std::normal_distribution<REAL>::param_type par;
      par._Mean = _Mean = mean;
      par._Sigma = _SD = sd;
      distribution.param(par);
      distribution.reset();
      //return true;
      return Valid = _SD>0;
    }
  void sRand(int seed)
    {
      Seed = seed;
    }
  void Randomize()
    {
      Seed = (int)time(NULL);
      //srand((unsigned int)time(NULL));
    }
  REAL SD() {return _SD;}
  REAL Mean() {return _Mean;}
  static REAL Z_Score(REAL X, REAL mean, REAL sd)
    {
      return (X-mean)/sd;
    }
  static REAL PDF(REAL X, REAL mean, REAL sd)
    {//PDF(x)=exp(- (x-mean)^2 /(2*sd*sd)) / sqrt(2*PI*sd*sd)
      REAL SD_2=sd*sd, X_mean = X-mean;
      return exp(- X_mean*X_mean/(2*SD_2)) / sqrt(2*PI*SD_2);
    }
  static REAL CDF(REAL X, REAL mean, REAL sd)
    {
      return (1 + erf((X-mean)/(sd*SQRT_2)))/2.0;
    }
  static void GetNormalValues(int n, REAL *x, REAL mean, REAL sd)
    {// TODO: simulate using icdf
      int i, nh=n>>1;
      REAL cdf, stp = 1.0/n, min=mean-sd*10, p=min;

      for(cdf=i=0; i<nh; i++,cdf+=stp)
         {
           
         }
    }
  static int FindREAL(REAL x, int &index, REAL *b, int n)
    {// Found: returns 1, x=b[index], not found, returns 0, x>b[index]
      int l=0, h=n-1;

      while(h>=l)
           {
             index=l+((h-l)>>1);
             if(x>b[index])
                l=index+1;
             else if(x<b[index])
                     h=index-1;
             else return 1;
           }
      index = l;
      return 0;
    }
  static REAL GoodnessOfFit(REAL *Samples, int nSamples, REAL mean, REAL sd,
                            REAL LFit[3], REAL RFit[3], const REAL* &z, int &nZ)
    {
      const REAL Z[]={-3, -1.5,-1, -.6, -.3, -.1, 0,
                      .1, .3, .6, 1, 1.5, 3, 4};
      REAL FF, X[CountOf(Z)], O[CountOf(Z)], E[CountOf(Z)];
      int i, j, k, nFreq = CountOf(Z);
    
      z = Z;
      nZ = CountOf(Z);
      for(i=0; i<nFreq; i++)
         {
           X[i] = mean+Z[i]*sd;
           E[i] = CDF(X[i], mean, sd) * nSamples;
           O[i] = 0;
         }
      for(i=nFreq-1; i>0; i--)
          E[i] -= E[i-1];
      for(i=0; i<nSamples; i++)
         {
           FindREAL(Samples[i], k, X, nFreq);
           if(k<0) k++;
           else if(k>=CountOf(Z)) k=CountOf(Z)-1;
           O[k]+=1;
         }
      for(i=0; i<3; i++)
         {
           LFit[i] = O[i]/E[i];//1-fabs(O[i]-E[i])/(O[i]+E[i]);
           j = nFreq-3+i;
           RFit[i] = O[j]/E[j];//1-fabs(O[j]-E[j])/(O[j]+E[j]);
         }
      for(i=0, FF=1; i<nFreq; i++)
          FF+=fabs(O[i]-E[i])/max(O[i],E[i])*E[i];
      return 1-FF/nFreq/nSamples;
      //return Chi_Square::GoodnessOfFit_NormalDist(nFreq, O, E);
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
  REAL GetNormalValue()
    {
      return Valid?distribution(generator):REAL_MAX;
    }
  operator REAL() {return GetNormalValue();}
  bool GetNormalValues(int n, REAL *x,
                       bool HasMin=false, REAL Min=-REAL_MAX, 
                       bool HasMax=false, REAL Max=REAL_MAX
                      )
    {
      if(!Valid)
         return false;
      REAL *y = x + n;
      while(x<y)
           {
             *x = distribution(generator);
             if((!HasMin || *x>=Min)&&(!HasMax || *x<=Max))
                x++;
           }
      return true;
    }
  //http://people.sc.fsu.edu/~jburkardt/cpp_src/normal_dataset/normal_dataset.html
  bool GetNormalValues(int nsamples, REAL var, REAL mean, REAL *x,
                       bool HasMin=false, REAL Min=-REAL_MAX, 
                       bool HasMax=false, REAL Max=REAL_MAX
                      )
    {
      //REAL *pvar=&var;
      //return GetNormalValues(1, nsamples, &pvar, &mean, &x);
      Init(mean, sqrt(var));
      return GetNormalValues(nsamples, x, HasMin, Min, HasMax, Max);
    }
  bool GetNormalValues(int nfactors, int nsamples, REAL **varcovar,
                       REAL *means, REAL **x,
                       bool HasMin=false, REAL Min=-REAL_MAX, 
                       bool HasMax=false, REAL Max=REAL_MAX
                      )
    {
      int i, j, k;
      REAL **b, **Y;

      b = NewMatrix<REAL>(nfactors, nfactors);

      if(!CholeskyFactor(nfactors, varcovar, b))
        { //varcovar is not positive definite symmetric
          DeleteMatrix((void**)b, nfactors);
          return false;
        }
      // Y = MxN matrix of samples of the 1D normal distribution 
      //    with mean 0 and variance 1.
      Y = NewMatrix<REAL>(nfactors, nsamples);
      //Normal1(-1, NULL);// reset
      for(i=0; i<nfactors; i++)
         {
           Normal1(-1, NULL);//reset
           Normal1(nsamples, Y[i]);
         }
      for(j=0; j<nsamples; j++) // Compute X = MU + R' * Y.
         {
           for(i=0; i<nfactors; i++)
              {
                x[i][j] = means[i];
                for(k=0; k<nfactors; k++)
                    x[i][j] += b[k][i]*Y[k][j];
              }
         }
      DeleteMatrix((void**)b, nfactors);
      DeleteMatrix((void**)Y, nfactors);
      return true;
    }
private:
  bool CholeskyFactor(int n, REAL **varcovar, REAL **b)
    {
      int i, j, k;
      REAL s;
    
      for(i=0; i<n; i++)
         {
           for(j=0; j<n; j++ )
              {
                b[i][j] = varcovar[i][j];
              }
         }
      for(j=0; j<n; j++)
         {
           for(k=0; k<j; k++)
              {
                for(i=0; i<k; i++)
                   {
                     //b[k][j] -= b[i][k] * b[i][j];
                     b[j][k] -= b[k][i] * b[j][i];
                   }
                //b[k][j] /= b[k][k];
                b[j][k] /= b[k][k];
              }
           s=b[j][j];
           for(i=0; i<j; i++)
              {
                //s -= b[i][j]*b[i][j];
                s -= b[j][i]*b[j][i];
              }
           if(s<=0.0)
              return false;
           b[j][j] = sqrt(s);
         }
      //  Since the Cholesky factor is in R8GE format, zero out
      //      the upper triangle.
      for(i=0; i<n; i++)
         {
           for(j=i+1; j<n; j++)
               b[i][j] = 0.0;
         }
      return true;
    }
  bool Normal1(int nsamples, REAL *x)
    {
      int i;
      static int made=0, saved=0;
      static REAL y = 0.0;
      REAL a, b, c, d;

      if(nsamples<0)
        {
          y = 0.0;
          made = saved = 0;
          return true;
        }
      if(nsamples==0)
         return false;
    if(saved==1)
      {
        *x++ = y;
        --nsamples;
        saved = 0;
      }
    saved = nsamples&1;
    nsamples&=0xfffffffe;
    RandVector(nsamples, x);
    made+=nsamples;
    for(i=0; i<nsamples; i++, i++)
       {
         a = (REAL)sqrt(-2.0*log(x[i]));
         d = 2.0*PI*x[i+1];
         b = (REAL)cos(d);
         c = (REAL)sin(d);
         //x0 = (REAL)sqrt(-2.0*log(x[i])) * cos(2.0*PI*x[i+1]);
         //x1 = (REAL)sqrt(-2.0*log(x[i])) * sin(2.0*PI*x[i+1]));
         x[i]   = a*b;
         x[i+1] = a*c;
       }
    if(saved)
      {
        REAL xx[2];
        RandVector(2, xx);
        ++made; ++made;
         a = (REAL)sqrt(-2.0*log(xx[0]));
         d = 2.0*PI*xx[1];
         b = (REAL)cos(d);
         c = (REAL)sin(d);
        x[nsamples]=a*b;
        y=a*c;
      }
      return true;
    }
public:
  inline REAL Rand()
    {
      int k;

      k = Seed / 127773;
      Seed = 16807 * (Seed - k*127773) - k*2836;
      if(Seed<0)
         Seed += 0x7FFFFFFF;
      return (REAL)(Seed * 4.656612875E-10);
    }
  REAL RandRange(REAL from, REAL to)
    {
      return from + Rand() * (to-from);
    }
  void RandVector(int n, REAL* r)
    {
      for(int i=0; i<n; i++)
          r[i] = Rand();
    }
  void RandVectorRange(int n, REAL* r, REAL from, REAL to)
    {
      for(int i=0; i<n; i++)
          r[i] = RandRange(from, to);
    }
};
#undef PI