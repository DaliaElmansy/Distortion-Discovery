#pragma once
#include"NormalDistribution.h"
#include"MemoryAllocation.h"
#include"UPGMA.h"
#include"ChiSquare.h"
#include<process.h>
#include"Tools.h"
#include"Comparizons.h"
#include"Set.h"

//#define TRAIN_ALL
#define MAX_DIMENSIONS 2
#define MAX_TITLE 64
#define MAX_SOURCE 256
#define MAX_CLASSES 2
#define MAX_CLASSNAMELEN 32
#define MAX_CATEGORYNAMELEN 16
#define MAX_SAMPLENAMELEN 32
#define MAX_SAMPLES 128
#define MAX_MIS 32
#define FREQSIZE 20
#define MAXROCSTEPS 17
#define MAX_SPOTTERS 10000
#define SimulatedTag "##-Simulated"
#define ModelFileName "Model.txt"
#define PredictorsDistFName "PredictorsDist.bin"
#define PredictorsNoDistFName "PredictorsNoDist.bin"
#define MAX_DISTORTED 4
#define MAX_ORED_BLOCKS 32
#define MAX_ANDED_BLOCKS 16
#define MAXFREQ FREQSIZE
#define HEATMAPHEIGHT 60
#define HEATMAPWIDTH MAXFREQ
#define LOWLONG(x) (*((int*)(&(x))))
#define HIGHLONG(x) (((int*)(&(x)))[1])
#define XLONG(x, i) (((int*)(&(x)))[i])
#define ALLLONG(x) (LOWLONG(x)+HIGHLONG(x))

class Dataset
{
  private:
  typedef unsigned int(WINAPI *ThreadAPI)(void*);
  struct HeatMap{long long hm[HEATMAPHEIGHT][HEATMAPWIDTH];};
  class Distortion
  {
    struct ORedBlock{int Spotter, Category;};
    struct ANDedBlock
           {BYTE Class;
            int nMis, Mis[MAX_MIS];
            int nORed;
            ORedBlock ORedBlocks[MAX_ORED_BLOCKS];
            ANDedBlock *Next;
            ANDedBlock(){Zero();}
            bool Init(int nord, int nmis)
                     {Reset();nORed=nord;nMis=nmis;}
            void Reset(){Zero();}
            void Zero(){Next=0; nMis=nORed=0;}
           ~ANDedBlock(){if(Next!=0) delete Next;
                         Reset();
                        }
           };

    char buf[1024], *p;
    bool ParseID(int &id)
       {
         while(*p>'9'||*p<'0')
               ++p;
         if(1!=sscanf_s(p,"%d",&id))
            return false;
         while(*p>='0'&&*p<='9')
               p++;
         return true;
       }
    bool ParseDistorted()
       {
         int id;
         char *q=p;

         nDistorted=0;
         do{
             if(!ParseID(id))
                return false;
             ++nDistorted;
             if(*p==',')
                p++;
           } while(*p!=';');
         if(nDistorted==0)
            return false;
         p=q;
         nDistorted=0;
         do{
             if(!ParseID(id))
                return false;
             Distorted[nDistorted++]=id-1;
             if(*p==',')
                p++;
           } while(*p!=';');
         p++;
         return true;
       }
    bool ParseMis(ANDedBlock &anded)
       {
         char *q;
         int mis;

         while(*p++!='{')
            if(p[-1]=='\0')
               return false;
         anded.nMis=0;
         //q=p;
         //do{//count nMis
         //    if(!ParseID(mis))
         //       return false;
         //    ++anded.nMis;
         //    while((*p<'0'||*p>'9')&&*p!='}'&&*p!='\0')
         //          p++;
         //    if(*p=='\0')
         //       return false;
         //  } while(*p!='}');
         //anded.nMis=0;
         //p=q;
         do{//Parse Mis
             if(!ParseID(mis))
                return false;
             anded.Mis[anded.nMis++]=mis-1;
             while((*p<'0'||*p>'9')&&*p!='}'&&*p!='\0')
                   p++;
             if(*p=='\0')
                return false;
           } while(*p!='}');
         ++p;
         return true;
       }
    bool ParseANDedBlock(ANDedBlock &anded)
       {
         int classid;

         while(*p++!='[')
            if(p[-1]=='\0')
               return false;
         if(!ParseID(classid))
            return false;
         anded.Class = (BYTE)classid;
         if(!ParseMis(anded))
            return false;
         if(!ParseORedBlocks(anded))
            return false;
         while(*p++!=']')
            if(p[-1]=='\0')
               return false;
         return true;
       }
    bool ParseORedBlocks(ANDedBlock &anded)
       {
         char *q;
         ORedBlock ored;

         while(*p++!='(')
            if(p[-1]=='\0')
               return false;
         anded.nORed=0;
         //q=p;
         //do{// count nORed
         //    if(!ParseORedBlock(ored))
         //       return false;
         //    ++anded.nORed;
         //    while(*p!='['&&*p!=')')
         //       if(*p++=='\0')
         //          return false;
         //  } while(*p++!=')');
         //anded.nORed=0;
         //p=q;
         do{// Parse ORed
             if(!ParseORedBlock(anded.ORedBlocks[anded.nORed++]))
                return false;
             while(*p!='['&&*p!=')')
                   ++p;
           } while(*p++!=')');
         return true;
       }
    bool ParseORedBlock(ORedBlock &ored)
       {
         int cat;
         
         if(!ParseID(ored.Spotter))
            return false;
         --ored.Spotter;
         if(!ParseID(cat))
            return false;
         ored.Category = (BYTE)cat;
         return true;
       }
    public:
    int nDistorted;
    int Distorted[MAX_DISTORTED];
    BYTE nANDed;
    ANDedBlock ANDedBlocks[MAX_ANDED_BLOCKS];
   
    Distortion() {Reset();}
   ~Distortion() {Reset();}
    void Reset() {nDistorted = nANDed = 0;}
    //D(F1, F2; AND([c1,{S1,S2,S3},OR([F3,cat3],[F4,cat4],[F5,cat5])],
    //              [c2,{S4,S5,S6},OR([F6,cat6],[F7,cat7],[F8,cat8])],
    //              [c3,{S7,S8,S9},OR([F9,cat9],[F10,cat10],[F11,cat11])]
    //             ):C
    // )
    bool ReadMulti(FILE *f)// "Dm" has just been read out of f
       {
         char c, co[6], *q=co;
         int blks, bc, // blks:nested block depth, bc:block counter
             indnt; // ANDed block indentation

         nANDed = 0;
         p=buf;
         *p++='D'; *p++='m';
         blks = bc= indnt = 0;
         while((c=getc(f))!=EOF)
              {
                if(c=='('||c=='['||c=='{')
                  {
                    switch(blks)
                          {
                            case 0: case 1: c='('; break;
                            case 2: if(nANDed>0)
                                      {
                                        *p++='\n';
                                 for(int i=0; i<indnt; i++)
                                     *p++ = ' ';
                                       }
                            case 4: c='['; break;
                            case 3: c=bc==0?'{':'('; break;
                          }
                    *q++ = c=='('?')':(c=='['?']':'}');
                    if(++blks>5)
                       return false;
                  }
                else if(c==')'||c==']'||c=='}')
                  {
                    if(blks==3)
                       ++nANDed;
                    c = *--q;
                    bc=--blks==2?0:1;
                  }
                if(c!=' '&&c!='\t'&&c!='\n')
                  {
                    if(blks==1 && c==')')
                      {
                        *p++='\n';
                        for(int i=1; i<indnt; i++)
                            *p++ = ' ';
                      }
                    *p++=(c>='a'&&c<='z')? c-32: c;
                    if(blks==2 && c=='(')
                       indnt = (int)(p-buf);
                    else if(blks==1 && (c==','||c==';'))
                            *p++ = ' ';
                  }
                if(p-buf>=sizeof(buf)) // too long formula
                   return false;
                if(blks==0)
                  {
                    *p='\0';
                    break;
                  }
              }
         return blks==0&&nANDed>0;
       }
    bool ParseMulti()
       {
         int i;

         p=buf;
         for(;*p!='('; p++)
          if(*p=='\0')
             return false;
         if(!ParseDistorted())
            return false;
         while(*p++!='(')
            if(p[-1]=='\0')
               return false;
         for(i=0; i<nANDed; i++)
          if(!ParseANDedBlock(ANDedBlocks[i]))
             return false;
         while(*p++!=')')
            if(p[-1]=='\0')
               return false;
         return true;
       }
    //D(F1, F2; F3, c3, cat3, {S1,S2,S3})
    bool Read(FILE *f)// "D(" has just been read out of f
       {
         char c;
         int blks; // blks:nested block depth, bc:block counter

         blks=1; // the first '(' has been read before calling
         p=buf;
         *p++='D';
         *p++='(';
         while((c=getc(f))!=EOF)
              {
                if(c=='('||c=='['||c=='{')
                  {
                    c = '{';
                    if(++blks>5)
                       return false;
                  }
                if(c==')'||c==']'||c=='}')
                   c=blks--==2?'}':')'; 
                if(c!=' '&&c!='\t'&&c!='\n')
                   *p++=c;
                if(p-buf>=sizeof(buf)) // too long formula
                   return false;
                if(blks==0)
                  {
                    *p='\0';
                    break;
                  }
              }
         return blks==0? Parse(): false;
       }
    bool Parse()
       {
         int i;

         p=buf;
         for(;*p!='('; p++)
          if(*p=='\0')
             return false;
         if(!ParseDistorted())
            return false;
         nANDed=1;
         ANDedBlocks[0].nORed=1;
         if(!ParseID(ANDedBlocks[0].ORedBlocks[0].Spotter))
            return false;
         --ANDedBlocks[0].ORedBlocks[0].Spotter;
         if(!ParseID(i))
            return false;
         ANDedBlocks[0].Class=(BYTE)i;
         if(!ParseID(i))
            return false;
         ANDedBlocks[0].ORedBlocks[0].Category=(BYTE)i;
         return ParseMis(ANDedBlocks[0]);
       }
    bool Write(FILE *f)
       {
         return fprintf_s(f, "\n%s\n", buf)==strlen(buf)+2;
       }
  };
  struct SpotterContext
  {
    int Spotter;
    REAL Percentile; // Percentile = Mean + Percentile*SD
    BYTE Class, Category;
    bool operator>(const SpotterContext &d)
    {return   Spotter>d.Spotter
            ||Spotter==d.Spotter&&Class>d.Class
            ||Spotter==d.Spotter&&Class==d.Class&&Category>d.Category;
    }
  };
  struct MarkerContext
  {
    BYTE nD;//, nClusters, ClusterClasses[4];
    int Marker[MAX_DIMENSIONS];
    REAL MidPoint;
    BYTE Direction;
  };
  struct DistortionInstance : public MarkerContext, SpotterContext
  {
    REAL InfSynCor;
    REAL Precision, Cohesion, Score, InfSyn, HypSyn;
    int nMis;
    WORD Mis[MAX_MIS];
  };
  struct SpotterType
  {
    int Spotter;
    REAL Percentile;// Percentile<0:Low, Percentile>0:High
    bool operator >(SpotterType &md){return Spotter>md.Spotter;}
  };
  struct CounterType
  {
    int Counters[MAXROCSTEPS][4], PRate;
    REAL AUC, PredictionRate,
         Sensitivity[MAXROCSTEPS], Specificity[MAXROCSTEPS];

    void ReWind()
    {
      //ZeroMem(Counters, sizeof(Counters));
      for(int i=0;i<MAXROCSTEPS;i++)
      for(int j=0;j<4;j++)
          Counters[i][j]=0;
      AUC=PredictionRate=PRate=0;
    }
    void Reset()
    {
      ReWind();
      for(int i=0; i<MAXROCSTEPS; i++)
          Sensitivity[i]=Specificity[i]=0;
    }
    void Merge(CounterType &C, int nRocSteps, int Cls=2)
    {
      int Cols[4]={0,1,2,3}, nC=4;

      if(Cls==0)
        {nC=2; Cols[0]=1; Cols[1]=2;}
      else if(Cls==1)
        {nC=2; Cols[0]=0; Cols[1]=3;}
      for(int k=0; k<nRocSteps; k++)
      for(int i=0; i<nC; i++)
          Counters[k][Cols[i]]+=C.Counters[k][Cols[i]];
    }
    //void MergeWeighted(CounterType &C, int nRocSteps)
    //{
    //  for(int k=0; k<nRocSteps; k++)
    //  for(int i=0; i<4; i++)
    //      Counters[k][i] += C.Counters[k][i]*C.PRate;
    //}
    void CalcPerformance(int nROC_Steps)
    {
      int i, j, i1, TP, FP, TN, FN, P, N;
      REAL FPR[MAXROCSTEPS];

      AUC = 0.0;
      if(    Counters[0][0]==0 && Counters[0][3]==0
         && (Counters[0][1]>0 || Counters[0][2]>0)
        )
        {
          for(i=0, j=nROC_Steps;i<nROC_Steps;i++)
             {
               Counters[i][0]=--j;
               Counters[i][3]=i;
             }
        }
      if(Counters[0][1]==0 && Counters[0][2]==0
         && (Counters[0][0]>0 || Counters[0][3]>0)
        )
        {
          for(i=0, j=nROC_Steps;i<nROC_Steps;i++)
             {
               Counters[i][1]=--j;
               Counters[i][2]=i;
             }
        }
      Sensitivity[0]=1.0;
      Specificity[0]=0.0;
      for(i=1; i<nROC_Steps-1; i++)
         {
           TP=Counters[i][0];
           FP=Counters[i][1];
           TN=Counters[i][2];
           FN=Counters[i][3];
           P = TP + FN;
           N = FP + TN;
           i1=i+1;
           Sensitivity[i]/*TPR*/=P?(REAL)TP/P:0.0;//Y-axis
           FPR[i]=N?(REAL)FP/N:0.0;//X-axis
           Specificity[i]=1-FPR[i];
           if(i>0)
           AUC += (Sensitivity[i]+Sensitivity[i-1])/2.0
                 *(Specificity[i]-Specificity[i-1]);
         }
      Sensitivity[i]=0.0;
      Specificity[i]=1.0;
      AUC += (Sensitivity[i]+Sensitivity[i-1])/2.0
            *(Specificity[i]-Specificity[i-1]);
    }
  };
  struct Predictor
  {
    int Marker[MAX_DIMENSIONS];
    BYTE nD, Class, Category, Direction;
    int nSpotters;
    SpotterType *Spotters;
    CounterType Counters[3];//Counters[x], x=0:Before Corr, x=1:AFter Corr, x=2: NotDist
    Predictor() {Spotters=NULL; Reset();}
   ~Predictor() 
    {
      DeleteVector(Spotters);
      Spotters=NULL;
    }
    void Reset(bool deleteSpotters=true)
    {
      Marker[0]=-1;// not-set
      nD=Class=Category=Direction=0;
      Counters[0].Reset();
      Counters[1].Reset();
      Counters[2].Reset();
      nSpotters=0;
      if(deleteSpotters)
        {
          DeleteVector(Spotters);
          Spotters=NULL;
        }
    }
    void ReWind()
    {
      nSpotters=0;
      Counters[0].ReWind();
      Counters[1].ReWind();
      Counters[2].ReWind();
    }
    void CalcPerformance(int nROC_Steps)
    {
      if(/*nSpotters>0&&*/Counters[1].PredictionRate>0)
        {
          Counters[0].CalcPerformance(nROC_Steps);
          Counters[1].CalcPerformance(nROC_Steps);
        }
      if(Counters[2].PredictionRate>0)
         Counters[2].CalcPerformance(nROC_Steps);
    }
    void Average(int n, Dataset *ds)
    {
//if(   Counters[1].PredictionRate>n
//   || Counters[2].PredictionRate>n
//  )
//{
//int ttt=0;
//ttt=n;
//}
      Counters[1].PredictionRate=(REAL)Counters[1].PRate/n;
      Counters[2].PredictionRate=(REAL)Counters[2].PRate/n;
      Counters[0].PRate=Counters[1].PRate;
      Counters[0].PredictionRate=Counters[1].PredictionRate;
      ds->MidPoints[*Marker]/=n;
      ds->Directions[*Marker]=ds->Directions[*Marker]>=(n/2.0);
    }
  };

  static int _cdecl CompPredictor(int *context, const Predictor*p1, const Predictor*p2)
  {// order[0]: 0=ascending, 1: descending
   // order[1]: 0=Dist,      1: NoDist
    if(context&&*context)//order==1
       return context&&context[1]? CompPredictorNoDistDesc(p1, p2)//nodist
                                 : CompPredictorDistDesc(p1, p2);//nodist
    else                 //oreder==0
       return context&&context[1]? CompPredictorNoDist(p1, p2)//nodist
                                 : CompPredictorDist(p1, p2);//nodist
  }
  static int __cdecl CompPredictorRef(void *_context, const void*_p1, const void*_p2)
  {// context[0]: Order 0=ascending, 1: descending
   // context[1]: Dist  0=Dist,      1: NoDist
   Predictor **p1=(Predictor **)_p1, **p2=(Predictor **)_p2;
   int *context=(int*)_context;
    if(context&&*context)//order==1
       return context&&context[1]? CompPredictorNoDistDesc(*p1, *p2)//nodist
                                 : CompPredictorDistDesc(*p1, *p2);//nodist
    else                 //oreder==0
       return context&&context[1]? CompPredictorNoDist(*p1, *p2)//nodist
                                 : CompPredictorDist(*p1, *p2);//nodist
  }
  static int __cdecl CompPredictorDist(const Predictor *p1, const Predictor *p2)
  {
    if(p1->Counters[1].AUC>p2->Counters[1].AUC)
       return 1;
    if(p1->Counters[1].AUC<p2->Counters[1].AUC)
       return -1;
    return 0;
  }
  static int __cdecl CompPredictorNoDist(const Predictor *p1, const Predictor *p2)
  {
    if(p1->Counters[2].AUC>p2->Counters[2].AUC)
       return 1;
    if(p1->Counters[2].AUC<p2->Counters[2].AUC)
       return -1;
    return 0;
  }
  static int __cdecl CompPredictorDistDesc(const Predictor *p1, const Predictor *p2)
  {
    if(p1->Counters[1].AUC<p2->Counters[1].AUC)
       return 1;
    if(p1->Counters[1].AUC>p2->Counters[1].AUC)
       return -1;
    return 0;
  }
  static int __cdecl CompPredictorNoDistDesc(const Predictor *p1, const Predictor *p2)
  {
    if(p1->Counters[2].AUC<p2->Counters[2].AUC)
       return 1;
    if(p1->Counters[2].AUC>p2->Counters[2].AUC)
       return -1;
    return 0;
  }
  struct ThreadParam
  {
    int *Marker, nD;
    REAL DistStep;
    FILE *f;
    Dataset *ds;
    int IFreq[FREQSIZE*3], Index;
    long long DistFreq[FREQSIZE], SynCorrFreq[FREQSIZE*3];
    HeatMap *hm;
    Predictor predictors[2][2];
    void (*Progress)();
    void (Dataset::*CallBack)(ThreadParam*);
    ThreadParam()
    {
      for(int Cls=0; Cls<2; Cls++)
      for(int Cat=0; Cat<2; Cat++)
         {
           if(NULL==(predictors[Cls][Cat].Spotters=
                        NewVector<SpotterType>(MAX_SPOTTERS)
                    )
             )
               Exit(1, "Failed to allocate ThreadParams.predictors.Spotters.");
           predictors[Cls][Cat].Class=Cls;
           predictors[Cls][Cat].Category=Cat;
         }
    }
  };
  typedef UPGMA::Cluster Cluster;
  struct ClusterType_1F
  {
    WORD nSamples[2];
    WORD *ClassSamples[2];
    REAL MidPoint;
    BYTE Direction;
    WORD ClassCounts[2][MAX_CLASSES];
    BYTE Classes[2];
  };
  struct ClusterType_2F
  {
    WORD nSamples[4];
    WORD *ClassSamples[4];
    REAL MidPoint[2];
    BYTE Direction[2];
    WORD ClassCounts[4][MAX_CLASSES];
    BYTE Classes[4];
  };
  struct ClusterType_3F
  {
    WORD nSamples[8];
    WORD *ClassSamples[8];
    REAL MidPoint[3];
    BYTE Direction[3];
    WORD ClassCounts[8][MAX_CLASSES];
    BYTE Classes[8];
  };

  bool GetUnsigned(FILE *f, unsigned &n)
     {
       char c;

       while((c=getc(f))<'0'||c>'9')
          if(c==EOF)
             return false;
       ungetc(c, f);
       return 1==fscanf_s(f, "%u",&n);
     }
  int GetString(FILE *f, char *s, int size, char sep=' ')
     {
       char c, *p, *q=s+size-1;

       while((c=getc(f))==' '||c=='\t'||c=='\n')
          if(c==EOF)
             return false;
       ungetc(c, f);
       for(p=s; c=getc(f); )
          {
            if(c=='\n'||c=='\r'||c=='\t'||c==EOF||c==' '
               ||c==sep
              )
              {
                *p=0;
                break;
              }
            if(p==q)
             return -1;// error: buf size too small
            *p++=c;
          }
       return p-s;
     }
  bool GetInt(FILE *f, int &n)
     {
       char c;

       while((c=getc(f))<'0'||c>'9'&&c!='-')
          if(c==EOF)
             return false;
       ungetc(c, f);
       return 1==fscanf_s(f, "%i",&n);
     }
  bool GetLongLong(FILE *f, long long &n)
     {
       char c;

       while((c=getc(f))<'0'||c>'9'&&c!='-')
          if(c==EOF)
             return false;
       ungetc(c, f);
       return 1==fscanf_s(f, "%lli",&n);
     }
  bool GetREAL(FILE *f, REAL &x)
     {
       char c;

       while((c=getc(f))<'0'||c>'9'&&c!='-'&&c!='.')
          if(c==EOF)
             return false;
       ungetc(c, f);
       return 1==fscanf_s(f, "%lf",&x);
     }
  bool GetEOL(FILE *f)
     {
       char c;

       while((c=getc(f))!='\n')
          if(c==EOF)
             return false;
       return true;
     }
  inline REAL H(int nsamples, WORD *classcounts) // Entropy
     {
       REAL h=0, p;
     
       for(int i=0; i<2; i++)
          {
            p = (REAL)classcounts[i]/nsamples;
            if(p>0.0 && p<1.0)
               h -= p * log(p);
          }
       return h / Log_2;
     }
  inline REAL CondH(ClusterType_1F &Clusters)
     { // Conditional Entropy
       int i, N;
       REAL cndh=0;
     
       for(N=i=0; i<2; i++)
        if(Clusters.nSamples[i] >= MinClusterSize)
          {
            N += Clusters.nSamples[i];
            cndh+=H(Clusters.nSamples[i], Clusters.ClassCounts[i])
                  *Clusters.nSamples[i];
          }
       return cndh/N;
     }
  inline REAL CondH(int nClusters, Cluster *Clusters)
     { // Conditional Entropy
       int i, N;
       REAL cndh=0;
     
       for(N=i=0; i<nClusters; i++)
        if(Clusters[i].nSamples >= MinClusterSize)
          {
            N += Clusters[i].nSamples;
            cndh+=H(Clusters[i].nSamples, Clusters[i].ClassCounts)
                  *Clusters[i].nSamples;
          }
       return cndh/N;
     }
  // Global Variables
  int From, To, nPairs, *Pairs;
  bool Factor_PairList, Syn2Found, //EvaluateCorrectionRates,
       PretrainedTesting;
  HeatMap *HeatMaps;
  int **OffsetsTable; // for distributin data on threads
  int **ThreadsPtr, **ThreadsEndPtr;
  int nSamplesTest, nThreads;
  BYTE *FactorMask, *CandidateSpottersMask;
  NormalDistribution nd;
  REAL Log_2;
  WORD ClassCounts[MAX_CLASSES], MaxCounts;
  HANDLE *ThreadHandles;
  UINT *ThreadIds;
  ThreadParam *ThreadParams;
  CRITICAL_SECTION critSec, critSecAbort, critSecProgress;
  bool AbortFlag;
  int nMask;
  Predictor *Predictors;
  int nPredictors;
  WORD *SampleIndex;
  char FactorNameBuf[512*1024], DatasetFileName[MAX_PATH];
  int *nTails;
  WORD **Tails;

  public:
  char *MI_Fname, *SynBinFname, *SynTxtFname,
       *PermutationsFname,
       *PredictionTestingDistFname,
       *PredictionTestingNoDistFname;
  long long Syn2_Score_HeatMap[HEATMAPHEIGHT][HEATMAPWIDTH];
  int MaxHeatMap;
  REAL *FactorMeans, *FactorSDs, *FactorMin, *FactorMax,
       **FactorClassMeans, **FactorClassSDs,
       **FactorClassMin, **FactorClassMax, **FactorICorr,
       CumulativeFreqActual[3][MAXFREQ],
       CumulativeFreqPermuted[3][MAXFREQ],
       DistFreqAve[3][MAXFREQ],
       pValues[3][MAXFREQ], **ROC_Steps, *MidPoints,
       MinCumFreqPerm[3][MAXFREQ], MaxCumFreqPerm[3][MAXFREQ];
  int *FactorNameIndex, *FactorNameIndexDsc, *Directions,
      *FactorMIIndex, *FactorMIIndexDsc,
      *FactorNoIndex, nParts;
  int IFreq_1F[FREQSIZE], SynFreq_2F[FREQSIZE*3];
  long long DistFreq_1F[FREQSIZE], DistFreq_2F[FREQSIZE],
       ICorrFreq_1F[FREQSIZE], SynCorrFreq_2F[FREQSIZE*3];
  REAL HC, *CndH_1F, /**Limits_1F,*/ **CndH_2F;
  char Source[MAX_SOURCE], Title[MAX_TITLE];
  REAL MinDistScore, MinCohesion, MinPrecision, MinAUC_Gain,
       MinI, MinICorr, MaxMisRatio, MaxMisTotal,
       MinSpottingRate,MinPredictionRate,MinAUC,
       TopAUC_Dist[3],TopAUC_NoDist,TopPredictionRate_Dist[3],
       TopPredictionRate_NoDist;
  int MinMis, MaxMis, MinSpotted, MinClusterSize, nShuffles, nROC_Steps;
  bool Simulated, Testing, Permuting, FrequencyOnly,
       Syn2Only, Randomize, SaveResults;
  int nPredictorsDist, nPredictorsNoDist, nTopPredictorsDist[3], nTopPredictorsNoDist;
  Predictor **pPredictorsDist, **pPredictorsNoDist;
  CounterType CombinedDist[3][2], CombinedNoDist;
  enum _FocusOn{Both, FP, FN};// FocusOn;
  int nFactors, nSamples, nClasses, nCategories;
  BYTE SampleClasses[MAX_SAMPLES], **FactorFreq, *ClustersMembership;
  int FactorFreqSize;
  char  ClassNames[MAX_CLASSES][MAX_CLASSNAMELEN],
        CategoryNames[2][MAX_CATEGORYNAMELEN],
        SampleNames[MAX_SAMPLES][MAX_SAMPLENAMELEN],
        **FactorNames;
  REAL **FactorData;
  ClusterType_1F *FactorClusters;

  Dataset()
     {
       Zero();
       InitializeCriticalSectionAndSpinCount(&critSec, 100);
       //InitializeCriticalSection(&critSec);
       EnterCriticalSection(&critSec);
       LeaveCriticalSection(&critSec);
       InitializeCriticalSectionAndSpinCount(&critSecAbort, 100);
       EnterCriticalSection(&critSecAbort);
       LeaveCriticalSection(&critSecAbort);
       InitializeCriticalSectionAndSpinCount(&critSecProgress, 100);
       EnterCriticalSection(&critSecProgress);
       LeaveCriticalSection(&critSecProgress);
       //EvaluateCorrectionRates=false;
       Simulated=false;
       nROC_Steps = MAXROCSTEPS;
       Syn2Only=AbortFlag=false;
       SynBinFname="Syn2.bin";
       SynTxtFname="Syn2.txt";
       MI_Fname="MI.txt";
       PermutationsFname="Permutations.txt";
       PredictionTestingDistFname="PredictionsDist.txt";
       PredictionTestingNoDistFname="PredictionsNoDist.txt";
       Log_2=log((REAL)2.0);
     }
  //Dataset(int nfactors, int nsamples, int nclasses,
  //        BYTE ncategories, BYTE* sampleclasses,
  //        char** classnames, char** factornames, char** samplenames
  //       )
  //   {
  //     InitializeCriticalSection(&critSec);
  //     EnterCriticalSection(&critSec);
  //     LeaveCriticalSection(&critSec);
  //     InitializeCriticalSection(&critSecAbort);
  //     EnterCriticalSection(&critSecAbort);
  //     LeaveCriticalSection(&critSecAbort);
  //     InitializeCriticalSection(&critSecProgress);
  //     EnterCriticalSection(&critSecProgress);
  //     LeaveCriticalSection(&critSecProgress);
  //     //EvaluateCorrectionRates=false;
  //     Syn2Only=Simulated=Testing=Permuting=false;
  //     Init(nfactors, nsamples, nclasses, ncategories, sampleclasses,
  //          classnames, factornames, samplenames
  //         );
  //     SynBinFname="Syn2.bin";
  //     SynTxtFname="Syn2.txt";
  //     MI_Fname="MI.txt";
  //     PermutationsFname="Permutations.txt";
  //     //EvaluateCorrectionRates=Simulated=false;
  //   }
 ~Dataset()
     {
       DeleteCriticalSection(&critSec);
       DeleteCriticalSection(&critSecAbort);
       DeleteCriticalSection(&critSecProgress);
       Reset();
     }
  void Reset()
     {
       if(Predictors)
          DeAllocateTesting();
       DeleteVector(FactorNames);
       DeleteVector(FactorNameIndex);
       DeleteVector(FactorNameIndexDsc);
       DeleteVector(FactorMIIndex);
       DeleteVector(FactorMIIndexDsc);
       DeleteVector(FactorNoIndex);
       DeleteVector(ClustersMembership);
       DeleteMatrix(FactorFreq, nFactors);
       DeleteMatrix((void**)FactorData, nFactors);
       DeleteVector(FactorMeans);
       DeleteVector(FactorMin);
       DeleteVector(FactorMax);
       DeleteVector(FactorSDs);
       DeleteMatrix(FactorClassMeans, nClasses);
       DeleteMatrix(FactorClassMin, nClasses);
       DeleteMatrix(FactorClassMax, nClasses);
       DeleteMatrix(FactorClassSDs, nClasses);
       DeleteVector(CndH_1F);
       //DeleteVector(Limits_1F);
       DeleteVector(ThreadHandles);
       DeleteVector(ThreadIds);
       DeleteVector(ThreadParams);
       DeleteVector(CandidateSpottersMask);
       DeleteVector(Pairs);
       DeleteMatrix((void**)CndH_2F, nFactors);
       DeleteVector(FactorMask);
       DeleteVector(SampleIndex)
       DeleteVector(FactorClusters);
       DeleteVector(Tails);
       DeleteVector(nTails);
       Zero();
     }
  void Zero()
     {
       nFactors=nSamples=nClasses=From=0;
       To = -1;
       Factor_PairList=Syn2Found=Testing=Permuting=Syn2Only=
           PretrainedTesting=false;
       SaveResults = true;
       nPairs=0;
       Pairs = NULL;
       FactorData=CndH_2F=NULL;
       FactorMeans=FactorSDs=FactorMin=FactorMax=CndH_1F=NULL;
       FactorClassMeans=FactorClassSDs=FactorClassMin=FactorClassMax=NULL;
       ClustersMembership=NULL;
       FactorFreq=NULL;
       FactorNames=NULL;
       FactorNameIndex=FactorNameIndexDsc=
       FactorMIIndex=FactorMIIndexDsc=FactorNoIndex=NULL;
       MinDistScore=0.0;
       ThreadHandles=NULL;
       ThreadIds=NULL;
       ThreadParams=NULL;
       FactorMask=NULL;
       SampleIndex=NULL;
       FactorClusters=NULL;
       CandidateSpottersMask=NULL;
       nMask=0;
       Predictors=NULL;
       pPredictorsDist=pPredictorsNoDist=NULL;
       Directions=NULL;
       MidPoints=NULL;
       ROC_Steps=NULL;
       MinI=0.5;
       MinICorr=0.7;
       MinAUC=0.5;
       MinMis=5;
       MinSpotted=2;
       MinSpottingRate=0.5;
       Tails=NULL;
       nTails=NULL;
       Randomize=false;
       //nShuffles=10;
       MaxMisRatio=0.25;
       MaxMisTotal=1.75;
       MinDistScore=MinPrecision=MinCohesion=MinAUC_Gain=0.0;
       MinPredictionRate=0.75;
       for(int i=0; i<3; i++)
       TopAUC_Dist[0]=TopPredictionRate_Dist[0]=0;
       TopAUC_Dist[1]=TopAUC_Dist[2]=
       TopPredictionRate_Dist[1]=TopPredictionRate_Dist[2]=0.9;
       TopAUC_NoDist=TopPredictionRate_NoDist=0.9;
     }
  void Abort()
     {
       EnterCriticalSection(&critSecAbort);
       AbortFlag=true;
       LeaveCriticalSection(&critSecAbort);
     }
  bool LoadParamFile(char *ParamFileName)
     {
       int i;
       FILE *f;
       REAL x;
       static char *_Keywords[]={"List","MaxMisRatio",
                                 "MaxMisTotal",
                                 "MinAUC",
                                 "MinAUC_Gain",
                                 "MinCohesion",
                                 "MinI","MinICorr",
                                 "MinMis","MinPrecision",
                                 "MinPredictionRate",
                                 "MinScore",
                                 "MinSpotted","MinSpottingRate",
                                 "nShuffles","TopAUC_Dist",
                                 "TopAUC_NoDist",
                                 "TopPredictionRate_Dist",
                                 "TopPredictionRate_NoDist"
                                };

       char buf[256], **Keywords=_Keywords, *p, *q;

       if(0!=fopen_s(&f, ParamFileName, "rt"))
           return false;
       while(fgets(buf, sizeof(buf), f))
            {
              if(*buf=='/' && buf[1]=='/') // comment
                 continue;
              for(p=buf; *p!=' '&&*p!='\t'&&*p!='\n'&&*p!='\r'; p++)
                 {}
              if(*p)
                {
                  for(*p++=0; *p==' '||*p=='\t'; p++)
                     {}
                  if(*p==0||*p=='\n'||*p=='\r')
                     p=0;
                }
              else p=0;
              if(Find((char*)buf, Keywords, CountOf(_Keywords), i, CompStri))
                {
                  char MsgBuf[512];
                  sprintf_s(MsgBuf, "\nUnknown keyword \'%s\' in Config File: %s\n",
                           buf, ParamFileName);
                  Exit(1, MsgBuf);
                }
              if(p)
                {
                  for(q=p; *q!='\n'&&*q!='\r'&&*q!='\t'&&*q; q++)
                     {}
                  *q=0;
                }
              switch(i)
                    {
                      case 0: // List
                              if(p&&*p)
                                 ReadFactorMask(p);
                      break;
                      case 1: // MaxMisRate
                              MaxMisRatio=atof(p);
                      break;
                      case 2: // MaxMisTotal
                              MaxMisTotal=atof(p);
                      break;
                      case 3: // MinAUC
                              MinAUC=atof(p);
                      break;
                      case 4: // MinAUC_Gain
                              MinAUC_Gain=atof(p);
                      break;
                      case 5: // MinCohesion
                              MinCohesion=atof(p);
                      break;
                      case 6: // MinI
                              MinI=atof(p);
                      break;
                      case 7: // MinIcor
                              MinICorr=atof(p);
                      break;
                      case 8: // MinMis
                              MinMis=atoi(p);
                      break;
                      case 9: // MinPrecision
                              MinPrecision=atof(p);
                      break;
                      case 10: // MinPredictionRate
                               MinPredictionRate=atof(p);
                      break;
                      case 11: // MinDistScore
                               MinDistScore=atof(p);
                      break;
                      case 12: // MinSpotted
                               MinSpotted=atoi(p);
                      break;
                      case 13: // MinSpotted
                               MinSpottingRate=atof(p);
                      break;
                      case 14: // nShuffles
                               nShuffles=atoi(p);
                      break;
                      case 15: // TopAUC_Dist
                               TopAUC_Dist[1] = 
                               TopAUC_Dist[2] = atof(p);
                               
                               for(p=++q;*q!=0&&*q!='\t'&&*q!='\n';q++)
                                  {}
                               *q=0;
                               x=atof(p);
                               if(x<=0 || x>1.0)
                                  break;
                               TopAUC_Dist[2] = x;
                      break;
                      case 16: // TopAUC_NoDist
                               TopAUC_NoDist = atof(p);
                      break;
                      case 17: // TopPredictionRate_Dist
                               TopPredictionRate_Dist[1]=
                               TopPredictionRate_Dist[2]=atof(p);
                               for(p=++q;*q!=0&&*q!='\t'&&*q!='\n';q++)
                                  {}
                               *q=0;
                               x=atof(p);
                               if(x<=0 || x>1.0)
                                  break;
                               TopPredictionRate_Dist[2] = x;
                      break;
                      case 18: // TopPredictionRate_NoDist
                               TopPredictionRate_NoDist=atof(p);
                      break;
                    }
            }
       fclose(f);
       return true;
     }
  bool SavePredictionTestingParamFile(char *ParamFileName)
     {
       int i;
       FILE *f;
       char *FocusOnNames[]={"Both","FP","FN"};
       if(0!=fopen_s(&f, ParamFileName, "wt"))
          return false;
       fprintf_s(f, "nShuffles\t%d\n", nShuffles);
       fprintf_s(f, "MinI\t%g\n", MinI);
       fprintf_s(f, "MinICorr\t%g\n", MinICorr);
       fprintf_s(f, "MinMis\t%d\n", MinMis);
       fprintf_s(f, "MaxMisRatio\t%g\n", MaxMisRatio);
       fprintf_s(f, "MaxMisTotal\t%g\n", MaxMisTotal);
       fprintf_s(f, "MinSpotted\t%d\n", MinSpotted);
       fprintf_s(f, "MinSpottingRate\t%g\n", MinSpottingRate);
       fprintf_s(f, "MinScore\t%g\n", MinDistScore);
       fprintf_s(f, "MinPrecision\t%g\n", MinPrecision);
       fprintf_s(f, "MinCohesion\t%g\n", MinCohesion);
       fprintf_s(f, "MinAUC_Gain\t%g\n", MinAUC_Gain);
       fprintf_s(f, "MinPredictionRate\t%g\n", MinPredictionRate);
       fprintf_s(f, "MinAUC\t%g\n", MinAUC);
       fprintf_s(f, "TopPredictionRate_Dist\t");
       for(i=1; i<3; i++)
           fprintf_s(f, "%g%c", TopPredictionRate_Dist[i],
                     i<2?'\t':'\n');
       fprintf_s(f, "TopAUC_Dist\t");
       for(i=1; i<3; i++)
           fprintf_s(f, "%g%c", TopAUC_Dist[i], i<2?'\t':'\n');
       fprintf_s(f, "TopPredictionRate_NoDist\t%g\n", TopPredictionRate_NoDist);
       fprintf_s(f, "TopAUC_NoDist\t%g\n", TopAUC_NoDist);
       fclose(f);
       return true;
     }
  bool SaveDistortionDiscovering_SingleParamFile(char *ParamFileName)
     {
       int i;
       FILE *f;
       if(0!=fopen_s(&f, ParamFileName, "wt"))
          return false;
       fprintf_s(f, "MinMis\t%d\n", MinMis);
       fprintf_s(f, "MaxMisRatio\t%g\n", MaxMisRatio);
       fprintf_s(f, "MaxMisTotal\t%g\n", MaxMisTotal);
       fprintf_s(f, "MinSpotted\t%d\n", MinSpotted);
       fprintf_s(f, "MinSpottingRate\t%g\n", MinSpottingRate);
       fprintf_s(f, "MinScore\t%g\n", MinDistScore);
       fprintf_s(f, "MinPrecision\t%g\n", MinPrecision);
       fprintf_s(f, "MinCohesion\t%g\n", MinCohesion);
       fclose(f);
       return true;
     }
  bool SavePermutationTestingParamFile(char *ParamFileName)
     {
       int i;
       FILE *f;
       if(0!=fopen_s(&f, ParamFileName, "wt"))
          return false;
       fprintf_s(f, "nShuffles\t%d\n", nShuffles);
       fprintf_s(f, "MinMis\t%d\n", MinMis);
       fprintf_s(f, "MaxMisRatio\t%g\n", MaxMisRatio);
       fprintf_s(f, "MaxMisTotal\t%g\n", MaxMisTotal);
       fprintf_s(f, "MinSpotted\t%d\n", MinSpotted);
       fprintf_s(f, "MinSpottingRate\t%g\n", MinSpottingRate);
       fclose(f);
       return true;
     }
  bool Init(bool factor_PairList = false, int from=0, int to=-1)
     {
       int i, j, k, l;
       char *p;

       FactorFreqSize=11;
       Factor_PairList = factor_PairList;
       From = from;
       To = to;
       nCategories=2;
       //nShuffles=100;
       MinMis = int(0.5 + nSamples*0.05);
       MaxMisRatio = 0.25;
       MaxMis = int(0.5 + nSamples*MaxMisRatio);
       MaxMisTotal = 1.5;
       MinClusterSize = nSamples*0.3;
       //if(MinClusterSize>4)
       //   MinClusterSize=4;
       if(MinClusterSize<2)
          MinClusterSize=2;
       strcpy_s(CategoryNames[0], "Low");
       strcpy_s(CategoryNames[1], "High");
       ClustersMembership=NewVector<BYTE>(nFactors*nSamples);
       //ZeroMem(ClustersMembership, nFactors*nSamples);
       for(i=0;i<nFactors*nSamples;i++)
           ClustersMembership[i]=0;
       FactorFreq=NewMatrix<BYTE>(nFactors, FactorFreqSize*(nClasses+1));
       if(FactorNames[0]==NULL)
         {
           for(i=0, p=FactorNameBuf; i<nFactors; i++)
              {
                sprintf_s(p,16,"F%-6i",i+1);
                FactorNames[i]=p;
                p+=strlen(p)+1;
              }
         }
       //if(classnames==NULL)
       //  {
       //    for(l=1,k=nClasses,j=1; k>0; k-=j,l++)
       //        j*=26;
       //    ClassNames = NewMatrix<char>(nClasses, j);
       //  }
       //else ClassNames = NewVector<char*>(nClasses);
       //for(i=0; i<nClasses; i++)
       //   {
       //     char *p, *q;
       //
       //     if(classnames==NULL)
       //       {
       //         for(l=1,k=i+1,j=1; k>0; k-=j,l++)
       //            {
       //              j*=26;
       //            }
       //         ClassNames[i][l-1] = 0;
       //         for(j=l-2,k=i; j>=0; j--)
       //            {
       //              ClassNames[i][j] = 65+k%26;
       //              k /= 26;
       //              k--;
       //            }
       //       }
       //     else
       //       {
       //         p=classnames[i];
       //         for(q=p,k=1; *q; q++,k++)
       //            {                 
       //            }
       //         ClassNames[i] = NewVector<char>(k);
       //         q=ClassNames[i];
       //         do{
       //             *q++=*p;
       //           } while(*p++);
       //       }
       //   }
       if(SampleNames[0][0]==0)
         {
           int *cc=NewVector<int>(nClasses);
           for(i=0; i<nClasses; i++)
               cc[i]=0;
           char* frmt=MaxCounts<10?"%s%01d":(MaxCounts<100?"%s%02d":"%s%03d");
           for(i=0; i<nSamples; i++)
               sprintf_s(SampleNames[i],16,frmt,
                         ClassNames[SampleClasses[i]],
                         ++cc[SampleClasses[i]]
                        );
           DeleteVector(cc);
         }
       FactorData = NewMatrix<REAL>(nFactors, nSamples);
       FactorMeans = NewVector<REAL>(nFactors);
       FactorMin = NewVector<REAL>(nFactors);
       FactorMax = NewVector<REAL>(nFactors);
       FactorSDs = NewVector<REAL>(nFactors);
       FactorClassMeans=NewMatrix<REAL>(nFactors, nClasses);
       FactorClassMin = NewMatrix<REAL>(nFactors, nClasses);
       FactorClassMax = NewMatrix<REAL>(nFactors, nClasses);
       FactorClassSDs = NewMatrix<REAL>(nFactors, nClasses);
       //test auto class naming (A,B,C...Z,AA,AB,AC..AZ,BA,BB...etc).
       //for(i=0; i<nClasses; i++)
       //   {
       //     for(l=1,k=i+1,j=1; k>0; k-=j,l++)
       //        {
       //          j*=26;
       //        }
       //     printf("%4d  %d   %s\n",i, l-1, ClassNames[i]);
       //     if(i==nClasses-1||0x1F==(i&0x1F)) getc(stdin);
       //   }
       nThreads=NumberOfProcessors()-1;
       ThreadHandles=NewVector<HANDLE>(nThreads);
       ThreadIds=NewVector<UINT>(nThreads);
       ThreadParams=NewVector<ThreadParam>(nThreads);
       MinPredictionRate=0.10;
       AbortFlag=false;
       if(Factor_PairList)
          return LoadFactors_Pairs();
       return true;
     }
  int PairNo(int i, int j)
     {
       if(i==j)
          return -1;
       if(i<j)
         {
           int tmp=i;
           i=j;
           j=tmp;
         }
       return ((i*i-i)>>1)+j;
     }
  bool LoadFactors_Pairs()
     {
       int i,j;
       FILE *f;

       //ToDo: adjust for factor names not numbers
       if(0!=fopen_s(&f, "Factor_PairList.txt", "rt"))
          return false;
       nPairs=0;
       while(2==fscanf_s(f, "%i\t%i",&i,&j))
             nPairs++;
       rewind(f);
       Pairs=NewVector<int>(nPairs);
       nPairs=0;
       while(2==fscanf_s(f, "%i\t%i",&i,&j))
             Pairs[nPairs++] = PairNo(i, j);
       SortInt(Pairs, nPairs);
       fclose(f);
       return nPairs>0;
     }
  bool SimulateNormalFactorValues(REAL frommean, REAL tomean,
                                  REAL fromvar, REAL tovar,
                                  bool HasMin=false, REAL Min=-REAL_MAX, 
                                  bool HasMax=false, REAL Max=REAL_MAX,
                                  int fromfactor=0, int nfactors=0,
                                  int fromsample=0, int nsamples=0
                                 )
     {
       int i, j;
       REAL **Index, **VarCovar, *Means;

       if(nfactors==0)
          nfactors=nFactors-fromfactor;
       if(nsamples==0)
          nsamples=nSamples-fromsample;
       Index=NewVector<REAL*>(nfactors);
       for(i=0; i<nfactors; i++)
           Index[i]=FactorData[fromfactor+i]+fromsample;
       VarCovar=NewMatrix<REAL>(nfactors, nfactors);
       Means = NewVector<REAL>(nfactors);
       nd.Randomize();
       for(i=0; i<nfactors; i++)
          {
            Means[i] = nd.RandRange(frommean, tomean);
            VarCovar[i][i] = nd.RandRange(fromvar, tovar);
            for(j=i+1; j<nfactors; j++)
                VarCovar[j][i]=VarCovar[i][j]=0;
          }
       nd.GetNormalValues(nfactors, nsamples, VarCovar, Means, Index,
                          HasMin, Min, HasMax, Max);
       DeleteVector(Index);
       DeleteVector(Means);
       DeleteMatrix((void**)VarCovar, nfactors);
       CalcMeansSDs();
       CalcClassMeansSDs();
       return true;
     }
  void CalcMeansSDs(int f=-1)
     {
       int from=(f<0)?0:f, to=f<0?nFactors-1:f;

       for(int i=from; i<=to; i++)
           CalcMeansSDs(FactorData[i], nSamples, NULL,
                        FactorMeans+i, FactorSDs+i,
                        FactorMin+i, FactorMax+i);
     }
  void CalcClassMeansSDs(int f=-1)
     {
       int i, j, n[MAX_CLASSES];
       int from=(f<0)?0:f, to=f<0?nFactors-1:f;
       REAL **ClassData=NewMatrix<REAL>(2, MaxCounts);

       for(i=from; i<=to; i++)
          {
            for(j=0; j<nClasses; j++)
                n[j]=0;
            for(j=0; j<nSamples; j++)
                ClassData[SampleClasses[j]][n[SampleClasses[j]]++]=FactorData[i][j];
            for(j=0; j<nClasses; j++)
                CalcMeansSDs(ClassData[j], ClassCounts[j], NULL,
                         FactorClassMeans[i]+j, FactorClassSDs[i]+j,
                         FactorClassMin[i]+j, FactorClassMax[i]+j);
          }
       DeleteMatrix(ClassData, 2); 
     }
  void CalcOneTail(int Spotter, WORD *sIndex, REAL *l, int *nTail, bool collect=false, BYTE Cat=0)
     {
       int i;
       
       if(!collect||Cat==0)
         {
           i=FindIndex(l[0], sIndex, FactorData[Spotter], nSamples>>1, nTail[0]);
           if(collect&&i==0)
              for(;nTail[0]<nSamples-1&&l[0]==FactorData[Spotter][sIndex[nTail[0]+1]];)
                   nTail[0]++;
           if(i<0)
              nTail[0]--;
           nTail[0]++;
         }
       if(!collect||Cat==1)
         {
           i=FindIndex(l[1], sIndex, FactorData[Spotter], nSamples, nTail[1]);
           if(collect&&i==0)
              for(;nTail[1]>0&&l[1]==FactorData[Spotter][sIndex[nTail[1]-1]];)
                   nTail[1]--;
           if(i>0)
              nTail[1]++;
           nTail[1]=nSamples-nTail[1];
         }
     }
  void CalcTails()
     {
       int i, *nTail;// Low:[0,Tail[0]-1], Hi:[Tail[0],nSamples-1]
       WORD *sIndex, **Tail;
       REAL l[2];

       for(i=0, Tail=Tails, nTail=nTails; i<nFactors; i++, nTail++, nTail++, Tail++, Tail++)
          {
            sIndex=SampleIndex+i*nSamples;
            l[0]=FactorMeans[i]-FactorSDs[i];
            l[1]=FactorMeans[i]+FactorSDs[i];
            CalcOneTail(i, sIndex, l, nTail);
            Tail[0]=sIndex;
            Tail[1]=sIndex+(nSamples-nTail[1]);
//char TestBuf[4096],Cap[64],*TestP=TestBuf;
//for(int j=0;j<nSamples;j++)
//{sprintf_s(TestP,sizeof(TestBuf)-(TestP-TestBuf),"%3d\t%7g\n",sIndex[j],FactorData[i][sIndex[j]]);
//TestP+=strlen(TestP);
//}
//sprintf_s(TestP,sizeof(TestBuf)-(TestP-TestBuf),
//          "Min=%7g\tMax=%7g\n"
//          "Mean=%7g\tSD=%7g\n"
//          "l0=%7g\tl1=%7g",FactorMin[i],FactorMax[i]
//                          ,FactorMeans[i],FactorSDs[i]
//                          ,l0,l1);
//sprintf_s(Cap,"Factor #: %d",i);
//MessageBox(0,TestBuf,Cap,MB_OK);
          }
     }
  void CalcMeansSDs(REAL *Data, int nsamples, int *samples,
                    REAL *Mean, REAL *SD, REAL *Min=0, REAL *Max=0)
     {
       int i;
       REAL d;

       if(Min!=NULL)
          *Min=*Max=*Data;
       for(*Mean=i=0; i<nsamples; i++)
        if(samples==NULL)
          {
            if(Min!=NULL)
              {
                if(Data[i]<*Min)
                   *Min=Data[i];
                else if(Data[i]>*Max)
                   *Max=Data[i];
              }
            *Mean+=Data[i];
          }
        else
          {
            if(Min!=NULL)
              {
                if(Data[samples[i]]<*Min)
                   *Min=Data[samples[i]];
                else if(Data[samples[i]]>*Max)
                   *Max=Data[samples[i]];
              }
            *Mean+=Data[samples[i]];
          }
       *Mean /= nsamples;
       if(SD==NULL)
          return;
       for(*SD=i=0; i<nsamples; i++)
          {
            if(samples==NULL)
                d=Data[i]-*Mean;
            else
                d=Data[samples[i]]-*Mean;
            *SD += d*d;
          }
       *SD = sqrt(*SD/nsamples);
     }
  bool InsertSingleMarkers(int fromFactor, int n,
                           REAL ave, REAL var, int span)
     {
       int i,j,k,l;
       REAL Ave[MAX_CLASSES], Buf[MAX_SAMPLES],
            SD=sqrt(var);

       if(fromFactor+n>=nFactors || fromFactor<0)
          return false;
       for(j=0; j<nClasses; j++)
           Ave[j]=ave+SD*j*span;// 6 + SD*32
       for(i=fromFactor; i<fromFactor+n; i++)
          {
            REAL mn=REAL_MAX, mx=-REAL_MAX;

            for(j=0; j<nClasses; j++)
               {
                 if(!nd.GetNormalValues(ClassCounts[j], var, Ave[j], Buf))
                    return false;
                 for(k=l=0; l<ClassCounts[j]; k++)
                  if(j==SampleClasses[k])
                     FactorData[i][k]=Buf[l++];
               }
            CalcMeansSDs(FactorData[i], nSamples, NULL,
                         FactorMeans+i, FactorSDs+i,
                         FactorMin+i, FactorMax+i);
          }
       return true;
     }

  bool InsertPairMarkers(int fromFactor, int n, REAL ave, REAL var)
     {
       int i,j,k,l,h,q;
       REAL **Buf, v1[]={var,0},  v2[]={0,var}, *v[]={v1,v2},
              aves[2][4]={{ave*(REAL)1.5, ave*(REAL)1.5, ave, ave},
                          {ave, ave*(REAL)1.5, ave*(REAL)1.5, ave}},
              **varcovar=v, *I[2];

       if(fromFactor+n+n>=nFactors || fromFactor<0)
          return false;
       Buf = NewMatrix<REAL>(2, MaxCounts);
       nd.Randomize();
       for(i=fromFactor; i<fromFactor+n+n; i++,i++)
          {
            for(j=0; j<nClasses; j++)
               {
                 h=ClassCounts[j]>>1;
                 int ss[2]={0, h}, nn[2]={h, ClassCounts[j]-h};
                 for(q=0; q<2; q++)
                    {
                      REAL *means[2]={aves[0]+q+q, aves[1]+q+q};
                      I[0] = Buf[0]+ss[q];
                      I[1] = Buf[1]+ss[q];
                      if(!nd.GetNormalValues(2,nn[q],varcovar,means[j],I))
                        {
                          DeleteMatrix((void**)Buf,2);
                          return false;
                        }
                    }
                 for(k=l=0; l<ClassCounts[j]; k++)
                  if(j==SampleClasses[k])
                    {
                      FactorData[i][k]=Buf[0][l];
                      FactorData[i+1][k]=Buf[1][l++];
                    }
               }
            for(j=i; j<i+2; j++)
                CalcMeansSDs(FactorData[i], nSamples, NULL,
                     FactorMeans+j, FactorSDs+j,
                     FactorMin+j, FactorMax+j);
          }
       DeleteMatrix((void**)Buf,2);
       return true;
     }

  bool InsertSpotter(int spotter, int nMis, int*Mis, BYTE Cat)
     {
       int i;
       REAL t;
       
       srand(Randomize?(unsigned)time(0):0);
       for(i=0; i<nMis; i++)
          {
            t=(REAL)rand()/100.0/RAND_MAX;
            if(Cat==0)
               FactorData[spotter][Mis[i]]= 
                          (FactorMin[spotter]-=t);
            else
               FactorData[spotter][Mis[i]]= 
                          (FactorMax[spotter]+=t);
          }
       CalcMeansSDs(spotter);
       return true;
     }
  bool InsertDistortion(Distortion &d)
     {
       int i, j, Marker = d.Distorted[0];;
       REAL &MidPoint=FactorClusters[Marker].MidPoint;

       for(i=0; i<d.nDistorted; i++)
        if(d.Distorted[i]<0||d.Distorted[i]>=nFactors)
           return false;
       DoClustering(Marker);
       for(i=0; i<d.nANDed; i++)
          {
            int *Mis=d.ANDedBlocks[i].Mis;
            for(j=0; j<d.ANDedBlocks[i].nMis; j++)
             if(SampleClasses[d.ANDedBlocks[i].Mis[j]]!=
                d.ANDedBlocks[i].Class
               )
                return false;
             else FactorData[Marker][Mis[j]]=FactorData[Marker][Mis[j]]>MidPoint?FactorMin[Marker]:FactorMax[Marker];
            CalcMeansSDs(Marker);
            for(j=0; j<d.ANDedBlocks[i].nORed; j++)
             if(!InsertSpotter(d.ANDedBlocks[i].ORedBlocks[j].Spotter,
                               d.ANDedBlocks[i].nMis,
                               d.ANDedBlocks[i].Mis,
                               d.ANDedBlocks[i].ORedBlocks[j].Category
                              )
               ) return false;
          }
       DoClustering(Marker);
       return true;
     }

  bool Save(const char* FileName)
     {
       FILE *f;
       int i, j;

       if(0==fopen_s(&f, FileName, "wt"))
         {
           fprintf(f, "%s\n%s\n", Source, Title);
           for(i=0; i<nClasses; i++)
              {
                fprintf(f, ClassNames[i]);
                fprintf(f, (i<nClasses-1)?" ":"\n");
              }
           fprintf(f, "%d %d\n", nFactors, nSamples);
           for(i=0; i<nSamples; i++)
               fprintf(f, i<nSamples-1?"%s\t":"%s\n", SampleNames[i]);
           for(i=0; i<nSamples; i++)
               fprintf(f, i<nSamples-1?"%d\t":"%d\n", SampleClasses[i]);
           for(i=0; i<nFactors; i++)
              {
                fprintf(f, "%s\t", FactorNames[i]);
                for(j=0; j<nSamples; j++)
                    fprintf(f,(j<nSamples-1)?"%f\t":"%f\n",FactorData[i][j]);
              }
           fclose(f);
           return true;
         }
       return false;
     }
  static bool CheckSimulated(char *src)
     {
       return !_strnicmp(src, SimulatedTag, strlen(SimulatedTag));
     }
  bool Load(const char* FileName, bool sim=false)
     {
       FILE *f;
       int i, j, k;
       char *p, *q;
       char Buf[2048];
       char *frmt0[2]={"%f\t","%f\n"}, *frmt1[2]={"%lf\t","%lf\n"},
            **frmt=sizeof(REAL)==4?frmt0:frmt1;

       strcpy_s(DatasetFileName, FileName);
       Reset();
       if(0!=fopen_s(&f, DatasetFileName, "rt"))
          Exit(1, "Failed to open Dataset file!");
       fgets(Source, sizeof(Source), f);
       Source[strlen(Source)-1]='\0';
       if(!sim)
          Simulated = CheckSimulated(Source);
       fgets(Title, sizeof(Title), f);
       Title[strlen(Title)-1]='\0';
       fgets(Buf, sizeof(Buf), f);
       nClasses=0;
       for(p=Buf, q=ClassNames[nClasses]; *p&&*p!='\n'; p++)
        if(*p==' ')
          {
            *q='\0';
            q=ClassNames[++nClasses];
            if(nClasses>=MAX_CLASSES)
               Exit(1, "Too Many Class Names");
          }
       else *q++=*p;
       *q++='\0';
       if(++nClasses<2)
          Exit(1, "Too few classes!");
       fscanf_s(f, "%d %d\n", &nFactors, &nSamples);
       if(nFactors<1)
          Exit(1, "Too few factors");
       if(nSamples<2)
          Exit(1, "Too few samples");
       if(nSamples>MAX_SAMPLES)
          Exit(1, "Too many samples!");
       fgets(Buf, sizeof(Buf), f);
       for(i=0,p=Buf,q=SampleNames[0]; *p&&*p!='\n'; p++)
        if(*p=='\t')
          {
            *q='\0';
            q=SampleNames[++i];
            if(i>=nSamples)
               Exit(1, "Too many samples found!");
          }
        else *q++=*p;
       *q++='\0';
       if(++i<nSamples)
          SampleNames[0][0]=0;
       for(i=0; i<nSamples; i++)
          {
            fscanf_s(f, "%d ", &j);
            SampleClasses[i] = (BYTE)j;
          }
       FactorNames = NewVector<char*>(nFactors);
       if(sim)
         {
           FactorNames[0]=NULL;
           fclose(f);
           if(!Init()) return false;
         }
       else
         {
           for(i=0, p=FactorNameBuf; i<nFactors; i++)
              {
                fscanf_s(f, "%s", p, 256);
                FactorNames[i]=p;
                p+=strlen(p)+1;
                fgets(Buf, sizeof(Buf), f);
              }
           if(!Init())
             {
               fclose(f);
               return false;
             }
           LoadFactorData(f);
           fclose(f);
         }
       CndH_1F=NewVector<REAL>(nFactors);
       SampleIndex = NewVector<WORD>(nFactors*nSamples);
       FactorClusters=NewVector<ClusterType_1F>(nFactors);
       Tails=NewVector<WORD*>(nFactors<<1);
       nTails=NewVector<int>(nFactors<<1);
       return CalculateIndices();
     }
bool LoadFactorData(FILE *f)
     {
       int i, j;
       char Buf[2048], *p, *q;

       rewind(f);
       for(i=0; i<6; i++)
           fgets(Buf, sizeof(Buf), f);
       for(i=0; i<nFactors; i++)
          {
            fgets(Buf, sizeof(Buf), f);
            char *p, *q;
            for(p=Buf; *p!='\t'; p++)
             if(*p==0)
                Exit(1, "Bad dataset file format!");
            p++;    
            for(j=0; j<nSamples; j++)
               {
                 for(q=p; *q!='\t'&&*q!='\n'&&*q!=0; q++);
                 *q++=0;
                 FactorData[i][j] = atof(p);
                 p=q;
               }
          }
       return true;
     }
bool CalculateIndices()
     {
       int i, j, k;

       for(i=0; i<nClasses; i++)
           ClassCounts[i]=0;
       for(i=0; i<nSamples; i++)
           ClassCounts[SampleClasses[i]]++;
       for(i=MaxCounts=0; i<nClasses; i++)
        if(MaxCounts<ClassCounts[i])
           MaxCounts=ClassCounts[i];
       HC = H(nSamples, ClassCounts);

       WORD *pSI=SampleIndex;
       for(i=0; i<nFactors; i++, pSI+=nSamples)
          {
            SequenceFill(pSI, nSamples);
            SortIndex(pSI, FactorData[i], nSamples);
          }

       CalcMeansSDs();
       CalcClassMeansSDs();
       CalcTails();
       for(i=0; i<nFactors; i++)
           DoClustering(i);
       for(i=0; i<nFactors; i++)
          {
            ZeroMemory(FactorFreq[i], FactorFreqSize*(nClasses+1));
            REAL Step=(FactorMax[i]-FactorMin[i])/FactorFreqSize;
            for(j=0; j<nSamples; j++)
               {
                 k=(int)((FactorData[i][j]-FactorMin[i])/Step);
                 if(k<0)
                    k=0;
                 if(k>=FactorFreqSize)
                    k=FactorFreqSize-1;
                 FactorFreq[i][k]++;
                 FactorFreq[i][k+(SampleClasses[j]+1)*FactorFreqSize]++;
               }
          }
       FactorNameIndex = NewVector<int>(nFactors);
       FactorNameIndexDsc = NewVector<int>(nFactors);
       FactorMIIndex = NewVector<int>(nFactors);
       FactorMIIndexDsc = NewVector<int>(nFactors);
       FactorNoIndex = NewVector<int>(nFactors);
       for(i=0; i<nFactors; i++)
           FactorNameIndex[i]=FactorNameIndexDsc[i]=
           FactorMIIndex[i]=FactorMIIndexDsc[i]=
           FactorNoIndex[i]=i;
       SortIndexStri(FactorNameIndex, FactorNames, nFactors);
       SortIndexStriDsc(FactorNameIndexDsc, FactorNames, nFactors);
       SortIndexDsc(FactorMIIndex, CndH_1F, nFactors);
       SortIndex(FactorMIIndexDsc, CndH_1F, nFactors);
       return true;
     }
  bool ReloadFactorData()
     {
       FILE *f;

       if(0!=fopen_s(&f, DatasetFileName, "rt"))
          Exit(1, "Failed to open Dataset file!");
       LoadFactorData(f);
       fclose(f);
       return CalculateIndices();
     }
  bool Simulate(const char* SimFileName, const char* TargetFileName)
     {
       FILE *f;
       int i;
       unsigned fromF, nF;
       REAL ave, var, span, FromAve, ToAve, FromVar, ToVar;
       char c, Buf[MAX_PATH+1], *p, *q;
       Distortion distortion;

       if(!_stricmp(SimFileName, TargetFileName))
          return false;
       Simulated = true;
       if(   !Load(SimFileName, Simulated)
          || 0!=fopen_s(&f,SimFileName,"rt")
         )
          return Simulated = false;
       sscanf_s(Source, "FromAve=%lf\tToAve=%lf\t"
                        "FromVar=%lf\tToVar=%lf",
                  &FromAve, &ToAve, &FromVar, &ToVar);
       for(i=0; i<6;)
          {
            c=getc(f);
            if(c=='\n')
               ++i;
            if(c==EOF)
              {
                fclose(f);
                return Simulated = false;
              }
          }
       SimulateNormalFactorValues(FromAve, ToAve, FromVar/*0.01*/, ToVar/*3.5*/, true, 0);
       while((c=getc(f))!=EOF)
            {
              if(c==' '||c=='\t'||c=='\n'||c=='\r')
                 continue;
              if((c|32)=='m')// insert markers
                {
                  c=getc(f);
                  if((c|32)=='s')// single markers
                    {
                      if(   !GetUnsigned(f, fromF)
                         || !GetUnsigned(f, nF)
                         || !GetREAL(f, ave)
                         || !GetREAL(f, var)
                         || !GetREAL(f, span)
                        )
                        {
                          fclose(f);
                          Simulated = false;
                          Exit(1, "Bad Simfile format!");
                        }
                      InsertSingleMarkers(fromF-1, nF, ave, var, span);
                    }
                  else if((c|32)=='p')// pair markers
                    {
                      if(   !GetUnsigned(f, fromF)
                         || !GetUnsigned(f, nF)
                         || !GetREAL(f, ave)
                         || !GetREAL(f, var)
                        )
                        {
                          fclose(f);
                          Simulated = false;
                          Exit(1, "Bad Simfile format!");
                        }
                      InsertPairMarkers(fromF-1, nF, ave, var);
                    }
                  else
                    {
                      fclose(f);
                      return Simulated = false;
                    }
                }
              else if((c|32)=='d')// insert distortion
                {
                  c=getc(f);
                  if((c|32)=='m')// insert multi distortion
                    {
                      if(  !distortion.ReadMulti(f)
                         ||!distortion.ParseMulti()
                        )
                        {
                          fclose(f);
                          return Simulated = false;
                        }
                    }
                  else// insert single distortion
                    {
                      ungetc(c, f);
                      while((c=getc(f))==' '||c=='\t'||c=='\n'||c=='\r')
                           {}
                      if(c==EOF)
                        {
                          fclose(f);
                          return Simulated = false;
                        }
                      distortion.Read(f);
                    }
                  if(!InsertDistortion(distortion))
                    {
                      fclose(f);
                      return Simulated = false;
                    }
                  FILE *fa;
                  strcpy_s(Buf, SimFileName);
                  q=p=Buf+strlen(Buf);
                  while(*--p!='.');
                  if(q<Buf+sizeof(Buf)-2)
                    {
                      for(;q>=p;q--)
                          *(q+2)=*q;
                      *p++='_';
                      *p='P';
                      if(0==fopen_s(&fa, Buf,"wt"))
                        {
                          distortion.Write(fa);
                          fclose(fa);
                        }
                    }
                }
              else return Simulated = false;
            }
       fclose(f);
       time_t t;
       time(&t);
       sprintf_s(Source, "%s at %s", SimulatedTag, ctime(&t));
       Source[strlen(Source)-1]=0;
       if(Save(TargetFileName))
         {//rename retval: 22:EINVAL, 2:ENOENT, 13:EACCES
           DeleteFile("Syn2-bak.bin");
           DeleteFile("Syn2-bak.txt");
           rename("Syn2.bin","Syn2-bak.bin");
           rename("Syn2.txt","Syn2-bak.txt");
           return true;
         }
       else return false;
     }
  REAL CndH(int nD, int nsamples, REAL **Data, BYTE *Mask)
     {
       int i, n=1<<(MAX_DIMENSIONS+1),
           EndClusteing=2, StartMeasure=1<<nD;
       REAL BestScore;
       Cluster Clusters[1<<(MAX_DIMENSIONS+1)];
       UPGMA upgma;

       upgma.Init(nsamples, nD, Clusters,
                  StartMeasure, this, Mask, nClasses, SampleClasses,
                  /*MinClusterSize,*/ Separation);
       upgma.DoClustering(EndClusteing, BestScore, Data);
       return HC - BestScore;
     }
  static unsigned int WINAPI Thread(ThreadParam *param)
     {
       (param->ds->*(param->CallBack))(param);
       _endthreadex(0);
       return 0;
     }
  int* GetMore()
     {
       if(ThreadsPtr<ThreadsEndPtr)
          return *ThreadsPtr++;
       else
          return NULL;
     }

  void CalcROC_Steps(int Marker)
     {
       int i;
       REAL Step, Epsilon=1.0e-10,
            Range = (FactorMax[Marker]-FactorMin[Marker]),
            *&RocSteps = ROC_Steps[Marker];

       RocSteps[nROC_Steps-1]=FactorMax[Marker]+Epsilon;
       RocSteps[0]=FactorMin[Marker]-Epsilon;
       Step=(Range+Epsilon+Epsilon)/(nROC_Steps-1);
       for(i=1; i<nROC_Steps-1; i++)
           RocSteps[i]=RocSteps[i-1]+Step;
     }

  int ClassifySample(REAL SampleValue, REAL TestPoint, BYTE Direction)
     {
       return Direction==0? (SampleValue<=TestPoint?0:1)
                          : (SampleValue>=TestPoint?0:1);
     }
  bool SampleInTail(int Sample, SpotterType &md)
     {
       REAL val = FactorData[md.Spotter][Sample],
              P = md.Percentile;

       return P>FactorMeans[md.Spotter]? val >= P  // High Tail (Category=1)
                                       : val <= P; // Low  Tail (Category=0);
     }
  void DoTest(Predictor &pr, bool Dist, bool NoDist)
     {
       int i, j, k, LastStep=nROC_Steps-1, TFPN[3],
           nS=nSamples, &Marker=*pr.Marker;
       bool DistAndMine, flip;
       BYTE TrueC, TestC;
       REAL *&RocSteps=ROC_Steps[Marker], data, data1,
            &MidPoint=FactorClusters[Marker].MidPoint;

#ifdef TRAIN_ALL
       nS=(int)(nS*.9);//try train with all samples
#endif
       
       for(i=nS; i<nSamplesTest; i++)
          {
            TrueC=SampleClasses[i];
            data=data1=FactorData[Marker][i];

            TestC = ClassifySample(data, MidPoint, pr.Direction);
            DistAndMine=Dist && pr.Class==TrueC;
            flip=false;
            if(DistAndMine)
              {
                for(j=0; j<pr.nSpotters&&!flip; j++)
                 if(SampleInTail(i, pr.Spotters[j]))
                    flip=true;
                if(flip)
                   data = MidPoint + MidPoint - data;
              }
            else if(!NoDist)
                    continue;
            for(k=0; k<nROC_Steps; k++)
               {
                 j=pr.Direction==0?k:LastStep-k;
                 if(DistAndMine)
                   {
                     TestC = ClassifySample(data, RocSteps[j], pr.Direction);
                     TFPN[1] = TrueC?(TestC?0   // TP
                                           :3)  // FN
                                    :(TestC?1   // FP
                                           :2); // TN
                     TestC = ClassifySample(data1, RocSteps[j], pr.Direction);
                     TFPN[0] = TrueC?(TestC?0   // TP
                                           :3)  // FN
                                    :(TestC?1   // FP
                                           :2); // TN
                     pr.Counters[0].Counters[k][TFPN[0]]++;
                     pr.Counters[1].Counters[k][TFPN[1]]++;
                   }
                 if(NoDist)
                   {
                     if(DistAndMine)
                        TFPN[2] = TFPN[0];
                     else
                       {
                         TestC= ClassifySample(data1, RocSteps[j], pr.Direction);
                         TFPN[2] = TrueC?(TestC?0   // TP
                                               :3)  // FN
                                        :(TestC?1   // FP
                                               :2); // TN
                       }
                     pr.Counters[2].Counters[k][TFPN[2]]++;
                   }// if(NoDist)
               }// for(k=0; k<nROC_Steps; k++)
          }// for(i=nS; i<nSamplesTest; i++)
     }
  void CalcCandidateSpotters()
     {
       if(CandidateSpottersMask==NULL)
          CandidateSpottersMask=NewVectorZero<BYTE>(nFactors);
       for(int i=0,j=(i<<1),k=j+1; i<nFactors; i++,j++,j++,k++,k++)
          {
            CandidateSpottersMask[i]=0;
            if(nTails[j]>=MinMis&&nTails[j]<=MaxMis)
               CandidateSpottersMask[i]|=1;
            if(nTails[k]>=MinMis&&nTails[k]<=MaxMis)
               CandidateSpottersMask[i]|=2;
          }
     }
  static void Exit(int i, const char *msg)
     {
       MessageBox(0, msg, "Error!", MB_OK);
       fcloseall();
       exit(i);
     }
  void Calc_nParts(int n)
     {
       nParts = nThreads*10;
       int i = (n + 3) >> 2,
           j = (n + 2047) >> 11;
       if(nParts > i)
          nParts = i;
       else if(nParts < j)
          nParts = j;
     }
  bool Calc_1Factor(void Progress()=0)
     {
       int i, j, k, *p;

       //nThreads=1;
       if(!Syn2Found)
          LoadSyn2();
       CalcCandidateSpotters();
       FactorICorr=NewMatrix<REAL>(nFactors, 2);
       for(i=0; i<nFactors; i++)
           FactorICorr[i][0]=
           FactorICorr[i][1]=HC-CndH_1F[i];
       HeatMaps=NewVector<HeatMap>(nThreads);
       Calc_nParts(nFactors);
       OffsetsTable = NewMatrix<int>(nParts,2);
       ThreadsPtr = OffsetsTable;
       ThreadsEndPtr = ThreadsPtr + nParts;
       DivideVector(nFactors, nParts, OffsetsTable);
       for(i=0; i<nThreads; i++)
          {
            ThreadParams[i].predictors[0][0].nD=
            ThreadParams[i].predictors[0][1].nD=
            ThreadParams[i].predictors[1][0].nD=
            ThreadParams[i].predictors[1][1].nD=1;
            ThreadParams[i].Index=i;
            ThreadParams[i].ds=this;
            ThreadParams[i].nD=1;
            ThreadParams[i].CallBack=&Dataset::Calc_1Factor;
            ThreadParams[i].hm=HeatMaps+i;
            ThreadParams[i].Progress=Progress;
            ThreadHandles[i]=(HANDLE)_beginthreadex(NULL, 0,
                                   (ThreadAPI)Thread, ThreadParams+i,
                                                     0, ThreadIds+i); 
            if(ThreadHandles[i]==NULL) 
              {
                char MsgBuf[512];
                sprintf_s(MsgBuf,"Failed to create thread (errno: %d)\n", errno);
                while(--i>=0)
                      CloseHandle(ThreadHandles[i]);
                DeleteMatrix((void**)OffsetsTable, nParts);
                DeleteVector(HeatMaps);
                return false;
                //Exit(1, MsgBuf);
              }
          }
       WaitForMultipleObjects(nThreads, ThreadHandles, TRUE, INFINITE);
       for(i=0; i<nThreads; i++)
           CloseHandle(ThreadHandles[i]);
       if(ThreadsPtr<ThreadsEndPtr) // aborted
         {
           //EnterCriticalSection(&critSecAbort);
           AbortFlag=false;
           //LeaveCriticalSection(&critSecAbort);
           DeleteMatrix((void**)OffsetsTable, nParts);
           DeleteVector(HeatMaps);
           return false;
         }
       if(!Testing)
       for(i=0; i<MAXFREQ; i++)
          {
            DistFreq_1F[i]=ThreadParams[0].DistFreq[i];
            for(j=1; j<nThreads; j++)
                DistFreq_1F[i]+=ThreadParams[j].DistFreq[i];
          }
       if(!Permuting && !Testing)
         {
           for(i=0; i<CountOf(IFreq_1F); i++)
              {
                IFreq_1F[i] = ThreadParams[0].IFreq[i];
                for(j=1; j<nThreads; j++)
                    IFreq_1F[i] += ThreadParams[j].IFreq[i];
              }
           //ZeroMem(ICorrFreq_1F, sizeof(ICorrFreq_1F));
           for(i=0; i<FREQSIZE; i++)
               ICorrFreq_1F[i]=0;
           for(i=0; i<nFactors; i++)
              {
                j=FactorICorr[i][0]*MAXFREQ;
                if(j>=MAXFREQ)
                   j= MAXFREQ-1;
                LOWLONG(ICorrFreq_1F[j])++;
                j=FactorICorr[i][1]*MAXFREQ;
                if(j>=MAXFREQ)
                   j= MAXFREQ-1;
                HIGHLONG(ICorrFreq_1F[j])++;
              }
            MaxHeatMap=0;
            for(i=0; i<HEATMAPHEIGHT; i++)
            for(j=0; j<HEATMAPWIDTH; j++)
               {
                 Syn2_Score_HeatMap[i][j]=ThreadParams[0].hm->hm[i][j];
                 for(k=1; k<nThreads; k++)
                     Syn2_Score_HeatMap[i][j]+=ThreadParams[k].hm->hm[i][j];
                 p=(int*)(Syn2_Score_HeatMap[i]+j);
                 k=*p+p[1];
                 if(MaxHeatMap<k)
                    MaxHeatMap=k;
               }
            if(SaveResults)
               SaveI();
          }
       DeleteMatrix((void**)OffsetsTable, nParts);
       DeleteVector(HeatMaps);
       DeleteMatrix(FactorICorr, nFactors);
       return true;
     }
  void DoClustering(int Marker)//, Cluster *Clusters, REAL &limit, REAL &minH)
     {// Assuming exactly two clusters
       int i, j, k, l, L[2], sc[2], Counts[2][MAX_SAMPLES];
       ClusterType_1F &Clusters=FactorClusters[Marker];
       REAL h, *data=FactorData[Marker],
            &minH=CndH_1F[Marker], &limit=Clusters.MidPoint;
       WORD *indx=SampleIndex+Marker*nSamples;

       if(FactorMax[Marker]-FactorMin[Marker]<1.0e-9)
         {
           minH=1.0;
           limit=FactorMin[Marker];
           return;
         }
       Counts[0][0]=Counts[1][0]=sc[0]=sc[1]=0;
       for(i=0; i<nSamples; i++)
          {
            sc[SampleClasses[indx[i]]]++;
            Counts[0][i]=sc[0];
            Counts[1][i]=sc[1];
          }
       Clusters.ClassCounts[0][0]=0;//Counts[0][0];
       Clusters.ClassCounts[0][1]=0;//Counts[1][0];
       Clusters.ClassCounts[1][0]=ClassCounts[0];//-Counts[0][0];
       Clusters.ClassCounts[1][1]=ClassCounts[1];//-Counts[1][0];
       Clusters.nSamples[0]=0;//Clusters[0].ClassCounts[0]+Clusters[0].ClassCounts[1];
       Clusters.nSamples[1]=nSamples;//Clusters[1].ClassCounts[0]+Clusters[1].ClassCounts[1];
       h = minH = 100;
       j=0;
//if(Marker==4168)
//{
//int ttt=0;
//ttt=j;
//}
       for(i=0; i<nSamples; i++)
          {
            L[0]=L[1]=0;
            L[SampleClasses[indx[i]]]++;
            //for(k=i+1, l=0;    k<nSamples
            //                && (   SampleClasses[indx[k]]==SampleClasses[indx[i]]
            //                    //|| data[indx[k]]==data[indx[k-1]]
            //                   )
            //              ; k++
            //   )
            //   {
            //     l++;
            //     L[SampleClasses[indx[k]]]++;
            //   }
            //i+=l;
            Clusters.ClassCounts[0][0]+=L[0];
            Clusters.ClassCounts[0][1]+=L[1];
            Clusters.ClassCounts[1][0]-=L[0];
            Clusters.ClassCounts[1][1]-=L[1];
            Clusters.nSamples[0]+=L[0]+L[1];
            Clusters.nSamples[1]-=L[0]+L[1];
            if(i>=MinClusterSize && i<=nSamples-MinClusterSize)
              {
                h = CondH(Clusters);
                if(minH>h)
                  {
                    minH = h;
                    j = i;
                  }
              }
          }
//if(minH>1)
//{
//int ttt=0;
//ttt=j;
//}
       Clusters.ClassCounts[0][0]=Counts[0][j];
       Clusters.ClassCounts[0][1]=Counts[1][j];
       Clusters.ClassCounts[1][0]=ClassCounts[0]-Counts[0][j];
       Clusters.ClassCounts[1][1]=ClassCounts[1]-Counts[1][j];
       Clusters.nSamples[0]=Clusters.ClassCounts[0][0]+Clusters.ClassCounts[0][1];
       Clusters.nSamples[1]=Clusters.ClassCounts[1][0]+Clusters.ClassCounts[1][1];
       Clusters.ClassSamples[0]=indx;
       Clusters.ClassSamples[1]=indx+j+1;
       limit = data[indx[j]];
       if(++j<nSamples)
          limit = (limit + data[indx[j]])/2.0;
       Clusters.Classes[0]=Clusters.ClassCounts[0][1]>Clusters.ClassCounts[0][0];
       Clusters.Classes[1]=Clusters.ClassCounts[1][1]>Clusters.ClassCounts[1][0];
       if(Clusters.Classes[0]==Clusters.Classes[1])
         {
           if(Clusters.nSamples[0]>Clusters.nSamples[1])
              Clusters.Classes[0]=1-Clusters.Classes[0];
            else Clusters.Classes[1]=1-Clusters.Classes[1];
         }
       Clusters.Direction=Clusters.Classes[0];
       BYTE *Membership=ClustersMembership+(Marker*nSamples), b;
       for(b=0; b<2; b++)
       for(i=0; i<Clusters.nSamples[b]; i++)
           Membership[Clusters.ClassSamples[b][i]]=b;
//if(Marker==4560)
//{
//int ttt=0;
//ttt=j;
//}
#ifdef _DEBUG
if(Clusters[0].Class==Clusters[1].Class)
{int ttt=0;
ttt = j;
}
#endif
     }
  void Calc_1Factor(ThreadParam* param)
     {
       int i, j, *Offsets;
       REAL Step = (REAL)1.0/(CountOf(IFreq_1F));
       FILE *f=0;
       char Buf[256];
       bool AbFlag;

       for(i=0; i<FREQSIZE; i++)
          {
            param->IFreq[i] = 0;
            param->DistFreq[i]=0;
          }
       for(i=0; i<HEATMAPHEIGHT; i++)
       for(j=0; j<HEATMAPWIDTH; j++)
           param->hm->hm[i][j]=0;
       if(!Testing && SaveResults && !FrequencyOnly)
         {
           sprintf_s(Buf, "Dist1_%d.bin", param->Index);
           if(   0!=fopen_s(&f, Buf, "wb")
              || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
             )
             {
               char Msg[128];
               sprintf_s(Msg, "Failed to open file %s", Buf);
               Exit(1, Msg);
             }
         }
       else f=NULL;
       param->f=f;
       param->DistStep = (REAL)1.0/(CountOf(DistFreq_1F));
       do{
           EnterCriticalSection(&critSecAbort);
           AbFlag=AbortFlag;
           LeaveCriticalSection(&critSecAbort);
           if(AbFlag)
              break;
           EnterCriticalSection(&critSec);
           Offsets=GetMore();
           //if(Offsets) printf_s("\n%d \n", Offsets[0]);
           LeaveCriticalSection(&critSec);
           if(Offsets==NULL)
              break;
           for(i=Offsets[0]; i<=Offsets[1]; i++)
              {
                if(FactorMask && FactorMask[i]==0)
                   continue;
                if(Testing)
                   MidPoints[i]+=FactorClusters[i].MidPoint;
                j=int((HC-CndH_1F[i])/Step);
                
                if(j<0)
                   j=0;
                if(j>=CountOf(IFreq_1F))
                   j=CountOf(IFreq_1F)-1;
                param->IFreq[j]++;
                param->Marker=&i;
                FindDistortions(param);
              }
           if(param->Progress)
             {
               EnterCriticalSection(&critSecProgress);
               param->Progress();
               LeaveCriticalSection(&critSecProgress);
             }
         } while(true);
       if(f)
          fclose(f);
     }
  bool SaveI()
     {
       int i;
       FILE *f;
       REAL step=(REAL)1.0/FREQSIZE, start=step/2;

       if(   0!=fopen_s(&f, MI_Fname, "wt")
          || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
         )
          return false;
       for(i=0; i<nFactors; i++)
           fprintf_s(f, "%s\t%8.5f\n", FactorNames[i],HC-CndH_1F[i]);
       for(i=0; i<FREQSIZE; i++, start+=step)
           fprintf_s(f, "%6.3f\t%i\t%i\t%i\n", start,
                     IFreq_1F[i],
                     LOWLONG(ICorrFreq_1F[i]),
                     HIGHLONG(ICorrFreq_1F[i])
                    );
       fclose(f);
       return true;
     }
  bool LoadIFreq_1F()
     {
       int i;
       REAL x;
       FILE *f;

       if(   0!=fopen_s(&f, MI_Fname, "rt")
          || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
         )
          return false;
       for(i=0; i<nFactors; i++)
        if(!GetEOL(f))
           break;
       if(i<nFactors)
         {
           fclose(f);
           return false;
         }
       for(i=0; i<MAXFREQ; i++)
          {
            if(!GetREAL(f, x))
               break;
            if(!GetInt(f, IFreq_1F[i]))
               break;
            if(!GetInt(f, LOWLONG(ICorrFreq_1F[i])))
               break;
            if(!GetInt(f, HIGHLONG(ICorrFreq_1F[i])))
               break;
          }
       fclose(f);
       return i==MAXFREQ;
     }
  bool SaveDist_1Factor(char *Dist_Single)
     {
       int i, j=0, bufLen=0;
       FILE *f=0, *fn=0;
       REAL step=(REAL)1.0/FREQSIZE, start=step/2;
       char Buf[16];
       DistortionInstance Dist;
  
       if(   NULL!=fopen_s(&f, Dist_Single, "wt")
          || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
         )
          return false;
       if(!FrequencyOnly)
         {
           int nSigDist=0;
           //int SigSize= sizeof(Dist)
           //            -sizeof(Dist.Precision)
           //            //-sizeof(Dist.Cohesion)
           //            -sizeof(Dist.Score)
           //            -sizeof(Dist.InfSyn)
           //            -sizeof(Dist.HypSyn)
           //            -sizeof(Dist.nMis)
           //            -sizeof(Dist.Mis);
           //if(EvaluateCorrectionRates)
           //   for(i=0; i<CountOf(DistFreq_1F); i++)
           //       DistFreq_1F[i]=0;
           WriteDistFileHeader(1, f);
           for(int th=0; th<nThreads; th++)
              {
                //i=0;
                sprintf_s(Buf, "Dist1_%d.bin", th);
                if(   NULL!=fopen_s(&fn, Buf, "rb")
                   || 0!=setvbuf(fn, 0, _IOFBF, 1024*1024)
                  )
                  {
                    fclose(f);
                    return false;
                  }
                //if(EvaluateCorrectionRates)
                //  {
                //    MarkerCorrection1 m;
                //    char b[4096], 
                //         *FP=b, *FP_Cor=b+1024, *FN=b+2048, *FN_Cor=b+3072;
                //
                //    while(   fread(&m.Marker,sizeof(int),1,fn)
                //          && fread(&m.MI,sizeof(REAL),1,fn)
                //          && fread(&m.MI_Cor,sizeof(REAL),1,fn)
                //          && fread(m.Mis,nSamples,1,fn)
                //         )
                //         {
                //           char *F[][2]={{FP, FP_Cor}, {FN, FN_Cor}};
                //           int len[][2]={{1024,1024}, {1024,1024}};
                //           fprintf_s(f,"%s\t%6.3f\t%6.3f\t",
                //                     FactorNames[m.Marker], m.MI, m.MI_Cor);
                //           i=(int)(m.MI_Cor*CountOf(DistFreq_1F));
                //           if(i>99)
                //              i=99;
                //           else if(i<0)
                //                   i=0;
                //           DistFreq_1F[i]++;
                //           int nFP=0, nFP_C=0, nFN=0, nFN_C=0;
                //           for(i=0; i<nSamples; i++)
                //              {
                //                int s=m.Mis[i]-1;
                //                if(s<0)
                //                   continue;
                //                BYTE c=SampleClasses[i];
                //                char *sni=SampleNames[i];
                //                if(c==0)
                //                  {
                //                    nFP++;
                //                    if(s==0)
                //                       nFP_C++;
                //                  }
                //                else if(c==1)
                //                  {
                //                    nFN++;
                //                    if(s==0)
                //                       nFN_C++;
                //                  }
                //                j=(int)(strlen(sni)+1);
                //                sprintf_s(F[c][0], len[c][0], "%s ", sni);
                //                F[c][0]+=j;
                //                len[c][0]-=j;
                //                if(s==0)
                //                  {
                //                    sprintf_s(F[c][1],len[c][1],"%s ",sni);
                //                    F[c][1]+=j;
                //                    len[c][1]-=j;
                //                  }
                //              }
                //           FP[1024-len[0][0]-(len[0][0]==1024?0:1)]=0;
                //           FP_Cor[1024-len[0][1]-(len[0][1]==1024?0:1)]=0;
                //           FN[1024-len[1][0]-(len[1][0]==1024?0:1)]=0;
                //           FN_Cor[1024-len[1][1]-(len[1][1]==1024?0:1)]=0;
                //           fprintf_s(f, "%d\t%s\t%d\t%s\t%d\t%s\t%d\t%s\n",
                //                        nFP, FP, nFP_C, FP_Cor,
                //                        nFN, FN, nFN_C, FN_Cor);
                //         }
                //  }
                //else
                while(fread(&Dist, sizeof(Dist), 1, fn))
                     {
                       //Dist.HypSyn = !Syn2Found?0:
                       //              Syn2(Dist.Spotter, Dist.Marker[0]);
                       WriteDistFileRecord(Dist, f);
                       //i++;
                     }
                //j+=i;
                fclose(fn);
                DeleteFile(Buf);
              }
         }
       for(i=0; i<FREQSIZE; i++, start+=step)
           fprintf_s(f, "%6.3f\t%i\t%i\n", start,
                     LOWLONG(DistFreq_1F[i]),
                     HIGHLONG(DistFreq_1F[i]));
       fclose(f);
       //system("del Dist1_*.bin >nul 2>&1");
       return SaveSyn2_Score_HeatMap();
     }
  bool Calc_2Factor(char *DistFname, void Progress()=0)
     {
       int i, j, k, nElements;

#ifdef _DEBUG
//nThreads=1;
#endif
       if(Syn2Only)
          SaveResults=false;
       if(!Syn2Found && !Syn2Only)
          LoadSyn2();
       CalcCandidateSpotters();
       CndH_2F = NewTriangularMatrix<REAL>(nFactors);
       nElements=TriangularMatrixSize(nFactors);
       HeatMaps=NewVector<HeatMap>(nThreads);
       Calc_nParts(nElements);
       OffsetsTable = NewMatrix<int>(nParts,4);
       ThreadsPtr = OffsetsTable;
       ThreadsEndPtr = ThreadsPtr + nParts;
       DivideTriangularMatrix(nFactors, nParts, OffsetsTable);
       for(i=0; i<nThreads; i++)
          {
            ThreadParams[i].predictors[0][0].nD=
            ThreadParams[i].predictors[0][1].nD=
            ThreadParams[i].predictors[1][0].nD=
            ThreadParams[i].predictors[1][1].nD=2;
            ThreadParams[i].Index=i;
            ThreadParams[i].ds=this;
            ThreadParams[i].nD=2;
            ThreadParams[i].CallBack=&Dataset::Calc_2Factor;
            ThreadParams[i].hm=HeatMaps+i;
            ThreadParams[i].Progress=Progress;
            ThreadHandles[i]=(HANDLE)_beginthreadex(NULL, 0,
                                (ThreadAPI)Thread, ThreadParams+i,
                                                     0, ThreadIds+i); 
            if(ThreadHandles[i]==NULL)
              {
                DeleteMatrix((void**)OffsetsTable, nParts);
                DeleteVector(HeatMaps);
                while(--i>=0)
                      CloseHandle(ThreadHandles[i]);
                return false;
              }
          }
       WaitForMultipleObjects(nThreads, ThreadHandles, TRUE, INFINITE);
       for(i=0; i<nThreads; i++)
           CloseHandle(ThreadHandles[i]);
       if(ThreadsPtr<ThreadsEndPtr) // aborted
         {
           //EnterCriticalSection(&critSecAbort);
           AbortFlag=false;
           //LeaveCriticalSection(&critSecAbort);
           DeleteMatrix((void**)OffsetsTable, nParts);
           DeleteVector(HeatMaps);
           return false;
         }
       //if(!EvaluateCorrectionRates)
       //  {
       for(i=0; i<FREQSIZE*3; i++)
          {
            SynFreq_2F[i] = ThreadParams[0].IFreq[i];
            for(j=1; j<nThreads; j++)
                SynFreq_2F[i]+=ThreadParams[j].IFreq[i];
          }
       for(i=0; i<FREQSIZE; i++)
          {
            DistFreq_2F[i] = ThreadParams[0].DistFreq[i];
            for(j=1; j<nThreads; j++)
                DistFreq_2F[i]+=ThreadParams[j].DistFreq[i];
          }
       for(i=0; i<HEATMAPHEIGHT; i++)
       for(j=0; j<HEATMAPWIDTH; j++)
          {
            Syn2_Score_HeatMap[i][j]=ThreadParams[0].hm->hm[i][j];
            for(k=1; k<nThreads; k++)
                Syn2_Score_HeatMap[i][j]+=ThreadParams[k].hm->hm[i][j];
          }
       //if(Fname!=NULL)
       if(!Syn2Found||Syn2Only)
          SaveSyn2();
       if(SaveResults&&!Syn2Only)
          SaveDist_2Factors(DistFname);
         //}//if(!EvaluateCorrectionRates)
       DeleteMatrix((void**)OffsetsTable, nParts);
       DeleteVector(HeatMaps);
       return true;
     }
  void Calc_2Factor(ThreadParam* param)
     {
       int i, j, k, nClusters, EndClustering=2, *Offsets, Marker[2], Echo,
                StartMeasure = 4; //(1<<nD)<<2;
       int PairNumber;
       REAL Step=(REAL)3.0/(CountOf(SynFreq_2F)), *Data[2], CndH,
            BestScore, limits[]={0,0};//TODO: calc limits for pairs
       Cluster Clusters[4];
       UPGMA upgma;
       FILE *f=0;
       char Buf[16];
       int LastPair = PairNo(nFactors-1, nFactors-2), Tip = LastPair/20;
       bool AbFlag;

       if(!Syn2Found||Syn2Only)
          for(i=0; i<CountOf(SynFreq_2F); i++)
              param->IFreq[i]=0;
       for(i=0; i<FREQSIZE; i++)
           param->DistFreq[i]=0;
       for(i=0; i<CountOf(param->SynCorrFreq); i++)
           param->SynCorrFreq[i]=0;
       for(i=0; i<HEATMAPHEIGHT; i++)
       for(j=0; j<HEATMAPWIDTH; j++)
           param->hm->hm[i][j]=0;
       if(!Testing && SaveResults && !FrequencyOnly)
         {
           sprintf_s(Buf, "Dist2_%d.bin", param->Index);
           if(   0!=fopen_s(&f, Buf, "wb")
              || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
             )
             {
               char Msg[128];
               sprintf_s(Msg, "Failed to open file %s", Buf);
               Exit(1, Msg);
             }
         }
       else f=NULL;
       param->f=f;
       param->DistStep = (REAL)1.0/(CountOf(DistFreq_2F));
       upgma.Init(nSamples, 2, Clusters,
                  StartMeasure, this, NULL, nClasses,
                  SampleClasses, Separation);
       do{
           EnterCriticalSection(&critSecAbort);
           AbFlag=AbortFlag;
           LeaveCriticalSection(&critSecAbort);
           if(AbFlag)
              break;
           EnterCriticalSection(&critSec);
           Offsets=GetMore();
           LeaveCriticalSection(&critSec);
           if(Offsets==NULL)
              break;
           i=Offsets[0];
           j=Offsets[1];
           PairNumber = PairNo(i, j);
           Data[0] = FactorData[i];
           do{
               bool CalcPair=Factor_PairList&&bsearch(&PairNumber,Pairs,nPairs,sizeof(int),Comp<int>)
                             ||PairNumber>=From && (To<0||PairNumber<=To);
               if(Syn2Only)
                  CalcPair=false;
               if(!Syn2Found || Syn2Only || CalcPair)
                 {
                   Data[1] = FactorData[j];
                   nClusters=upgma.DoClustering(EndClustering,
                                                BestScore, Data);
                   CndH = HC-BestScore;
                   if(Syn2Only||!Syn2Found)
                     {
                       CndH_2F[i][j] = CndH;
                       k=int((2+Syn2(i,j))/Step);
                       if(k<0)
                          k=0;
                       if(k>=CountOf(SynFreq_2F))
                          k=CountOf(SynFreq_2F)-1;
                       param->IFreq[k]++;
                     }
                   if(CalcPair)
                     {
                       Marker[0]=i;
                       Marker[1]=j;
                       param->Marker=Marker;
                       FindDistortions(param);
                     }
                 }
               if(++j==i)
                 {
                   j=0;
                   Data[0] = FactorData[++i];
                 }
               PairNumber++;
             } while(i<Offsets[2]||i==Offsets[2]&&j<=Offsets[3]);
           if(param->Progress)
             {
               EnterCriticalSection(&critSecProgress);
               param->Progress();
               LeaveCriticalSection(&critSecProgress);
             }
         } while(true);
       if(f!=NULL)
          fclose(f);
     }
  bool LoadSyn2()
     {
       int i;
       FILE *f;
       if(   0!=fopen_s(&f, SynBinFname, "rb")
          || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
         )
          return false;
       if(CndH_2F==NULL)
          CndH_2F = NewTriangularMatrix<REAL>(nFactors);
       for(i=1; i<nFactors; i++)
           fread(CndH_2F[i], sizeof(REAL), i, f);
       fread(SynFreq_2F, sizeof(int), CountOf(SynFreq_2F), f);
       fclose(f);
       return Syn2Found=true;
     }
  bool SaveSyn2()
     {
       int i, j;
       FILE *f;
       REAL step=(REAL)3.0/CountOf(SynFreq_2F), start=-2+step/2;

       if(   0!=fopen_s(&f, SynTxtFname, "wt")
          || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
         )
          return false;
       for(i=0; i<nFactors-1; i++)
           fprintf_s(f,"\t%s",FactorNames[i]);
       fprintf_s(f,"\n");
       for(i=1; i<nFactors; i++)
          {
            fprintf_s(f, FactorNames[i]);
            for(j=0; j<i; j++)
                fprintf_s(f, "\t%8.5f", Syn2(i,j));
            fprintf_s(f, "\n");
          }
       for(i=0; i<CountOf(SynFreq_2F); i++, start+=step)
           fprintf_s(f, "%6.3f\t%i\n", start,SynFreq_2F[i]);
       fclose(f);
       if(   0!=fopen_s(&f, SynBinFname, "wb")
          || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
         )
          return false;
       for(i=1; i<nFactors; i++)
           fwrite(CndH_2F[i], sizeof(REAL), i, f);
       fwrite(SynFreq_2F, sizeof(int), CountOf(SynFreq_2F), f);
       fclose(f);
       return Syn2Found=true;
     }
  bool CalcSyn2(void Progress()=0)
     {
       Syn2Only=true;
       Syn2Found=false;
       Calc_2Factor("", Progress);
       Syn2Only=false;
       return Syn2Found;
     }
  bool SaveDist_2Factors(char *DistFname)
     {
       int i;
       FILE *f, *fn;
       REAL step=(REAL)1.0/CountOf(DistFreq_2F), start=step/2;
       char Buf[16];
       DistortionInstance Dist;
  
       if(   0!=fopen_s(&f, DistFname, "wt")
          || 0!=setvbuf(f, 0, _IOFBF, 1024*1024)
         )
          return false;
       if(!FrequencyOnly)
         {
           int nSigDist=0,
               SigSize= sizeof(DistortionInstance)
                       -sizeof(int)*(1+MAX_MIS);
           WriteDistFileHeader(2, f);
           for(i=0; i<nThreads; i++)
              {
                sprintf_s(Buf, "Dist2_%d.bin", i);
                if(   0!=fopen_s(&fn, Buf, "rb")
                   || 0!=setvbuf(fn, 0, _IOFBF, 1024*1024)
                  )
                   return false;
                while(fread(&Dist, sizeof(DistortionInstance), 1, fn))
                      WriteDistFileRecord(Dist, f);
                fclose(fn);
                DeleteFile(Buf);
              }
         }
       for(i=0; i<CountOf(DistFreq_2F); i++, start+=step)
           fprintf_s(f, "%6.3f\t%i\t%i\n", start,
                     LOWLONG(DistFreq_2F[i]),
                     HIGHLONG(DistFreq_2F[i]));
       fclose(f);
       return SaveSyn2_Score_HeatMap();
     }
  void CopyClusterType_1F(ClusterType_1F &dest, ClusterType_1F &src)
     {
       for(int clust=0; clust<2; clust++)
          {
            dest.nSamples[clust]=src.nSamples[clust];
            for(int cls=0; cls<nClasses; cls++)
                dest.ClassCounts[clust][cls]=src.ClassCounts[clust][cls];
          }
     }
  void AdjustClusterClasses(ClusterType_1F &ClustersC)
     {
       for(int clust=0; clust<2; clust++)
        if(ClustersC.ClassCounts[clust][0]>ClustersC.ClassCounts[clust][1])
           ClustersC.Classes[clust]=0;
        else ClustersC.Classes[clust]=1;       
        if(ClustersC.Classes[0]==ClustersC.Classes[1])
           ClustersC.Classes[0]=1-ClustersC.Classes[0];
     }
  void CalcCorrection(int Marker, ClusterType_1F &ClustersC,
                      DistortionInstance &di
                     )
     {
       int i, clust, cls;
       ClusterType_1F &Clusters=FactorClusters[Marker];

       CopyClusterType_1F(ClustersC, Clusters);
       for(i=0; i<di.nMis; i++)
          {
            clust=ClustersMembership[(Marker*nSamples)+di.Mis[i]];
            cls=SampleClasses[di.Mis[i]];
            ClustersC.ClassCounts[clust][cls]--;
            ClustersC.nSamples[clust]--;
            ClustersC.ClassCounts[1-clust][cls]++;
            ClustersC.nSamples[1-clust]++;
          }
       //AdjustClusterClasses(ClustersC);
     }
  REAL Normalize(REAL Data, int Factor)
     {
       return (Data-FactorMeans[Factor])/FactorSDs[Factor];
     }
  REAL DeNormalize(REAL Data, int Factor)
     {
       return Data*FactorSDs[Factor] + FactorMeans[Factor];
     }
  void FindDistortions(ThreadParam *param)
     {
       int i, j, k, d, Cls, Cat, nMis[2], MinSpot[2],
           nD=param->nD,allMis[2][2],*Marker=param->Marker;
       BYTE &Direction=FactorClusters[*Marker].Direction;
       DistortionInstance di[2][2];
       SpotterType *ptr;
       long long *DistFreq=param->DistFreq,
                 *SynCorrFreq=param->SynCorrFreq;
       HeatMap *&heatMap=param->hm;
       BYTE DistortedSamples[2][2][MAX_SAMPLES],
           *ClustMembership=ClustersMembership+(*Marker)*nSamples,
            MisMap[2][MAX_SAMPLES];
       ClusterType_1F &Clusters=FactorClusters[*Marker],
                      ClustersC;
       REAL DistStep=param->DistStep, Range,
            InfSyn=nD==1? HC-CndH_1F[*Marker]
                        : Syn2(*Marker, Marker[1]);;
       WORD  Mis[2][MAX_SAMPLES];
       const REAL Epsilon=1e-10;
       REAL l[2];
       int nt[2];
       WORD *p, *q;
       bool NoDist;

       nMis[0] = nMis[1] = 0;
       for(i=0; i<nSamples; i++) // Mis[x]: class-x-samples mislabeled as class-y-samples
        if(SampleClasses[i]!=Clusters.Classes[ClustMembership[i]])
           Mis[SampleClasses[i]][nMis[SampleClasses[i]]++] = i;
       /*nMis[0]<MinMis*/
       /*nMis[1]<MinMis*/
       if(   nMis[0]>MaxMis && nMis[1]>MaxMis
          || nMis[0]+nMis[1] > MaxMis*MaxMisTotal
          || !Testing && nMis[0]<MinMis && nMis[1]<MinMis
         )
          return;
       MinSpot[0] = nMis[0]*MinSpottingRate;
       if(MinSpot[0]<MinSpotted)
          MinSpot[0]=MinSpotted;
       MinSpot[1] = nMis[1]*MinSpottingRate;
       if(MinSpot[1]<MinSpotted)
          MinSpot[1]=MinSpotted;
       //ZeroMem(Map, nSamples<<1);
       for(Cls=0; Cls<2; Cls++)
       for(i=0;i<nSamples;i++)
           MisMap[Cls][i]=0;
       for(Cls=0; Cls<2; Cls++)
       for(i=0;i<nMis[Cls];i++)
           MisMap[Cls][Mis[Cls][i]]=1;

       for(Cls=0; Cls<2; Cls++)
       for(Cat=0; Cat<2; Cat++)
          {
            di[Cls][Cat].Class=Cls;
            di[Cls][Cat].Category=Cat;
            di[Cls][Cat].Cohesion=0.0;
            di[Cls][Cat].nD = nD;
            for(i=0; i<nD; i++)
                di[Cls][Cat].Marker[i] = Marker[i];
            di[Cls][Cat].InfSyn = InfSyn;
            di[Cls][Cat].HypSyn = 0;
          }
       if(Testing)
         {
           if(Direction>0)
              Directions[*Marker]++;
           CalcROC_Steps(*Marker);
           //ZeroMem(DistortedSamples, sizeof(DistortedSamples));
           for(Cls=0;Cls<2;Cls++)
           for(Cat=0;Cat<2;Cat++)
           for(i=0;i<MAX_SAMPLES;i++)
               DistortedSamples[Cls][Cat][i]=0;
           for(Cls=0; Cls<nClasses; Cls++)
           for(Cat=0; Cat<nCategories; Cat++)
              {
                param->predictors[Cls][Cat].ReWind();
                param->predictors[Cls][Cat].Marker[0]=Marker[0];
                if(nD==2)
                   param->predictors[Cls][Cat].Marker[1]=Marker[1];
                param->predictors[Cls][Cat].Direction=Direction;
                allMis[Cls][Cat]=nMis[Cls];
                for(i=0; i<nMis[Cls]; i++)
                    DistortedSamples[Cls][Cat][Mis[Cls][i]]=1;
              }
         }
       if(nMis[0]>=MinMis||nMis[1]>=MinMis)
       for(d=0; d<nFactors; d++)
          {
            if(   d==*Marker   // Marker & Spotter don't mix
               || (nD==2&&d==Marker[1])
               || !CandidateSpottersMask[d]
              )
               continue;
            //if(   Testing
            //   && allMis[0][0]==0
            //   && allMis[0][1]==0
            //   && allMis[1][0]==0
            //   && allMis[1][1]==0
            //  )
            //   break;

            WORD *sIndex=SampleIndex+d*nSamples,
                 **Tail=Tails+(d<<1);
            int *nTail=nTails+(d<<1);// Low:[0,Tail[0]-1], Hi:[Tail[0],nSamples-1]
            for(Cls=0; Cls<nClasses; Cls++)
               {
                 if(nMis[Cls]>MaxMis||nMis[Cls]<MinMis)
                    continue;
                 //SortIndex(Mis[Cls],FactorData[d],nMis[Cls]);
                 for(Cat=0; Cat<nCategories; Cat++)
                    {
                      if(   Testing
                         &&!(CandidateSpottersMask[d]&(Cat+1))
                           //(   nTail[Cat]<MinMis
                           // || nTail[Cat]>MaxMis
                           // //|| allMis[Cls][Cat]==0
                           //)
                        )
                         continue;
                      int &nmis=di[Cls][Cat].nMis;
                      WORD *mis=di[Cls][Cat].Mis;
                      NoDist=false;
                      di[Cls][Cat].Spotter = d;
                      for(i=nmis=0;i<nTail[Cat];i++)//mis=Instersection(Tail[Cat],Mis[Cls])
                       if(MisMap[Cls][Tail[Cat][i]])
                          mis[nmis++]=Tail[Cat][i];
                      //IntersectionIndex(
                      //           Mis[Cls],nMis[Cls],
                      //           Tail[Cat], nTail[Cat],
                      //           mis, nmis,
                      //           FactorData[d]);
//char MisBuf[256],tBuf[256], misBuf[256], *px;
//for(i=0,j=sizeof(MisBuf),px=MisBuf; i<nMis[Cls]; i++,px+=5,j-=5)
//sprintf_s(px,j,"%3u%s",Mis[Cls][i],i<nMis[Cls]-1?", ":"\0");
//for(i=0,j=sizeof(tBuf),px=tBuf; i<nTail[Cat]; i++,px+=5,j-=5)
//sprintf_s(px,j,"%3u%s",Tail[Cat][i],i<nTail[Cat]-1?", ":"\0");
//for(i=0,j=sizeof(misBuf),px=misBuf; i<nmis; i++,px+=5,j-=5)
//sprintf_s(px,j,"%3u%s",mis[i],i<nmis?", ":"\0");

                      if(nmis<MinSpot[Cls])
                         continue;
                      if(!(Testing||Permuting))
                        {
                          if(nD==1)
                             di[Cls][Cat].HypSyn = Syn2Found? Syn2(*Marker, d):0;
                          else if(nD==2)
                             di[Cls][Cat].HypSyn = Syn3(*Marker,Marker[1],d);
                          else di[Cls][Cat].HypSyn = 0;//for future 
                          if(nmis>0)
                            {
                              CalcCorrection(*Marker, ClustersC, di[Cls][Cat]);
                              di[Cls][Cat].InfSynCor = HC-CondH(ClustersC);
                            }
                          else
                              di[Cls][Cat].InfSynCor = di[Cls][Cat].InfSyn;
                        }
                      p = mis;
                      q = p + nmis;
                      l[Cat] = FactorData[d][*p];
                      if(Cat==0)
                        {
                          for(p++; p<q; p++)//Calc Precision
                           if(l[Cat]<FactorData[d][*p])
                              l[Cat]=FactorData[d][*p];
                          //for(p=mis; p<q; p++)//Calc Cohesion
                          //    di[Cls][Cat].Cohesion*=(FactorData[d][*p]-
                          //                  FactorMin[d]+Epsilon)/Range;
                          //Range=FactorMeans[d]-FactorSDs[d]-FactorMin[d]+Epsilon;
                        }
                      else
                        {
                          for(p++; p<q; p++)//Calc Precision
                           if(l[Cat]>FactorData[d][*p])
                              l[Cat]=FactorData[d][*p];
                          //for(p=mis; p<q; p++)//Calc Cohesion
                          //    di[Cls][Cat].Cohesion*=(FactorMax[d]-
                          //                  FactorData[d][*p]+Epsilon)/Range;
                          //Range=FactorMax[d]-FactorMeans[d]-FactorSDs[d]+Epsilon;
                        }
                      CalcOneTail(d, sIndex, l, nt, true, Cat);
                      di[Cls][Cat].Precision = ((REAL)nmis)/nt[Cat];
//if(di[Cls][Cat].Precision>1||di[Cls][Cat].Precision<0)
//{
//int ttt=0;
//ttt=Cls;
//}
                      //if(Range<Epsilon)
                      //   Range=Epsilon;
                      //di[Cls][Cat].Cohesion=1-(FactorData[d][mis[nmis-1]]-FactorData[d][mis[0]])/Range;
                      //if(di[Cls][Cat].Cohesion>1.0)
                      //   di[Cls][Cat].Cohesion=1.0;
                      di[Cls][Cat].Score=di[Cls][Cat].Precision
                                        /**di[Cls][Cat].Cohesion*/;
                      if(   Testing
                         && (   di[Cls][Cat].Score<MinDistScore
                             || di[Cls][Cat].Precision<MinPrecision
                             //|| di[Cls][Cat].Cohesion<MinCohesion
                            )
                        )
                         continue;
                      if(Testing && allMis[Cls][Cat]>0)
                         for(i=0; i<nmis; i++)
                          if(DistortedSamples[Cls][Cat][mis[i]]==1)
                            {
                              DistortedSamples[Cls][Cat][mis[i]]=2;
                              allMis[Cls][Cat]--;
                            }
                      if(!Testing)
                        {
                          k = (int)(di[Cls][Cat].Score/DistStep);
                          if(k >= MAXFREQ)
                             k = MAXFREQ-1;
                          else if(k<0)
                             k = 0;
                          XLONG(DistFreq[k], Cls)++;
                         }
                      if(Permuting)
                         continue;
                      if(Testing)
                        {
                          Predictor &pr=param->predictors[Cls][Cat];
                          SpotterType md;
                          md.Spotter = d;
                          md.Percentile=l[Cat];
                          if(   InsertOrdered(md,
                                              pr.Spotters,
                                              pr.nSpotters, ptr,
                                              MAX_SPOTTERS
                                             )
                             && pr.nSpotters >MAX_SPOTTERS
                            )
                            {
                              char MsgBuf[512];
                              sprintf_s(MsgBuf, "Too many spotters!");
                              Exit(1, MsgBuf);
                            }
                          continue;
                        }// if(Testing)
                      i=(int)((di[Cls][Cat].HypSyn+2.0)/3.0*HEATMAPHEIGHT);
                      j=(int)(di[Cls][Cat].Score*HEATMAPWIDTH);
                      if(i<0) i=0;
                      if(i>=HEATMAPHEIGHT)
                         i=HEATMAPHEIGHT-1;
                      if(j<0) j=0;
                      if(j>=HEATMAPWIDTH)
                         j=HEATMAPWIDTH-1;
                      XLONG(heatMap->hm[i][j], Cls)++;
                      if(nD==1)
                        {
                          if(FactorICorr[*Marker][Cls]<di[Cls][Cat].InfSynCor)
                             FactorICorr[*Marker][Cls]=di[Cls][Cat].InfSynCor;
                        }
                      else if(nD==2)
                        {
                          j = CountOf(SynCorrFreq_2F);
                          k = (int)(di[Cls][Cat].InfSynCor/DistStep);
                          if(k >= j)
                             k = j-1;
                          else if(k<0)
                             k = 0;
                          XLONG(SynCorrFreq[k], Cls)++;
                        }
                      if(param->f!=NULL&&di[Cls][Cat].Score>=MinDistScore)
                         fwrite(&di[Cls][Cat], sizeof(DistortionInstance), 1, param->f);
                    } // for(Cat=0; Cat<nCategories; Cat++)
               } // for(Cls=0; Cls<nClasses; Cls++)
          } // for(d=0; d<nFactors; d++)
       if(Testing)
       for(Cls=0; Cls<2; Cls++)
       for(Cat=0; Cat<2; Cat++)
          {
            if(allMis[Cls][Cat]==0)//all mislabeled have been corrected
               di[Cls][Cat].InfSynCor=1.0;
            else if(allMis[Cls][Cat]==nMis[Cls])// no mislabeled was corrected
                    di[Cls][Cat].InfSynCor=di[Cls][Cat].InfSyn;
            else{
                  CopyClusterType_1F(ClustersC, Clusters);
                  for(i=0; i<nMis[Cls]; i++)
                   if(DistortedSamples[Cls][Cat][Mis[Cls][i]]==2)
                     {
                       BYTE &clust=ClustMembership[Mis[Cls][i]],
                            cls=SampleClasses[Mis[Cls][i]];
                       ClustersC.ClassCounts[clust][Cls]--;
                       ClustersC.nSamples[clust]--;
                       ClustersC.ClassCounts[1-clust][Cls]++;
                       ClustersC.nSamples[1-clust]++;
                     }
                  di[Cls][Cat].InfSynCor=HC-CondH(ClustersC);
                }
            bool Dist = di[Cls][Cat].InfSynCor>=MinICorr && param->predictors[Cls][Cat].nSpotters>0,
                 NoDist = Cls+Cat==0 && InfSyn>=MinI;
            if(Dist || NoDist)
              {
                DoTest(param->predictors[Cls][Cat], Dist, NoDist);
                MergePredictors(Predictors[((*Marker)<<2)+(Cls<<1)+Cat],
                              param->predictors[Cls][Cat], Dist, NoDist);
              }
          }//if(Testing||EvaluateCorrectionRates) // for(Cls... for(Cat
     }
  void MergePredictors(Predictor &md, Predictor &p, bool dist, bool nodist)
     {
       int i;
     
       //if(dist)
       //  {
       //    if(p.Counters[1].AUC-p.Counters[0].AUC<MinAUC_Gain)
       //      {
       //        dist=false;
       //        if(!nodist)
       //           return;
       //      }
       //  }
       if(md.Marker[0]<0) //1st time
         {
           md.Marker[0]=p.Marker[0];
           md.nD = p.nD;
           md.Class=p.Class;
           md.Category=p.Category;
           md.Direction=p.Direction;
         }
       if(dist)
         {
           md.Counters[0].Merge(p.Counters[0], nROC_Steps, p.Class);
           md.Counters[1].Merge(p.Counters[1], nROC_Steps, p.Class);
           md.Counters[1].PRate++;
         }
       if(nodist)
         {
           md.Counters[2].Merge(p.Counters[2], nROC_Steps);
           md.Counters[2].PRate++;
         }
       if(p.nSpotters>0)
         {
           if(   NULL==md.Spotters
              && NULL==(md.Spotters=NewVector<SpotterType>(MAX_SPOTTERS))
             )
              Exit(1, "Failed to allocate memory for Spotters");
           if(md.nSpotters==0)
             {
               md.nSpotters=p.nSpotters;
               for(i=0; i<p.nSpotters; i++)
                   md.Spotters[i]=p.Spotters[i];
             }
           else
             {
               SpotterType *mdp;
               for(i=0; i<p.nSpotters; i++)
                if(!InsertOrdered<SpotterType>(p.Spotters[i],
                                                 md.Spotters, md.nSpotters,
                                                 mdp, MAX_SPOTTERS)
                  )
                  {
                    if(      p.Category==1
                          && mdp->Percentile>p.Spotters[i].Percentile
                       ||    p.Category==0
                          && mdp->Percentile<p.Spotters[i].Percentile
                      )
                        mdp->Percentile = p.Spotters[i].Percentile;
                  }
                else if(md.nSpotters>MAX_SPOTTERS)
                        Exit(1, "Too many spotters!");
             }
         }
     }
  REAL Syn2(int i, int j)
     {
       if(i<j)
         {
           int k=i;
           i=j;
           j=k;
         }
       //REAL I2=HC-CndH_2F[i][j], Ii=HC-CndH_1F[i], Ij=HC-CndH_1F[j],
       //     Syn=I2-Ii-Ij, Syn_=CndH_1F[i]+CndH_1F[j]-CndH_2F[i][j]-HC;
//#ifdef _DEBUG
//REAL syn2=CndH_1F[i]+CndH_1F[j]-CndH_2F[i][j]-HC;
//if(syn2<-2||syn2>1)
//{
//int ii;
//ii=i;
//}
//#endif
       return CndH_1F[i]+CndH_1F[j]-CndH_2F[i][j]-HC;
     }
  void CountFrequencies(long long *DistFreq, int *nDist)
     {
       int i, j;

       for(j=1; j<3; j++)
          {
            nDist[j]=XLONG(DistFreq[0], j-1);
            for(i=1; i<MAXFREQ; i++)
                nDist[j]+=XLONG(DistFreq[i], j-1);
          }
       nDist[0]=nDist[1]+nDist[2];
     }
  void CalcCummulativeFreq(long long *DistFreq, REAL **CumulativeFreq, int *nDist)
     {
       int i, j, k;
       REAL x;
       
       CountFrequencies(DistFreq, nDist);
       for(k=1; k<3; k++)
        if(nDist[k]>0)
           for(i=MAXFREQ-1, x=0.0; i>=0; i--)
              {
                CumulativeFreq[k][i]=XLONG(DistFreq[i],k-1)+x;
                x=CumulativeFreq[k][i];
              }
        else
           for(i=0; i<MAXFREQ; i++)
              {
                CumulativeFreq[k][i]=0;
              }
       for(i=0; i<MAXFREQ; i++)
           CumulativeFreq[0][i]=CumulativeFreq[1][i]+CumulativeFreq[2][i];
       for(k=0; k<3; k++)
        if(nDist[k]>0)
           for(i=0; i<MAXFREQ; i++)
               CumulativeFreq[k][i]/=nDist[k];//cumFreq normalized
     }
  void Permute()
     {
       int i, j, k, Index[MAX_SAMPLES];

       for(i=0; i<nSamples; i++)
          {
            SampleClasses[i]=0;
            Index[i]=i;
          }
       k = ClassCounts[1];
       for(i=0; i<k; i++)
          {
            j=i+(int)((((REAL)rand())*(nSamples-1-i))/RAND_MAX+0.5);
            SampleClasses[Index[j]] = 1;
            //Swap<int>(Index[i], Index[j]);
            Index[j] = Index[i];
          }
     }
  bool PermutationTesting(void Progress())
     {
       int i, j, k, nDist[3], Counts[3][MAXFREQ+1], nD=1;
       REAL *CumFrqActualIndx[3]={CumulativeFreqActual[0],
                                  CumulativeFreqActual[1],
                                  CumulativeFreqActual[2]
                                 },
            *CumFrqPermutedIndx[3]={CumulativeFreqPermuted[0],
                                    CumulativeFreqPermuted[1],
                                    CumulativeFreqPermuted[2]
                                   }, x;
       bool srold=SaveResults, fonly=FrequencyOnly;
       BYTE SampleClassesTemp[MAX_SAMPLES];
       long long *DistFreq=nD==1?DistFreq_1F:DistFreq_2F;

       SaveResults = false;
       MinPrecision=MinCohesion=MinDistScore=0;
       Permuting=FrequencyOnly=true;
       if(!Calc_1Factor())
         {
           SaveResults = srold;
           FrequencyOnly = fonly;
           return false;
         }
       memcpy(SampleClassesTemp, SampleClasses, nSamples);
       CalcCummulativeFreq(DistFreq, CumFrqActualIndx, nDist);
       Progress();

       srand(Randomize?(unsigned)time(0):0);
       //ZeroMem(Counts, sizeof(Counts));
       for(k=0; k<3; k++)
       for(i=0; i<MAXFREQ; i++)
          {
            Counts[k][i]=0;
            DistFreqAve[k][i]=0.0;
            MinCumFreqPerm[k][i]=1.0;
            MaxCumFreqPerm[k][i]=0.0;
          }
       Counts[0][MAXFREQ]=Counts[1][MAXFREQ]=Counts[2][MAXFREQ]=0;
       for(i=0; i<nShuffles; i++)
          {
            Permute();
            if(!Calc_1Factor() && i<nShuffles-1)
               break;
            CalcCummulativeFreq(DistFreq, CumFrqPermutedIndx, nDist);
            for(k=0; k<3; k++)
            for(j=MAXFREQ-1; j>=0; j--)
               {
                 REAL &CumFreqPerm=CumulativeFreqPermuted[k][j];

                 if(   CumFreqPerm>0.0
                    && CumFreqPerm>=CumulativeFreqActual[k][j]
                   )
                    Counts[k][j]++;
                 DistFreqAve[k][j]+=CumFreqPerm;
                 if(MinCumFreqPerm[k][j]>CumFreqPerm)
                    MinCumFreqPerm[k][j]=CumFreqPerm;
                 if(MaxCumFreqPerm[k][j]<CumFreqPerm)
                    MaxCumFreqPerm[k][j]=CumFreqPerm;
               }
            Progress();
          }
       memcpy(SampleClasses, SampleClassesTemp, nSamples);
       SaveResults = srold;
       FrequencyOnly = fonly;
       if(i<nShuffles) // aborted
          return AbortFlag=false;
       for(k=0; k<3; k++)
       for(j=MAXFREQ-1; j>=0; j--)
          {
            DistFreqAve[k][j]/=nShuffles;
            if(Counts[k][j] < Counts[k][j+1])
               Counts[k][j] = Counts[k][j+1];
            pValues[k][j] = (REAL)Counts[k][j]/nShuffles;
          }
       return true;
     }
  bool SavePermutationTesting()
     {
       int i, j;
       FILE *f;

       if(0!=fopen_s(&f, PermutationsFname, "wt"))
          return false;
       fprintf_s(f, "Actual-FP\tMinPerm-FP\tPermuted-FP\tMaxPerm-FP\tp-value-FP\tActual-FN\tMinPerm-FN\tPermuted-FN\tMaxPerm-FN\tp-value-FN\tActual-Both\tMinPerm-Both\tPermuted-Both\tMaxPerm-Both\tp-value-Both\n");
       for(j=0; j<MAXFREQ; j++)
           fprintf_s(f, "%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\t%5.3lf\n",
                     CumulativeFreqActual[1][j],
                     MinCumFreqPerm[1][j],
                     DistFreqAve[1][j],
                     MaxCumFreqPerm[1][j],
                     pValues[1][j],
                     CumulativeFreqActual[2][j],
                     MinCumFreqPerm[2][j],
                     DistFreqAve[2][j],
                     MaxCumFreqPerm[2][j],
                     pValues[2][j],
                     CumulativeFreqActual[0][j],
                     MinCumFreqPerm[0][j],
                     DistFreqAve[0][j],
                     MaxCumFreqPerm[0][j],
                     pValues[0][j]
                    );
       fclose(f);
       return true;
     }
  bool LoadPermutationTesting()
     {
       int i, j;
       FILE *f;

       if(0!=fopen_s(&f, PermutationsFname, "rt"))
          return false;
       if(!GetEOL(f)) // header
         {
           fclose(f);
           return false;
         }
       for(i=0; i<MAXFREQ; i++)
        if(   !GetREAL(f, CumulativeFreqActual[1][i])
           || !GetREAL(f, MinCumFreqPerm[1][i])
           || !GetREAL(f, DistFreqAve[1][i])
           || !GetREAL(f, MaxCumFreqPerm[1][i])
           || !GetREAL(f, pValues[1][i])
           || !GetREAL(f, CumulativeFreqActual[2][i])
           || !GetREAL(f, MinCumFreqPerm[2][i])
           || !GetREAL(f, DistFreqAve[2][i])
           || !GetREAL(f, MaxCumFreqPerm[2][i])
           || !GetREAL(f, pValues[2][i])
           || !GetREAL(f, CumulativeFreqActual[0][i])
           || !GetREAL(f, MinCumFreqPerm[0][i])
           || !GetREAL(f, DistFreqAve[0][i])
           || !GetREAL(f, MaxCumFreqPerm[0][i])
           || !GetREAL(f, pValues[0][i])
          )
            break;
       fclose(f);
       return i==MAXFREQ;
     }
  void ReadFactorMask(char *FactorMaskFname)
     {
       char buf[256];
       int index, c=0;
       FILE *fMask;

       if(NULL!=fopen_s(&fMask, FactorMaskFname, "rt"))
         {
           printf_s("\nFailed to open file: %s", FactorMaskFname);
           getchar();
           exit(1);
         }
       if(FactorMask==NULL)
          FactorMask=NewVector<BYTE>(nFactors+3);
       int nf=(nFactors+3)>>2, *fm=(int*)FactorMask, *fme=fm+nf;
       char *el;
       while(fm<fme)
            *fm++=0;
       while(fgets(buf, 255, fMask))
            {
              for(el=buf; *el!=0 && *el!='\n' && *el!='\r'; el++)
                 {}
              *el=0;
              if(0!=FindIndexStri(buf, FactorNameIndex, FactorNames,
                                  nFactors, index))
                {
                  char MsgBuf[512];
                  sprintf_s(MsgBuf, "\nUnknown Factor Name: %s", buf);
                  Exit(1, MsgBuf);
                }
              FactorMask[FactorNameIndex[index]]=1;
              c++;
            }
       fclose(fMask);
       if(c==0)
         {
           Exit(1, "\nEmpty Factor List");
         }
       nMask = c;
     }
  inline int Count(int *C){return C[0]+C[1]+C[2]+C[3];}
  void CalcCombined()
     {
       int i, j, f;
       //int ContxtDist[2]={1,0}, ContxtNoDist[2]={1,1};

       nTopPredictorsNoDist=0;
       CombinedNoDist.Reset();
       //SortRef<Predictor>(pPredictorsNoDist, nPredictorsNoDist,
       //                   ContxtNoDist, CompPredictorRef);
       for(i=0; i<nPredictorsNoDist; i++)
          {
            if(pPredictorsNoDist[i]->Counters[2].AUC<TopAUC_NoDist)
               break;
            if(pPredictorsNoDist[i]->Counters[2].PredictionRate<TopPredictionRate_NoDist)
               continue;
            nTopPredictorsNoDist++;
            CombinedNoDist.Merge(pPredictorsNoDist[i]->Counters[2], nROC_Steps);
          }
       CombinedNoDist.CalcPerformance(nROC_Steps);

       //SortRef<Predictor>(pPredictorsDist, nPredictorsDist,
       //                   ContxtDist, CompPredictorRef);
       REAL MinAllAUC=Min(TopAUC_Dist[1],TopAUC_Dist[2]),
            MinAllPR=Min(TopPredictionRate_Dist[1],TopPredictionRate_Dist[2]);
       for(f=0; f<3; f++)
          {
            nTopPredictorsDist[f]=0;
            CombinedDist[f][0].Reset();
            CombinedDist[f][1].Reset();
          }
       
       for(i=0; i<nPredictorsDist; i++)
          {
            if(pPredictorsDist[i]->Counters[1].AUC<MinAllAUC)
               break;
            if(pPredictorsDist[i]->Counters[1].PredictionRate<MinAllPR)
               continue;
            f=pPredictorsDist[i]->Class+1;
            if(   pPredictorsDist[i]->Counters[1].AUC>=TopAUC_Dist[f]
               && pPredictorsDist[i]->Counters[1].PredictionRate>=TopPredictionRate_Dist[f]
              )
              {
                nTopPredictorsDist[f]++;
                CombinedDist[f][0].Merge(pPredictorsDist[i]->Counters[0], nROC_Steps, pPredictorsDist[i]->Class);
                CombinedDist[f][1].Merge(pPredictorsDist[i]->Counters[1], nROC_Steps, pPredictorsDist[i]->Class);
                CombinedDist[0][0].Merge(pPredictorsDist[i]->Counters[0], nROC_Steps, pPredictorsDist[i]->Class);
                CombinedDist[0][1].Merge(pPredictorsDist[i]->Counters[1], nROC_Steps, pPredictorsDist[i]->Class);
              }
          }
       nTopPredictorsDist[0]=nTopPredictorsDist[1]+nTopPredictorsDist[2];
       for(f=0; f<3; f++)
          {
            CombinedDist[f][0].CalcPerformance(nROC_Steps);
            CombinedDist[f][1].CalcPerformance(nROC_Steps);
          }
     }
  void AllocateTesting()
     {
       int i, j;

       if(Predictors==NULL)
         {
           if(NULL==(Predictors=NewVector<Predictor>(nFactors<<2)))
              Exit(1, "AllocateTesting(): Failed to Allocate Memory for Predictors");
         }
       else for(i=0; i<nFactors<<2; i++)
                Predictors[i].Reset(false);//false: keep Sporrers allocated
       if(ROC_Steps==NULL)
         {
           if(NULL==(ROC_Steps=NewMatrixZero<REAL>(nFactors, MAXROCSTEPS)))
              Exit(1, "AllocateTesting(): Failed to Allocate Memory for ROC_Steps");
         }
       else for(i=0; i<nFactors; i++)
                //ZeroMem(ROC_Steps[i], (MAXROCSTEPS)*sizeof(REAL));
            for(j=0;j<MAXROCSTEPS;j++)
                ROC_Steps[i][j]=0.0;
       if(Directions==NULL)
         {
           if(NULL==(Directions=NewVectorZero<int>(nFactors)))
              Exit(1, "AllocateTesting(): Failed to Allocate Memory for Directions");
         }
       else //ZeroMem(Directions, nFactors*sizeof(*Directions));
            for(i=0; i<nFactors; i++)
                Directions[i]=0;
       if(MidPoints==NULL)
         {
           if(NULL==(MidPoints=NewVectorZero<REAL>(nFactors)))
              Exit(1, "AllocateTesting(): Failed to Allocate Memory for MidPoints");
         }
       else //ZeroMem(MidPoints, nFactors*sizeof(REAL));
            for(i=0; i<nFactors; i++)
                MidPoints[i]=0;
     }
  void DeAllocateTesting()
     {
       DeleteVector(Predictors);
       Predictors=NULL;
       DeleteMatrix(ROC_Steps, nFactors);
       ROC_Steps=NULL;
       DeleteVector(Directions);
       Directions=NULL;
       DeleteVector(MidPoints);
       MidPoints=NULL;
       DeleteVector(pPredictorsDist);
       DeleteVector(pPredictorsNoDist);
       pPredictorsDist=pPredictorsNoDist=NULL;
     }
  void Shuffle(int nc0, int nc1)
     {
       int Index[MAX_SAMPLES];
       BYTE ClassBuf[MAX_SAMPLES], *cb=ClassBuf;
       REAL DataBuf[MAX_SAMPLES], *db=DataBuf;
       int i, j, cc0=ClassCounts[0], cc1=ClassCounts[1],
           *p0=Index, *p1=Index+cc0, *q;

       //if(nc0>cc0||nc1>cc1||(nc0==cc0&&nc1==cc1))
       //  {
       //    Exit(1,"Error: test sample counts.");
       //  }
       for(i=0; i<nSamples; i++)
        if(SampleClasses[i])
             *p1++ = i;
        else *p0++ = i;
       if(nc0<cc0)
          for(i=0; i<nc0; i++)
             {
               j=i+(int)((((REAL)rand())*(cc0-1-i))/RAND_MAX+0.5);
               Swap<int>(Index[i], Index[j]);
             }
       p0=Index+cc0;
       for(i=0; i<nc1; i++)
          {
            j=i+(int)((((REAL)rand())*(cc1-1-i))/RAND_MAX+0.5);
            Swap<int>(p0[i], p0[j]);
          }
       p0 = Index+nc0;
       q  = Index+cc0;
       p1 = Index+cc0+nc1;
       while(p0<q)
             Swap<int>(*p0++, *--p1);
       for(i=0; i<nSamples; i++)
           *cb++ = SampleClasses[Index[i]];
       for(i=0, cb-=nSamples; i<nSamples; i++)
           SampleClasses[i] = *cb++;
       for(j=0; j<nFactors; j++)
          {
            for(i=0; i<nSamples; i++)
                *db++ = FactorData[j][Index[i]];
            for(i=0, db-=nSamples; i<nSamples; i++)
                FactorData[j][i] = *db++;
            db-=nSamples;
          }
//#ifdef _DEBUG
//printf_s("\n");
//for(i=0; i<nSamples; i++)
//printf_s("\t%s",SampleNames[i]);
//printf_s("\n");
//for(i=0; i<nSamples; i++)
//printf_s("\t%s",ClassNames[SampleClasses[i]]);
//printf_s("\n");
//for(j=0; j<nFactors&&j<10; j++)
//{
//printf_s("\n%s",FactorNames[j]);
//for(i=0; i<nSamples; i++)
//printf_s("\t%7.3f",FactorData[j][i]);
//}
//#endif
     }
  bool Test(int nc0, int nc1, void Progress())
     {
       int i, j;
       bool SaveResultsOld;
       WORD TestClassCounts[MAX_CLASSES];
       BYTE SampleClassesTmp[MAX_SAMPLES];

       for(i=0; i<nSamples; i++)
           SampleClassesTmp[i]=SampleClasses[i];
       srand(Randomize?(unsigned)time(0):0);
       FrequencyOnly = true;
       nSamplesTest=nSamples;
       memcpy(TestClassCounts, ClassCounts, sizeof(TestClassCounts));
       SaveResultsOld = SaveResults;
       SaveResults = false;
#ifdef TRAIN_ALL
nc0=ClassCounts[0];
nc1=ClassCounts[1];
#endif
//nThreads=1;
       for(i=0; i<nShuffles; i++)
          {
            Shuffle(nc0, nc1); // Commented in CDS
            nSamples = nc0 + nc1;
            CalculateIndices();
            //ClassCounts[0]=nc0;
            //ClassCounts[1]=nc1;
            //CalcMeansSDs();
            bool abrt=!Calc_1Factor();
            nSamples=nSamplesTest;
            memcpy(ClassCounts, TestClassCounts, sizeof(ClassCounts));
            //Calc_2Factor("", "", "");
            Progress();
            if(abrt)
               break;
          }
       SaveResults = SaveResultsOld;
       for(j=0; j<nSamples; j++)
           SampleClasses[j]=SampleClassesTmp[j];
       ReloadFactorData();
       if(i<nShuffles) // aborted
         {
           EnterCriticalSection(&critSecAbort);
           AbortFlag=false;
           LeaveCriticalSection(&critSecAbort);
           return false;
         }
       return true;
     }
  void FillDiagonal(Predictor &pr)
     {
       int i, j, ct, cf;

       if(pr.Class==0)// FP
         {
           ct=0; cf=3;
           for(i=0, j=nROC_Steps; i<nROC_Steps; i++)
              {
                pr.Counters[0].Counters[i][ct]=
                pr.Counters[1].Counters[i][ct]=--j;
                pr.Counters[0].Counters[i][cf]=
                pr.Counters[1].Counters[i][cf]=i;
              }
         }
       else // FN
         {
           ct=1; cf=2;
           for(i=0, j=nROC_Steps; i<nROC_Steps; i++)
              {
                pr.Counters[0].Counters[i][ct]=
                pr.Counters[1].Counters[i][ct]=--j;
                pr.Counters[0].Counters[i][cf]=
                pr.Counters[1].Counters[i][cf]=i;
              }
         }

     }
  bool PredictionTesting(void Progress())
     {
       int i, j, k, //MinCount,
           nc0=(int)(0.5+ClassCounts[0]*0.9),// 90% training samples
           nc1=(int)(0.5+ClassCounts[1]*0.9),// 10% test samples
           ContxtDist[2]={1,0}, ContxtNoDist[2]={1,1};

       if(nSamples<6)
          Exit(1, "nSamples too few samples!");
       Testing = true;
       nc0 = Min(nc0, nc1);
       if(nc0<2) nc0=2;
       nc1 = nc0;
//nThreads=1;
//nShuffles=2;
       //MinCount=(int)(MinPredictionRate*nShuffles);
       AllocateTesting();
       if(!Test(nc0, nc1, Progress))
          return Testing = false;

       int nAll=nFactors<<2;
       for(i=nPredictorsDist=nPredictorsNoDist=0; i<nAll; i++)
        if(Predictors[i].Marker[0]>=0)
          {
            Predictors[i].Average(nShuffles, this);
            if(Predictors[i].Counters[1].PRate>0)
               FillDiagonal(Predictors[i]);
            Predictors[i].CalcPerformance(nROC_Steps);
            if(   Predictors[i].nSpotters>0
               && Predictors[i].Counters[1].PredictionRate>=MinPredictionRate
               && Predictors[i].Counters[1].AUC>=MinAUC
               && Predictors[i].Counters[1].AUC-
                  Predictors[i].Counters[0].AUC>=MinAUC_Gain
              )
               nPredictorsDist++;
            if((i&3)==0// Cls+Cat==0
               && Predictors[i].Counters[2].PredictionRate>=MinPredictionRate
               && Predictors[i].Counters[2].AUC>=MinAUC
              )
               nPredictorsNoDist++;
          }
       DeleteVector(pPredictorsDist);
       DeleteVector(pPredictorsNoDist);
       pPredictorsDist=pPredictorsNoDist=NULL;
       if(      nPredictorsDist>0
             && NULL==(pPredictorsDist=NewVectorZero<Predictor*>(nPredictorsDist))
          ||    nPredictorsNoDist>0
             && NULL==(pPredictorsNoDist=NewVectorZero<Predictor*>(nPredictorsNoDist))
         )
           Exit(1, "Failed to allocate memory for pPredictors");
       for(i=j=k=0; j<nPredictorsDist||k<nPredictorsNoDist; i++)
        if(Predictors[i].Marker[0]>=0)
          {// DistBefore:0   DistAfter:1   NoDist:2
            if(   Predictors[i].nSpotters>0
               && Predictors[i].Counters[1].PredictionRate>=MinPredictionRate
               && Predictors[i].Counters[1].AUC>=MinAUC
               && Predictors[i].Counters[1].AUC-
                  Predictors[i].Counters[0].AUC>=MinAUC_Gain
              )
               pPredictorsDist[j++]=Predictors+i;
            if((i&3)==0// Cls+Cat==0
               && Predictors[i].Counters[2].PredictionRate>=MinPredictionRate
               && Predictors[i].Counters[2].AUC>=MinAUC
              )
               pPredictorsNoDist[k++]=Predictors+i;
          }
       SortRef<Predictor>(pPredictorsDist, nPredictorsDist,
                          ContxtDist, CompPredictorRef);
       SortRef<Predictor>(pPredictorsNoDist, nPredictorsNoDist,
                          ContxtNoDist, CompPredictorRef);
       CalcCombined();
       Testing = false;
       return true;
     }

  bool SavePredictionTesting(char *predictionTestingDistFname,
                             char *predictionTestingNoDistFname)
     {
       int i, j, k;
       FILE *fDist=0, *fNoDist=0, *fbD=0, *fbND=0;
       int cs=sizeof(Predictors->Marker)+
              sizeof(Predictors->nD)+
              sizeof(Predictors->Class)+
              sizeof(Predictors->Category)+
              sizeof(Predictors->Direction)+
              sizeof(Predictors->nSpotters);

       if(   NULL!=fopen_s(&fDist, predictionTestingDistFname, "wt")
          || 0!=setvbuf(fDist, 0, _IOFBF, 1024*1024)
          || NULL!=fopen_s(&fNoDist, predictionTestingNoDistFname, "wt")
          || 0!=setvbuf(fNoDist, 0, _IOFBF, 1024*1024)
          || NULL!=fopen_s(&fbD, PredictorsDistFName, "wb")
          || 0!=setvbuf(fbD, 0, _IOFBF, 1024*1024)
          || NULL!=fopen_s(&fbND, PredictorsNoDistFName, "wb")
          || 0!=setvbuf(fbND, 0, _IOFBF, 1024*1024)
         )
         {
           char MsgBuf[512];
           sprintf_s(MsgBuf, "Failed to open file: %s\n",
                     fDist==NULL
                         ?predictionTestingDistFname
                         :(fNoDist==NULL?predictionTestingNoDistFname
                                        :(fbD==NULL?PredictorsDistFName
                                                   :PredictorsNoDistFName)));
           Exit(1, MsgBuf);
           return false;
         }
       WritePredictionTestingDistHeader(fDist);
       WritePredictionTestingNoDistHeader(fNoDist);
       for(i=0; i<nPredictorsDist; i++)
        if(   pPredictorsDist[i]->Counters[1].AUC>=TopAUC_Dist[pPredictorsDist[i]->Class+1]
           && pPredictorsDist[i]->Counters[1].PredictionRate>=TopPredictionRate_Dist[pPredictorsDist[i]->Class+1]
          )
           WritePredictionTestingDistRec(fDist, pPredictorsDist[i]);
       for(i=0; i<nPredictorsDist; i++)
        if(   pPredictorsDist[i]->Counters[1].AUC<TopAUC_Dist[pPredictorsDist[i]->Class+1]
           || pPredictorsDist[i]->Counters[1].PredictionRate<TopPredictionRate_Dist[pPredictorsDist[i]->Class+1]
          )
           WritePredictionTestingDistRec(fDist, pPredictorsDist[i]);
       for(i=0; i<nPredictorsNoDist; i++)
        if(   pPredictorsNoDist[i]->Counters[2].AUC>=TopAUC_NoDist
           && pPredictorsNoDist[i]->Counters[2].PredictionRate>=TopPredictionRate_NoDist
          )
            WritePredictionTestingNoDistRec(fNoDist, pPredictorsNoDist[i]);
       for(i=0; i<nPredictorsNoDist; i++)
        if(   pPredictorsNoDist[i]->Counters[2].AUC<TopAUC_NoDist
           || pPredictorsNoDist[i]->Counters[2].PredictionRate<TopPredictionRate_NoDist
          )
            WritePredictionTestingNoDistRec(fNoDist, pPredictorsNoDist[i]);
       fclose(fDist);
       fclose(fNoDist);

       if(!fwrite(&nPredictorsDist, sizeof(nPredictorsDist), 1, fbD))
          Exit(1, "Failed to write to file PredictorsDist.bin (1)");
       for(i=0; i<nPredictorsDist; i++)
          {
            j=(pPredictorsDist[i]->Marker[0]<<2)+(pPredictorsDist[i]->Class<<1)+pPredictorsDist[i]->Category;
            if(!fwrite(&j, sizeof(j), 1, fbD))
               Exit(1, "Failed to write to file PredictorsDist.bin (2)");
            if(!fwrite(pPredictorsDist[i]->Marker, cs, 1, fbD))
               Exit(1, "Failed to write to file PredictorsDist.bin (3)");
            if(!fwrite(&FactorClusters[pPredictorsDist[i]->Marker[0]].MidPoint, sizeof(REAL), 1, fbD))
               Exit(1, "Failed to write to file PredictorsDist.bin (4)");
            k=pPredictorsDist[i]->nSpotters*sizeof(SpotterType);
            if(!fwrite(pPredictorsDist[i]->Spotters, k, 1, fbD))
               Exit(1, "Failed to write to file PredictorsDist.bin (5)");
            if(!fwrite(pPredictorsDist[i]->Counters, sizeof(CounterType)<<1, 1, fbD))
               Exit(1, "Failed to write to file PredictorsDist.bin (6)");
          }
       fclose(fbD);

       if(!fwrite(&nPredictorsNoDist, sizeof(nPredictorsNoDist), 1, fbND))
          Exit(1, "Failed to write to file PredictorsNoDist.bin");
       for(i=0; i<nPredictorsNoDist; i++)
          {
            j=(pPredictorsNoDist[i]->Marker[0]<<2)+(pPredictorsNoDist[i]->Class<<1)+pPredictorsNoDist[i]->Category;
            if(!fwrite(&j, sizeof(j), 1, fbND))
               Exit(1, "Failed to write to file PredictorsDist.bin");
            if(!fwrite(pPredictorsNoDist[i]->Marker, sizeof(pPredictorsNoDist[i]->Marker), 1, fbND))
               Exit(1, "Failed to write to file PredictorsNoDist.bin");
            if(!fwrite(&pPredictorsNoDist[i]->Direction, sizeof(pPredictorsNoDist[i]->Direction), 1, fbND))
               Exit(1, "Failed to write to file PredictorsNoDist.bin");
            if(!fwrite(&FactorClusters[pPredictorsNoDist[i]->Marker[0]].MidPoint, sizeof(REAL), 1, fbND))
               Exit(1, "Failed to write to file PredictorsNoDist.bin");
            if(!fwrite(pPredictorsNoDist[i]->Counters+2, sizeof(CounterType), 1, fbND))
               Exit(1, "Failed to write to file PredictorsNoDist.bin");
          }
       fclose(fbND);
       return true;
     }
  void WritePredictionTestingDistHeader(FILE* f)
     {
       fprintf_s(f,"Marker\tClass\tCategory\tMidPOint\t"
                    "Direction\t"
                    "AUC-Before Corr\tAUC-After Corr\t"
                    "Prediction Rate\t"
                    "Specificity-Before Corr\t"
                    "Specificity-After Corr\t"
                    "Sensitivity-Before Corr\t"
                    "Sensitivity-After Corr\t"
                    "No of Spotters\tSpotters\n"
                );
     }
  void WritePredictionTestingDistRec(FILE *f, Predictor *p)
     {
       int i, d, &Marker=p->Marker[0];
       fprintf_s(f,"%s\t%s\t%s\t%g\t%i\t%g\t%g\t%g\t",
                   FactorNames[p->Marker[0]],
                   ClassNames[p->Class],
                   CategoryNames[p->Category],
                   FactorClusters[p->Marker[0]].MidPoint,
                   (int)p->Direction,
                   p->Counters[0].AUC, p->Counters[1].AUC,
                   p->Counters[1].PredictionRate
                );
       for(d=0; d<2; d++)
       for(i=0; i<nROC_Steps; i++)
           fprintf_s(f, "%g%c",p->Counters[d].Specificity[i],
                        i<nROC_Steps-1?' ':'\t'
                    );
       for(d=0; d<2; d++)
       for(i=0; i<nROC_Steps; i++)
           fprintf_s(f, "%g%c",p->Counters[d].Sensitivity[i],
                        i<nROC_Steps-1?' ':'\t'
                    );
       fprintf_s(f, "%d\t",p->nSpotters);
       for(i=0; i<p->nSpotters; i++)
           fprintf_s(f, "%s,%g%c",
                        FactorNames[p->Spotters[i].Spotter],
                        p->Spotters[i].Percentile,
                        i<p->nSpotters-1?';':'\n'
                    );
     }
  void WritePredictionTestingNoDistHeader(FILE* f)
     {
       fprintf_s(f,"Marker\tMidPoint\tDirection\t"
                   "AUC\tPrediction Rate\t"
                   "Specificity\tSensitivity\n"
                );
     }
  void WritePredictionTestingNoDistRec(FILE *f, Predictor *p)
     {
       int i, &Marker=p->Marker[0];

       fprintf_s(f, "%s\t%g\t%d\t%g\t%g\t",
                    FactorNames[p->Marker[0]],
                    FactorClusters[p->Marker[0]].MidPoint,
                    p->Direction,
                    p->Counters[2].AUC,
                    p->Counters[2].PredictionRate
                );
       for(i=0; i<nROC_Steps; i++)
           fprintf_s(f, "%g%c",p->Counters[2].Specificity[i],
                        i<nROC_Steps-1?' ':'\t'
                    );
       for(i=0; i<nROC_Steps; i++)
           fprintf_s(f, "%g%c",p->Counters[2].Sensitivity[i],
                        i<nROC_Steps-1?' ':'\n'
                    );
     }
  bool LoadPredictionTesting()
     {
       int i, j, k;
       FILE *fbD=0, *fbND=0;
       Predictor predictor;
       int cs=sizeof(Predictors->Marker)+
              sizeof(Predictors->nD)+
              sizeof(Predictors->Class)+
              sizeof(Predictors->Category)+
              sizeof(Predictors->Direction)+
              sizeof(Predictors->nSpotters);
       char Buf[sizeof(Predictors->Marker)+sizeof(Predictors->Direction)+sizeof(REAL)];

       nTopPredictorsNoDist=nTopPredictorsDist[0]=
       nTopPredictorsDist[1]=nTopPredictorsDist[2]=0;
       if(   NULL!=fopen_s(&fbD, PredictorsDistFName, "rb")
          || 0!=setvbuf(fbD, 0, _IOFBF, 1024*1024)
          || NULL!=fopen_s(&fbND, PredictorsNoDistFName, "rb")
          || 0!=setvbuf(fbND, 0, _IOFBF, 1024*1024)
         )
         {
           if(fbD) fclose(fbD);
           if(fbND) fclose(fbND);
           return false;
         }
       AllocateTesting();
       if(!fread(&nPredictorsDist, sizeof(nPredictorsDist), 1, fbD))
         {
           fclose(fbD);
           fclose(fbND);
           return false;
         }
       DeleteVector(pPredictorsDist);
       pPredictorsDist=NULL;
       if(   nPredictorsDist>0
          && NULL==(pPredictorsDist=NewVector<Predictor*>(nPredictorsDist))
         )
           Exit(1, "Failed to allocate memory for pPredictorsDist");
       for(i=0; i<nPredictorsDist; i++)
          {
            if(!fread(&j, sizeof(j), 1, fbD))
              {
                fclose(fbD);
                fclose(fbND);
                return false;
              }
            pPredictorsDist[i]=Predictors+j;
            if(   !fread(&Predictors[j].Marker, cs, 1, fbD)
               || !fread(&FactorClusters[Predictors[j].Marker[0]].MidPoint, sizeof(REAL), 1, fbD)
              )
              {
                fclose(fbD);
                fclose(fbND);
                return false;
              }
            if(   Predictors[j].Spotters==NULL
               && NULL==(Predictors[j].Spotters=NewVector<SpotterType>(MAX_SPOTTERS))
              )
                Exit(1, "Failed to allocate memory for Predictors[j].Spotters");
            k=Predictors[j].nSpotters*sizeof(SpotterType);
            if(   !fread(Predictors[j].Spotters, k, 1, fbD)
               || !fread(Predictors[j].Counters, sizeof(CounterType)<<1, 1, fbD)
              )
              {
                fclose(fbD);
                fclose(fbND);
                return false;
              }
          }
       fclose(fbD);

       if(!fread(&nPredictorsNoDist, sizeof(nPredictorsNoDist), 1, fbND))
         {
           fclose(fbND);
           return false;
         }
       DeleteVector(pPredictorsNoDist);
       pPredictorsNoDist=NULL;
       if(   nPredictorsNoDist>0
          && NULL==(pPredictorsNoDist=NewVector<Predictor*>(nPredictorsNoDist))
         )
           Exit(1, "Failed to allocate memory for pPredictorsNoDist");
       for(i=0; i<nPredictorsNoDist; i++)
          {
            if(!fread(&j, sizeof(j), 1, fbND))
              {
                fclose(fbND);
                return false;
              }
            pPredictorsNoDist[i]=Predictors+j;
            if(Predictors[j].Marker[0] < 0)// NoDist Only
              {
                if(   !fread(Predictors[j].Marker, sizeof(Predictors[j].Marker), 1, fbND)
                   || !fread(&Predictors[j].Direction, sizeof(Predictors[j].Direction), 1, fbND)
                   || !fread(&FactorClusters[Predictors[j].Marker[0]].MidPoint, sizeof(REAL), 1, fbND)
                  )
                  {
                    fclose(fbND);
                    return false;
                  }
              }
            else if(!fread(Buf, sizeof(Buf), 1, fbND))
                   {
                     fclose(fbND);
                     return false;
                   }
            if(!fread(Predictors[j].Counters+2, sizeof(CounterType), 1, fbND))
              {
                fclose(fbND);
                return false;
              }
          }
       fclose(fbND);

//       for(;1==fread(&predictor, cs, 1, fbD);)
//          {
//            j=(predictor.Marker[0]<<2)+((predictor.Class<<1)+predictor.Category);
//            memcpy(Predictors+j, &predictor, cs);
//          }
//
//       AllocateTesting();
//       nPredictorsDist=CountTextFileLines(fDist)-1;
//       nPredictorsNoDist=CountTextFileLines(fNoDist)-1;
//       fgets(buf, sizeof(buf), fDist); // bypass header
//       fgets(buf, sizeof(buf), fNoDist); // bypass header
//       DeleteVector(pPredictorsDist);
//       DeleteVector(pPredictorsNoDist);
//       pPredictorsDist=pPredictorsNoDist=NULL;
//       if(      nPredictorsDist>0
//             && NULL==(pPredictorsDist=NewVector<Predictor*>(nPredictorsDist))
//          ||    nPredictorsNoDist>0
//             && NULL==(pPredictorsNoDist=NewVector<Predictor*>(nPredictorsNoDist))
//         )
//           Exit(1, "Failed to allocate memory for pPredictors");
//       for(i=0; i<nPredictorsDist; i++)
//          {
//            ReadPredictionTestingDistRec(fDist, predictor);
//            j=(predictor.Marker[0]<<2)+(predictor.Class<<1)+predictor.Category;
//            Predictors[j].nD=predictor.nD;
//            Predictors[j].Marker[0]=predictor.Marker[0];
//            Predictors[j].Class=predictor.Class;
//            Predictors[j].Category=predictor.Category;
//            //Predictors[j].Counters[0]=predictor.Counters[0];
//            //Predictors[j].Counters[1]=predictor.Counters[1];
//            Predictors[j].nSpotters=predictor.nSpotters;
//            Predictors[j].Spotters=predictor.Spotters;
//            pPredictorsDist[i]=Predictors+j;
//            predictor.Spotters=NULL;
//            if(   !fread(Predictors[j].Counters, cs, 1, fbD)
//               || !fread(Predictors[j].Counters+1, cs, 1, fbD)
//              )
//               Exit(1, "Failed to read file PredictorsDist.bin");
//          }
//       predictor.Reset();
//       for(i=0; i<nPredictorsNoDist; i++)
//          {
//            ReadPredictionTestingNoDistRec(fNoDist, predictor);
//            j=(predictor.Marker[0]<<2)+(predictor.Class<<1)+predictor.Category;
//            if(Predictors[j].Marker[0]<0)
//              {
//                Predictors[j].nD=predictor.nD;
//                Predictors[j].Marker[0]=predictor.Marker[0];
//                Predictors[j].Class=predictor.Class;
//              }
//            Predictors[j].Counters[2]=predictor.Counters[2];
//            pPredictorsNoDist[i]=Predictors+j;
//            if(!fread(Predictors[j].Counters+2, cs, 1, fbND))
//               Exit(1, "Failed to read file PredictorsDist.bin");
//          }
//       fclose(fDist);
//       fclose(fNoDist);
       CalcCombined();
       return true;
     }
  //void ReadPredictionTestingDistRec(FILE *f, Predictor &p)
  //   {
  //     int i, d, &Marker=*p.Marker;
  //     REAL x;
  //     char buf[256];
  //
  //     if(-1==GetString(f, buf, sizeof(buf)))
  //        Exit(1, "Too long factor name!");
  //     if(0!=FindIndexStri(buf, (const int*)FactorNameIndex, FactorNames,nFactors, p.Marker[0]))
  //        Exit(1, "Unknown factor name!");
  //     if(-1==GetString(f, buf, sizeof(buf)))
  //        Exit(1, "Too long class name!");
  //     if(0!=_stricmp(buf, ClassNames[0]))
  //        p.Class = 0;
  //     else if(0!=_stricmp(buf, ClassNames[1]))
  //        p.Class = 1;
  //     else Exit(1, "Unknown class name!");
  //     if(-1==GetString(f, buf, sizeof(buf)))
  //        Exit(1, "Too long category name!");
  //     if(0!=_stricmp(buf, CategoryNames[0]))
  //        p.Category = 0;
  //     else if(0!=_stricmp(buf, CategoryNames[1]))
  //        p.Category = 1;
  //     else Exit(1, "Unknown category name!");
  //     GetREAL(f, FactorClusters[p.Marker[0]].MidPoint);
  //     GetInt(f, i);
  //     p.Direction=(BYTE)i;
  //     GetREAL(f, x);//p.Counters[0].AUC);
  //     GetREAL(f, x);//p.Counters[1].AUC);
  //     GetREAL(f, x);//p.Counters[1].PredictionRate);
  //     for(d=0; d<2; d++)
  //     for(i=0; i<nROC_Steps+2; i++)
  //         GetREAL(f, x);//p.Counters[d].Specificity[i]);
  //     for(d=0; d<2; d++)
  //     for(i=0; i<nROC_Steps+2; i++)
  //         GetREAL(f, x);//p.Counters[d].Sensitivity[i]);
  //     GetInt(f, p.nSpotters);
  //     if(p.nSpotters>0)
  //       {
  //         if(NULL==(p.Spotters=NewVector<SpotterType>(MAX_SPOTTERS)))
  //            Exit(1, "Failed to allocate memory for predictor.Spotters!");
  //         for(i=0; i<p.nSpotters; i++)
  //            {
  //              if(i>0) getc(f);
  //              if(-1==GetString(f, buf, sizeof(buf), ','))
  //                 Exit(1, "Too long factor name!");
  //              if(0!=FindIndexStri(buf, (const int*)FactorNameIndex, FactorNames,nFactors, p.Spotters[i].Spotter))
  //                 Exit(1, "Unknown factor name!");
  //              GetREAL(f, p.Spotters[i].Percentile);
  //            }
  //       }
  //     else p.Spotters=NULL;
  //   }
  //void ReadPredictionTestingNoDistRec(FILE *f, Predictor &p)
  //   {
  //     int i, &Marker=*p.Marker;
  //     char buf[256];
  //
  //     if(-1==GetString(f, buf, sizeof(buf)))
  //        Exit(1, "Too long factor name!");
  //     if(0!=FindIndexStri(buf, (const int*)FactorNameIndex, FactorNames, nFactors, p.Marker[0]))
  //        Exit(1, "Unknown factor name!");
  //     GetREAL(f, FactorClusters[p.Marker[0]].MidPoint);
  //     GetInt(f, i);
  //     p.Direction=(BYTE)i;
  //     GetREAL(f, p.Counters[2].AUC);
  //     GetREAL(f, p.Counters[2].PredictionRate);
  //     for(i=0; i<nROC_Steps+2; i++)
  //         GetREAL(f,p.Counters[2].Specificity[i]);
  //     for(i=0; i<nROC_Steps+2; i++)
  //         GetREAL(f,p.Counters[2].Sensitivity[i]);
  //   }
  bool LoadSyn2_Score_HeatMap()
     {
       int i, j;
       BOOL RetVal;
       FILE *f;

       if(NULL!=fopen_s(&f, "Syn2_Score_HeatMap.bin", "rb"))
          return false;
       RetVal =   1==fread(Syn2_Score_HeatMap, sizeof(Syn2_Score_HeatMap), 1, f)
               && 1==fread(&MaxHeatMap, sizeof(MaxHeatMap), 1,f);
       fclose(f);
       if(RetVal)
         {
           for(j=0; j<MAXFREQ; j++)
              {
                DistFreq_1F[j]=Syn2_Score_HeatMap[0][j];
                for(i=1; i<HEATMAPHEIGHT; i++)
                    DistFreq_1F[j]+=Syn2_Score_HeatMap[i][j];
              }
         }
       return RetVal;
     }
  bool SaveSyn2_Score_HeatMap()
     {
       FILE *f;

       if(NULL!=fopen_s(&f, "Syn2_Score_HeatMap.bin", "wb"))
           return false;
       fwrite(Syn2_Score_HeatMap, sizeof(Syn2_Score_HeatMap), 1, f);
       fwrite(&MaxHeatMap, sizeof(MaxHeatMap), 1, f);
       fclose(f);
       return true;
     }
  //bool ReadSingleRec(FILE *f, Dist_SingleRect &rct)
  //   {
  //     int i, d;
  //     char buf[256];
  //     REAL x;
  //
  //     if(-1==GetString(f, buf, sizeof(buf)))// spotter
  //        return false;
  //     if(-1==GetString(f, buf, sizeof(buf)))// marker
  //        return false;
  //     //if(0!=FindIndexStri(buf, (const int*)FactorNameIndex, FactorNames,nFactors, Marker))
  //     //   Exit(1, "Unknown factor name!");
  //     if(-1==GetString(f, buf, sizeof(buf)))//Class
  //        return false;
  //     if(0!=_stricmp(buf, ClassNames[0]))
  //        rct.Class = 0;
  //     else if(0!=_stricmp(buf, ClassNames[1]))
  //        rct.Class = 1;
  //     else return false;
  //     if(-1==GetString(f, buf, sizeof(buf)))//Category
  //        return false;
  //     GetREAL(f, x);//Score
  //     rct.Score=x*10000;
  //     GetREAL(f, x);//Precision
  //     GetREAL(f, x);//I
  //     GetREAL(f, x);//Icorr
  //     GetREAL(f, x);
  //     rct.Syn2=x*10000;
  //     GetEOL(f);
  //     return true;
  //   }
  bool Pass(char* &p, char *q, char sep='\t', int count =1)
     {
       for(int i=0; i<count; i++)
          {
            for(p++; p<q && *p && *p!=sep; p++)
               {}
            if(!*p || p>=q)
               return false;
          }
       return true;
     }
  //void CalcROC_Steps(Predictor &p, REAL *ROC_Steps)
  //   {
  //     int i, &Marker=*p.Marker;
  //     REAL step=(FactorMax[Marker]-FactorMin[Marker])/(nROC_Steps-1);
  //
  //     if(Directions[Marker])
  //       {
  //         ROC_Steps[0]=FactorMin[Marker];
  //         for(i=1; i<nROC_Steps; i++)
  //             ROC_Steps[i]=ROC_Steps[i-1]+step;
  //       }
  //     else
  //       {
  //         ROC_Steps[nROC_Steps-1]=FactorMin[Marker];
  //         for(i=nROC_Steps-2; i>=0; i--)
  //             ROC_Steps[i]=ROC_Steps[i+1]+step;
  //       }
  //   }
  //bool PretrainedTest(char *PT_PredictorsFileName)
  //   {
  //     int i, j, k, l, n, len;
  //     static char buf[0xC000];
  //     REAL ROC_Steps[MAXROCSTEPS];
  //     static const REAL Bins[]={0.01,0.02,0.03,0.04,0.05,
  //                               0.06,0.07,0.08,0.09,0.10}; //MinSpotRates
  //     char *p, *q, *e;
  //     FILE *f;
  //
  //     if(NULL!=fopen_s(&f, PT_PredictorsFileName, "rt"))
  //        return false;
  //     for(nPredictors=0; fgets(buf, sizeof(buf), f); nPredictors++)
  //        {
  //          len=(int)strlen(buf);
  //          if(len<13)
  //             break;
  //          if(buf[len-1]!='\n')
  //             return false;//Exit(1, "Bad predictors' file format");
  //        }
  //     rewind(f);
  //     //AUC=NewVector<REAL>(nPredictors);
  //     SortStri(FactorNames, nFactors);
  //     Predictors=NewVector<Predictor>(nPredictors);
  //     for(l=i=0; l<nPredictors; l++)
  //        {
  //          int &Marker=Predictors[i].Marker[0];
  //          fgets(buf, sizeof(buf), f);
  //          len = (int)strlen(buf);
  //          e = buf+len;
  //          p=q=buf;
  //          if(!Pass(q, e))
  //             return false;//Exit(1, "Bad predictors' file format");
  //          *q=0;
  //          if(!FindStri(p, (const char**)FactorNames, nFactors, j))
  //            {
  //              Predictors[i].Marker[0] = j;
  //              p=++q;
  //              if(5!=sscanf_s(p, "%lf\t%lf\t%lf\t%d\t%d\t",
  //                             Predictors[i].Counters[0].AUC,
  //                             Predictors[i].Counters[0].PredictionRate,
  //                             MidPoints+Marker,
  //                             Directions+Marker,
  //                             &Predictors[i].nSpotters
  //                            )
  //                 || Predictors[i].Counters[0].PredictionRate < 0.0
  //                 || Predictors[i].Counters[0].PredictionRate > 1.0
  //                 || Directions[Marker]>1
  //                 || Directions[Marker]<0
  //                 || Predictors[i].nSpotters <= 0
  //                )
  //                  return false;//Exit(1, "Bad predictors' file format");
  //              if(!Pass(p, e, '\t', 5))
  //                 Exit(1, "Bad predictors' file format");
  //              MidPoints[Marker]=DeNormalize(MidPoints[Marker], Marker);
  //              if(NULL==(Predictors[i].Spotters=
  //                        NewVector<SpotterType>(Predictors[i].nSpotters)
  //                       )
  //                )
  //                 return false;//Exit(1, "Failed to Allocate Memory for Spotters");
  //              for(j=n=0; n<Predictors[i].nSpotters; n++)
  //                 {
  //                   SpotterType &md=Predictors[i].Spotters[j];
  //                   q=++p;
  //                   if(!Pass(p, e, ' '))
  //                      return false;//Exit(1, "Bad predictors' file format");
  //                   *p++=0;
  //                   if(!FindStri(q, (const char**)FactorNames, nFactors, md.Spotter))
  //                     {
  //                       if(1!=sscanf_s(p, "%lf", &md.Percentile))
  //                          return false;//Exit(1, "Bad predictors' file format");
  //                       if(!Pass(p, e, n<Predictors[i].nSpotters-1?' ':'\n'))
  //                          return false;//Exit(1, "Bad predictors' file format");
  //                       md.Percentile=DeNormalize(md.Percentile,md.Spotter);
  //                       j++;
  //                     }
  //                 }
  //              Predictors[i].nSpotters = j;
  //              i++;
  //            }
  //        }
  //     nPredictors = i;
  //     fclose(f);
  //     int TrueC, TestC, bin, tfpn;
  //     REAL cs[CountOf(Bins)][MAXROCSTEPS][4]={0};
  //     REAL data, data1;
  //
  //     for(i=0; i<nPredictors; i++)
  //        {
  //          int &Marker=Predictors[i].Marker[0];
  //          REAL MidPoint = MidPoints[Marker];
  //          int &Direction = Directions[Marker];
  //
  //          //CalcROC_Steps(Predictors[i], ROC_Steps);
  //          for(j=0; j<nSamples; j++)
  //             {
  //               TrueC = SampleClasses[j];
  //               data = FactorData[Marker][j];
  //               TestC=ClassifySample(data, MidPoint, Direction);
  //               for(l=n=0; l<Predictors[i].nSpotters; l++)
  //                if(   !TestC
  //                   && SampleInTail(j, Predictors[i].Spotters[l])
  //                  )
  //                    n++;
  //               for(bin=0; bin<CountOf(Bins); bin++)
  //                  {
  //                    data1=(n >= l*Bins[bin])? MidPoint+MidPoint-data
  //                                            : data;
  //                    for(k=0; k<nROC_Steps; k++)
  //                       {
  //                         TestC=ClassifySample(data1, ROC_Steps[k], Direction);
  //                         tfpn = TestC?(TrueC?0   // TP
  //                                            :1)  // FP
  //                                     :(TrueC?3   // FN
  //                                            :2); // TN
  //                         cs[bin][k][tfpn]+=Predictors[i].Counters[0].PredictionRate;
  //                       }
  //                  }
  //             }
  //        }
  //     if(NULL!=fopen_s(&f, "Predictors-Results.txt","wt"))
  //        return false;//Exit(1, "Failed to create file: Predictors-Results.txt");
  //     for(bin=0; bin<CountOf(Bins); bin++)
  //        {
  //          REAL AUC=0, x=0, y=0;
  //          fprintf_s(f, "Min Spotting Rate>=%4.2f\n", Bins[bin]);
  //          fprintf_s(f,"SepPt\tTP\tFP\tTN\tFN\tTPR\tFPR\t"
  //                                         "Sensitivity\tSpecificity\n");
  //          for(i=0; i<nROC_Steps; i++)
  //             {
  //               REAL TP=cs[bin][i][0], FP=cs[bin][i][1],
  //                    TN=cs[bin][i][2], FN=cs[bin][i][3],
  //                    P=TP+FN, N=TN+FP,
  //                    TPR=P>0?TP/P:1, FPR=N>0?FP/N:1,
  //                    Sensitivity=TPR, Specificity=1-FPR;
  //               fprintf_s(f, "%5d\t%.0f\t%.0f\t%.0f\t%.0f\t"
  //                            "%5.3f\t%5.3f\t%5.3f\t%5.3f\n",
  //                            i+1, TP, FP, TN, FN, TPR, FPR,
  //                            Sensitivity, Specificity
  //                        );
  //               AUC += (TPR+y)/2.0 * (FPR-x);
  //               x = FPR;
  //               y = TPR;
  //             }
  //          AUC += (1.0+y)/2.0 * (1.0-x);
  //          fprintf_s(f, "AUC = %4.3f\n\n", AUC);
  //        }
  //     fclose(f);
  //     return true;
  //   }
  //void RepositoryEntry(char *fNameDistInstances, char *fNameTestSamples)
  //   {
  //     int nInstances;
  //     DistortionInstance *Instances;
  //     FILE *fd, *ft;
  //
  //     if(0!=fopen_s(&fd, fNameDistInstances, "rb"))
  //       {
  //         printf("Failed to open file %s\nPress any key",
  //                fNameDistInstances);
  //         getchar();
  //         return;
  //       }
  //     if(0!=fopen_s(&ft, fNameTestSamples, "rt"))
  //       {
  //         printf("Failed to open file %s\nPress any key",
  //                fNameTestSamples);
  //         getchar();
  //         fclose(fd);
  //         return;
  //       }
  //     fseek(fd, 0, SEEK_END);
  //     nInstances = ftell(fd)/sizeof(DistortionInstance);
  //     fseek(fd, 0, SEEK_SET);
  //     Instances = NewVector<DistortionInstance>(nInstances);
  //     fread_s(Instances, nInstances*sizeof(DistortionInstance),
  //             sizeof(DistortionInstance), nInstances, fd);
  //         
  //        
  //     DeleteVector(Instances)
  //     fclose(fd);
  //     fclose(ft);
  //   }
  REAL Syn2(int i, int j, REAL I_2)
     {
       if(i<j)
         {
           int k=i;
           i=j;
           j=k;
         }
       //REAL I2=HC-CndH_2F[i][j], Ii=HC-CndH_1F[i], Ij=HC-CndH_1F[j],
       //       Syn=I2-Ii-Ij, Syn_=CndH_1F[i]+CndH_1F[j]-CndH_2F[i][j]-HC;
       return I_2-HC-HC+CndH_1F[i]+CndH_1F[j];
     }
  REAL Syn3(int F1, int F2, int F3)
     {
       REAL *Data[3]={FactorData[F1],FactorData[F2],FactorData[F3]},
             I3=HC-CndH(3, nSamples, Data, NULL),
             I111=HC-CndH_1F[F1]+HC-CndH_1F[F2]+HC-CndH_1F[F3],
             I1_23=HC-CndH_1F[F1]+HC-CndH_2F[F2>F3?F2:F3][F2<F3?F2:F3],
             I2_13=HC-CndH_1F[F2]+HC-CndH_2F[F1>F3?F1:F3][F1<F3?F1:F3],
             I3_12=HC-CndH_1F[F3]+HC-CndH_2F[F1>F2?F1:F2][F1<F2?F1:F2],
             Iall=I111>I1_23?I111:I1_23;
       
       Iall = Iall>I2_13?Iall:I2_13;
       Iall = Iall>I3_12?Iall:I3_12;
       return I3-Iall;
     }
  static REAL Separation(int nClusters,void *clusters,void *d,int f)
     {// separation score for the clustering
       return( (Dataset*)d)->HC-
              ((Dataset*)d)->CondH(nClusters,(Cluster*)clusters);
     }
  void WriteDistFileHeader(int nd, FILE *f)
     {
       int i;
       char *I1=nd==1?"I":"Syn", *I2=nd==1?"I_Cor":"Syn_Cor";

       //if(!EvaluateCorrectionRates)
       fprintf_s(f, "Fs\tFm");
       for(i=1; i<nd; i++)
           fprintf_s(f, "%d\tFm",i);
       if(nd>1)
          fprintf_s(f, "%d",i);
       //if(EvaluateCorrectionRates)
       //   fprintf_s(f, "\t%s\t%s\tFP#\tFP\tFP_Cor#\tFP_Cor"
       //                 "\tFN#\tFN\tFN_Cor#\tFN_Cor\n",I1,I2);
       //else
       fprintf_s(f, "\tClass\tCategory\tScore\tPrecision\tCohesion"
                    "\t%s\t%s\tSyn-%d\tMislabeled\n",
                    I1, I2, nd+1);
     }
  //static int __cdecl CompUINT28(const void*i1, const void*i2)
  //   {
  //     return ((*(UINT*)i1)&0x3fffffff)>((*(UINT*)i2)&0x3fffffff)?1:
  //           (((*(UINT*)i1)&0x3fffffff)<((*(UINT*)i2)&0x3fffffff)?-1:0);
  //   }
  void WriteDistFileRecord(DistortionInstance &Dist, FILE *f)
     {
       int i, j;

       fprintf_s(f, FactorNames[Dist.Spotter]);
       for(i=0; i<Dist.nD; i++)
           fprintf_s(f, "\t%s",FactorNames[Dist.Marker[i]]);
       fprintf_s(f, "\t%s\t%s\t%9.6f\t%9.6f\t%9.6f\t%9.6f\t%9.6f"
                    "\t%9.6f\t",ClassNames[Dist.Class],
                    CategoryNames[Dist.Category],
                    Dist.Score, Dist.Precision, Dist.Cohesion,
                    Dist.InfSyn, Dist.InfSynCor, Dist.HypSyn);
       for(i=j=0; j<Dist.nMis; i++)
           fprintf_s(f, "%s%s", SampleNames[Dist.Mis[i]],
                     ++j==Dist.nMis?"":" ");
       fprintf_s(f, "\n");
     }
  void DivideVector(int nElements, int nParts, int** OffsetsTable)
     {
       int i;
       REAL Step=nElements/nParts, Cummulator;

       for(i=0,Cummulator=0; i<nParts; i++)
          {
            OffsetsTable[i][0] = int(Cummulator);
            OffsetsTable[i][1] = int(Cummulator+=Step)-1;
          }
       OffsetsTable[nParts-1][1] = nElements-1;
//for(i=0;i<nParts;i++)
//printf_s("%d\t%d\n", OffsetsTable[i][0], OffsetsTable[i][1]);
//getchar();
     }
  int TriangularMatrixSize(int n)
     {
       return (n*n-n)>>1;
     }
  int DivideTriangularMatrix(int nRows, int nParts, int *RowOfsets)
     {// divide triangular matrix into nParts equal size parts
       //(x*x)/(nRows*nRows) = 1/nParts
       //x*x =  nRows*nRows/nParts
       // x = nRows / Sqrt(nParts)
       //x1 = nRows / Sqrt(nParts/1)
       //x2 = nRows / Sqrt(nParts/2)
       //xi = nRows / Sqrt(nParts/i)
       RowOfsets[0] = 1;
       for(int i=1; i<nParts; i++)
           RowOfsets[i]=1+(int)(0.5+(nRows-1)/sqrt(nParts/i));
       //each equal part starts from row RowOfsets[i]
       // to row RowOfsets[i+1]-1
       // so, all thread is assigned an equal load
       // we calculate using nRows-1 because firts row is empty
       // then, we add 1 to neglect the empty row
       // we round up x by (int)(0.5+x) to send the fractions
       // to the shorters rows, so, keep max equality
       // Example: nRows=10, nParts = 3
       //   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
       //---|---|---|---|---|---|---|---|---|---|---|
       // 0 |   |   |   |   |   |   |   |   |   |   |
       //---|---|---|---|---|---|---|---|---|---|---|
// --> // 1 | * |   |   |   |   |   |   |   |   |   |-- RowOffsets[0]
       //---|---|---|---|---|---|---|---|---|---|---|
       // 2 | * | * |   |   |   |   |   |   |   |   |
       //---|---|---|---|---|---|---|---|---|---|---|
       // 3 | * | * | * |   |   |   |   |   |   |   |
       //---|---|---|---|---|---|---|---|---|---|---|
       // 4 | * | * | * | * |   |   |   |   |   |   |
       //---|---|---|---|---|---|---|---|---|---|---|
       // 5 | * | * | * | * | * |   |   |   |   |   |
       //---|---|---|---|---|---|---|---|---|---|---|
// --> // 6 | * | * | * | * | * | * |   |   |   |   |-- RowOffsets[1]
       //---|---|---|---|---|---|---|---|---|---|---|
       // 7 | * | * | * | * | * | * | * |   |   |   |
       //---|---|---|---|---|---|---|---|---|---|---|
// --> // 8 | * | * | * | * | * | * | * | * |   |   |-- RowOffsets[2]
       //---|---|---|---|---|---|---|---|---|---|---|
       // 9 | * | * | * | * | * | * | * | * | * |   |
       //---l---l---l---l---l---l---l---l---l---l---l
       // RowOffsets[0] = 1
       // RowOffsets[1] = 1+(int)(0.5+9/sqrt(3/1))= 6
       // RowOffsets[1] = 1+(int)(0.5+9/sqrt(3/2))= 8
       // Sizes of parts= 15, 13, 17 out of 45 elements
       // for a bigger matrix, say 100 rows:
       // RowOffsets[0] = 1
       // RowOffsets[1] = 1+(int)(0.5+99/sqrt(3/1))= 58
       // RowOffsets[1] = 1+(int)(0.5+99/sqrt(3/2))= 82
       // Sizes of parts= 3249, 3312, 3240 out of 9801 elements
     }
  void DivideTriangularMatrix(int nRows, int nParts, int **Offsets)
     {// divide triangular matrix into nParts equal size parts
      // returns StartRow, StartCol, EndRow, EndCol for each part
       int i, Sum, StartRow=1, StartCol=0, EndRow=StartRow, EndCol;
       REAL PartSize, CummlativeSize=0;

       PartSize = REAL(nRows*nRows-nRows)/(nParts+nParts);
//printf_s("\nTotal=%d\nnParts=%d\nPartSize=%7.2f",
//(nRows*nRows-nRows)/2,nParts, PartSize);
       for(i=Sum=0; i<nParts; i++)
          {
            CummlativeSize += PartSize;
            Offsets[i][0] = StartRow;
            Offsets[i][1] = StartCol;
            Sum += StartRow-StartCol;
            if(Sum>CummlativeSize+0.01)
              {
                Sum -= StartRow-StartCol;
                EndRow=StartRow;
                EndCol=StartCol+int(CummlativeSize-Sum-1);
                Sum=int(CummlativeSize);
              }
            else
              {
                while(Sum+EndRow+1<=CummlativeSize)
                      Sum += ++EndRow;
                if(CummlativeSize-Sum>=0.99)
                  {
                    EndRow++;
                    EndCol = int(CummlativeSize-Sum-1);
                    Sum += EndCol+1;
                  }
                else
                   EndCol = EndRow-1;
              }
            Offsets[i][2] = EndRow;
            Offsets[i][3] = EndCol;
            StartRow = EndRow;
            StartCol = EndCol+1;
            if(StartCol==StartRow)
              {
                StartCol = 0;
                StartRow++;
              }
//int s=(Offsets[i][2]&1)
//?(Offsets[i][2]-1)/2*Offsets[i][2]+Offsets[i][3]+1
//:(Offsets[i][2])/2*(Offsets[i][2]-1)+Offsets[i][3]+1;
//printf_s("\n%d\t%d\t%d\t%d\t\t%7.2f\t%d\t%d",
//Offsets[i][0],Offsets[i][1],
//Offsets[i][2],Offsets[i][3],
//CummlativeSize, Sum,s
//);
          }
        Offsets[i-1][3]=nRows-2;
//printf_s("\n\n%d\t%d\t%d\t%d",Offsets[i-1][0],Offsets[i-1][1],
//Offsets[i-1][2],Offsets[i-1][3]);
//getchar();
     }
};

  //REAL NormalDistFitness(REAL *Data, int n,
  //                       REAL Mean, REAL SD, REAL *Fs,
  //                       REAL Quantiles[4][2])
  //   {
  //     int i, j, Counters[4];
  //     const REAL Ps[4]={0.682689, 0.8, 0.9, 0.95};
  //     REAL F,  Sigmas[4] = {SD, 1.281551*SD, 1.644853*SD, 1.959963*SD};
  //
  //     for(i=0; i<4; i++)
  //        {
  //          Quantiles[i][0] = Mean-Sigmas[i];
  //          Quantiles[i][1] = Mean+Sigmas[i];
  //        }
  //     for(i=0; i<CountOf(Counters); i++)
  //         Counters[i] = n;
  //     for(i=0; i<n; i++)
  //        {
  //          for(j=0; j<4; j++)
  //           if(Data[i]<Quantiles[j][0] || Data[i]>Quantiles[j][1])
  //              Counters[j]--;
  //        }
  //     F = 1;
  //     for(i=0; i<4; i++)
  //        {
  //          Fs[i] = Counters[i] / (Ps[i]*n);
  //          if(Fs[i]>=1.0)
  //             F /= Fs[i];
  //          else F *= Fs[i];
  //        }
  //     return F;
  //   }
  //REAL Silhouette(int n, int *samples, REAL *data, REAL gMean,
  //                REAL &Mean, REAL &Var, REAL &Max, REAL &Min)
  //   {
  //     int i;
  //     REAL S, a, b;
  //
  //     Max=Min=data[*samples];
  //     for(Mean=i=0; i<n; i++)
  //        {
  //          Mean += data[samples[i]];
  //               if(Max<data[samples[i]])
  //                  Max=data[samples[i]];
  //          else if(Min>data[samples[i]])
  //                  Min=data[samples[i]];
  //        }
  //     Mean /= n;
  //     for(Var=i=0; i<n; i++)
  //         Var += data[samples[i]]>Mean?data[samples[i]]-Mean
  //                                     :Mean-data[samples[i]];
  //     Var /= n;
  //     for(S=i=0; i<n; i++)
  //        {
  //          a = data[samples[i]]-Mean;
  //          if(a<0)
  //             a=-a;
  //          b = data[samples[i]]-gMean;
  //          if(b<0)
  //             b=-b;
  //          S += (b-a)/((b>a)?b:a);
  //        }
  //     return S/n;
  //   }
  