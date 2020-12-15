#pragma once

#include"MemoryAllocation.h"
#include<math.h>
#include<CRTDBG.H>

#define MAX_DIMENSIONS 2
#define CountOf(X) (sizeof(X)/sizeof(*(X)))
#define REAL_MAX DBL_MAX
#define MAX_CLASSES 2
#define MAX_SAMPLES 128
typedef double REAL;

class UPGMA
{
public:
  struct Cluster
  {
    WORD nSamples, Samples[MAX_SAMPLES];
    WORD ClassCounts[MAX_CLASSES];
    BYTE Class;
  };
protected:
  struct Node
  {
    int Level;
    REAL Center[MAX_DIMENSIONS], Separation;
    int nSamples, *Samples, Sample;
    int ClassCounts[MAX_CLASSES];
    BYTE Class;
    Node *I, *J;
    Node()
      {
        I = J = 0;
        Samples = 0;
        Separation = -1;
      }
   ~Node()
      {
        DeleteVector(Samples);
      }
  };
  typedef unsigned char BYTE;
  struct Merger{int r1, r2, deleted; Node *n10, *n20, *n11;};
  int Size, nSamples, nClasses, nRoots, Level, nD;// nD:# of dimensions
  BYTE *SampleClasses;
  int StartMeasure, nAllNodes, NavStackPtr,
      *AffectedFactors, nAffectedFactors;
  REAL **DistanceMatrix, *DistMatBuf, *MinDist;
  Node **Roots, *Nodes, *pnode;
  Merger *NavStack;
  BYTE *Mask;
  REAL(*SeparationCallback)(int,void*,void*,int);
  void *Owner;
public:
  Cluster *Clusters;
  UPGMA()
     {
       Size = 0;
       Roots=0;
       Nodes=0;
       DistanceMatrix=0;
       DistMatBuf=MinDist=0;
       NavStack=0;
       AffectedFactors=0;
     }
 ~UPGMA()
     {
       Reset();
     }
  void Reset()
     {
       DeleteVector(DistanceMatrix);
       DeleteVector(DistMatBuf);
       DeleteVector(MinDist);
       DeleteVector(Roots);
       DeleteVector(Nodes);
       DeleteVector(NavStack);
       DeleteVector(AffectedFactors);
     }
  void Init(int nsamples, int nd, Cluster *clusters, int startmeasure,
           void *owner, BYTE *mask, int nclasses, BYTE *sampleclasses,
           REAL(*separationcallback)(int, void*, void*, int)
          )
     {
       nD = nd;
       Clusters = clusters;
       StartMeasure = startmeasure;
       Mask = mask;
       Owner = owner;
       SeparationCallback = separationcallback;
       nSamples = nsamples;
       nClasses = nclasses;
       SampleClasses = sampleclasses;
       nAllNodes = (nSamples<<1);
       if(Size<nSamples) // object reused for a bigger dataset
         {
           Reset();
           DistanceMatrix = NewVector<REAL*>(nSamples);
           DistMatBuf=NewVector<REAL>((nSamples*nSamples-nSamples)>>1);
           MinDist=NewVector<REAL>(nSamples);
           Roots = NewVector<Node*>(nSamples);
           Nodes = NewVector<Node>(nAllNodes);
           NavStack = NewVector<Merger>(nSamples);
           AffectedFactors=NewVector<int>(nSamples);
           Size = nSamples;
           for(int i=nSamples; i<nAllNodes; i++)
               Nodes[i].Samples = NewVector<int>(nSamples);
         }
       else if(Size>nSamples)
         {
           for(int i=nSamples; i<nAllNodes; i++)
            if(Nodes[i].Samples==0)
               Nodes[i].Samples = NewVector<int>(nSamples);
            else
               break;
         }
     }
  void Partition()
     {
       for(int i=0; i<nRoots; i++)
          {
            Clusters[i].nSamples = Roots[i]->nSamples;
            if(Roots[i]->nSamples==1)
               Clusters[i].Samples[0] = Roots[i]->Sample;
               //Clusters[i].Samples = &Roots[i]->Sample;
            else
               for(int j=0; j<Roots[i]->nSamples; j++)
                   Clusters[i].Samples[j] = Roots[i]->Samples[j];
               //Clusters[i].Samples = Roots[i]->Samples;
            Clusters[i].Class = Roots[i]->Class;
            for(int j=0; j<nClasses; j++)
                Clusters[i].ClassCounts[j]=Roots[i]->ClassCounts[j];
          }
     }
  void MinDistance(int &I, int &J)
     {// this function guarantees I<J
       int j;
       REAL *p=MinDist, *q=p, *r=MinDist+nRoots;

       while(++p<r)
          if(*q>*p)
             q=p;
       I=(int)(q-MinDist);
_ASSERTE(I>=0 && I<nRoots);
#ifdef _DEBUG
static char b[10*1024];
ShowDistMatrix(b,sizeof(b),I,J);
#endif
       for(J=0; J<I; J++)
        if(*q==DistanceMatrix[I][J])
           break;

       if(J==I)
         {
           while(*q!=DistanceMatrix[++J][I])
                {
_ASSERTE(J<nRoots);
                }
         }
       else
         {
           j=J;
           J=I;
           I=j;
         }
_ASSERTE(J>0 && J<nRoots && J!=I);
     }
  void RemoveRowCol(int J)
     {
       int i, j;

       for(i=J; i<nRoots; i++)
          {
            DistanceMatrix[i] = DistanceMatrix[i+1];
            MinDist[i]=MinDist[i+1];
          }
       for(i=J+1; i<nRoots; i++)
          {
            for(j=J; j<i; j++)
                DistanceMatrix[i][j] = DistanceMatrix[i][j+1];
          }
     }
  REAL Distance(int I, int J)
     {
       REAL distance = 0, d;

       for(int i=0; i<nD; i++)
          {
            d = Roots[I]->Center[i] - Roots[J]->Center[i];
            distance += d*d;
          }
       if(nD==1)
          return sqrt(distance);
       else
          return (Roots[I]->Class==Roots[J]->Class)?sqrt(distance)
                                                   :sqrt(distance)*1.1;
     }
  void ReCalcDistances(int I, int J)
     {
       int i;

       for(i=0; i<I; i++)
           DistanceMatrix[I][i] = Distance(I, i);
       while(++i<nRoots)
            {
              DistanceMatrix[i][I] = Distance(i, I);
              if(MinDist[i]>DistanceMatrix[i][I])
                 MinDist[i]=DistanceMatrix[i][I];
            }
       CalcMinDist(I);
       while(--nAffectedFactors>=0)
             CalcMinDist(AffectedFactors[nAffectedFactors]);
     }
  void CalcDistances()
     {
       int i;

       for(i=1; i<nRoots; i++)
          {
            for(int j=0; j<i; j++)
                DistanceMatrix[i][j] = Distance(i, j);
          }
       for(i=0; i<nRoots;i++)
           CalcMinDist(i);
     }
#ifdef _DEBUG
void ShowDistMatrix(char *b, int bufsize, int I, int J)
{
int i, j, l;
char *p=b;

return;
sprintf_s(p, bufsize, "\nI=%d\tJ=%d\n\tMinDst",I,J);
l=(int)strlen(p);
p+=l;
bufsize-=l;
for(i=0; i<nRoots; i++)
   {
     sprintf_s(p, bufsize, "\t  %d",i);
     l=(int)strlen(p);
     p+=l;
     bufsize-=l;
   }
sprintf_s(p, bufsize, "\n");
l=(int)strlen(p);
p+=l;
bufsize-=l;
for(i=0; i<nRoots; i++)
   {
     sprintf_s(p, bufsize, "%2d-\t%5.2f",i,MinDist[i]);
     l=(int)strlen(p);
     p+=l;
     bufsize-=l;
     for(j=0; j<i; j++)
        {
          sprintf_s(p, bufsize, "\t%5.2f",DistanceMatrix[i][j]);
          l=(int)strlen(p);
          p+=l;
          bufsize-=l;
        }
     sprintf_s(p, bufsize, "\n");
     l=(int)strlen(p);
     p+=l;
     bufsize-=l;
   }
printf_s(b);
getchar();
}
#endif

  void CalcMinDist(int i)
     {
       int j;
       REAL mindst;

       mindst=REAL_MAX;
       for(j=0; j<i; j++)
        if(mindst>DistanceMatrix[i][j])
           mindst=DistanceMatrix[i][j];
         
       for(++j; j<nRoots; j++)
        if(mindst>DistanceMatrix[j][i])
           mindst=DistanceMatrix[j][i];
       MinDist[i]=mindst;
     }
  void FindMergeAffectedFactors(int I, int J)
     {
       nAffectedFactors=0;
       for(int i=0; i<nRoots; i++)
        if(i!=I&&i!=J)
          {
            if(   DistanceMatrix[i<I?I:i][i<I?i:I]==MinDist[i]
               || DistanceMatrix[i<J?J:i][i<J?i:J]==MinDist[i]
              )
               AffectedFactors[nAffectedFactors++]=i<J?i:i-1;
          }
     }
  bool Merge(int I, int J)
     {// expecting I<J
       int i, j, *p, *q, *r;

       if(pnode-Nodes>=nAllNodes)// not supposed to happen
          return false;
       NavStack[NavStackPtr].r1=I;
       NavStack[NavStackPtr].r2=J;
       NavStack[NavStackPtr].n10 = Roots[I];
       NavStack[NavStackPtr].n20 = Roots[J];
       pnode->nSamples = Roots[I]->nSamples+Roots[J]->nSamples;
       pnode->I = Roots[I];
       pnode->J = Roots[J];
       pnode->Level = ++Level;
       r = pnode->Samples;
       if(pnode->I->nSamples==1)
          *r++ = pnode->I->Sample;
       else
         {
           p = pnode->I->Samples;
           q = p + pnode->I->nSamples;
           while(p<q)
                 *r++ = *p++;
         }
       if(pnode->J->nSamples==1)
          *r++ = pnode->J->Sample;
       else
         {
           p = pnode->J->Samples;
           q = p + pnode->J->nSamples;
           while(p<q)
                 *r++ = *p++;
         }
       for(i=0; i<nD; i++)
           pnode->Center[i]=(Roots[I]->Center[i]*Roots[I]->nSamples+
                             Roots[J]->Center[i]*Roots[J]->nSamples
                            ) / pnode->nSamples;
       for(i=0,j=0; i<nClasses; i++)
          {
            pnode->ClassCounts[i]=Roots[I]->ClassCounts[i]+
                                  Roots[J]->ClassCounts[i];
            if(j<pnode->ClassCounts[i])
              {
                j=pnode->ClassCounts[i];
                pnode->Class = i;
              }
          }
       FindMergeAffectedFactors(I, J);
       Roots[I] = pnode;
       NavStack[NavStackPtr].n11 = Roots[I];
       NavStack[NavStackPtr++].deleted = J;
#ifdef _DEBUG
static char b[10*1024];
ShowDistMatrix(b,sizeof(b),I,J);
#endif
       --nRoots;
       for(i=J;i<nRoots; i++)
           Roots[i]=Roots[i+1];
#ifdef _DEBUG
ShowDistMatrix(b,sizeof(b),I,J);
#endif
       RemoveRowCol(J);
#ifdef _DEBUG
ShowDistMatrix(b,sizeof(b),I,J);
#endif
       ReCalcDistances(I, J);
#ifdef _DEBUG
ShowDistMatrix(b,sizeof(b),I,J);
#endif
       //CalcDistances();
       pnode++; // move to the next available node
       return true;
     }
  int DoClustering(int UntilnClusters, REAL &BestScore, REAL **Data,
                   int f=0)
     {
       int i, j, k, BestnClusters;
       int I, J;
       REAL Score;

       nRoots = nSamples;
       DistanceMatrix[0]=0;
       for(i=1,j=0; i<nSamples; i++,j+=i-1)
           DistanceMatrix[i]=DistMatBuf+j;
       pnode = Nodes+nSamples; // first available for Merge()
       Level = -1;
       for(i=j=0; j<nSamples; i++)
          {
            if(Mask!=0 && Mask[i]==0)
               continue;
            Roots[j]=Nodes+j;
            Roots[j]->Sample = i;
            for(k=0; k<nD; k++)
               {
                 Roots[j]->Center[k] = Data[k][j];
                 Roots[j]->nSamples = 1;
                 Roots[j]->Level = -1;
               }
            for(k=0; k<nClasses; k++)
                Roots[j]->ClassCounts[k]=0;
            Roots[j]->Class = SampleClasses[i];
            Roots[j]->ClassCounts[Roots[j]->Class]=1;
            j++;
          }
       CalcDistances();
       BestScore = 0;
       NavStackPtr = 0;
       BestnClusters=1<<nD;
       while(nRoots>UntilnClusters)
            {
              MinDistance(I, J);
_ASSERTE(I>=0&&I<nRoots);
              Merge(I, J);
_ASSERTE(I>=0&&I<nRoots);
              if(nRoots<=StartMeasure)
                {
                  Partition();
                  Score=SeparationCallback(nRoots,Clusters,Owner,f);
                  Roots[I]->Separation = Score;
                  //if(nRoots==BestnClusters)
                  //  {
                  //    BestScore=Score;
                  //    break;
                  //  }
                  if(BestScore<Score)
                    {
                      BestScore=Score;
                      BestnClusters = nRoots; // TODO: unfold to BestnClusters
                    }
                }
            }
       BestScore=Score;
       return nRoots;
     }
  void Forward(int &nclusters, Cluster*& clusters, REAL &score)
     {
       int i, j;
       Node *nd;

       if(nRoots == 2)
          return;
       j=NavStack[NavStackPtr].deleted;
       nd = Roots[j];
       --nRoots;
       for(i=j; i<nRoots; i++)
           Roots[i] = Roots[i+1];
       Roots[NavStack[NavStackPtr].r1] = NavStack[NavStackPtr].n11;
       Roots[NavStack[NavStackPtr].r2] = nd;
       ++NavStackPtr;
       Partition();
       nclusters = nRoots;
       clusters = Clusters;
       score = Roots[NavStack[NavStackPtr].r1]->Separation;
     }
  void Backward(int &nclusters, Cluster*& clusters, REAL &score)
     {
       int i, j;

       if(nRoots == StartMeasure)
          return;
       j=NavStack[--NavStackPtr].deleted;
       for(i=nRoots; i>j; i--)
           Roots[i] = Roots[i-1];
       Roots[NavStack[NavStackPtr].r1] = NavStack[NavStackPtr].n10;
       Roots[NavStack[NavStackPtr].r2] = NavStack[NavStackPtr].n20;
       Partition();
       nclusters = ++nRoots;
       clusters = Clusters;
       for(i=1,j=0; j<nRoots; j++)
        if(Roots[i]->Level<Roots[j]->Level)
           i=j;
       score = Roots[i]->Separation;
     }
};
