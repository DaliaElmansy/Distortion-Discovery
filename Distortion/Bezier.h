#pragma once

#include<Windows.h>
#include"Tools.h"

class Bezier
{
protected:
  struct REALPOINT {REAL x, y;} *RealPoints;
  REAL *xKnots, *yKnots, *rhs, *rx, *ry, *tmp,
        xScale, yScale;
  int nKnots, nControls, KnotSize;
public:
  POINT *Points;
  int nPoints;

  Bezier() {Zero();}
 ~Bezier() {Reset();}
  void Zero()
       {
         Points = NULL;
         RealPoints = NULL;
         xKnots = yKnots = rhs = rx = ry = tmp = NULL;
         nPoints = nKnots = nControls = KnotSize = 0;
         xScale = yScale = 1.0;
       }
  void Reset()
       {
         if(Points!=NULL) delete[] Points;
         if(RealPoints!=NULL) delete[] RealPoints;
         if(xKnots!=NULL) delete[] xKnots;
         if(yKnots!=NULL) delete[] yKnots;
         if(rhs!=NULL) delete[] rhs;
         if(rx!=NULL) delete[] rx;
         if(ry!=NULL) delete[] ry;
         if(tmp!=NULL) delete[] tmp;
         Zero();
       }
  BOOL Allocate()
       {
         int n = KnotSize * 3 - 2, nc = KnotSize-1;

         if(NULL==(Points=new POINT[n+3+10]))//+3 to be able tp close a polygon
            return FALSE;
         if(NULL==(RealPoints=new REALPOINT[n+10]))
            return FALSE;
         if(NULL==(xKnots=new REAL[nc+10]))
            return FALSE;
         if(NULL==(yKnots=new REAL[nc+10]))
            return FALSE;
         if(NULL==(rhs=new REAL[nc+10]))
            return FALSE;
         if(NULL==(rx=new REAL[nc+10]))
            return FALSE;
         if(NULL==(ry=new REAL[nc+10]))
            return FALSE;
         if(NULL==(tmp=new REAL[nc+10]))
            return FALSE;
         return TRUE;
       }
  void ApplyScale()
       {
         for(int i=0; i<nPoints; i++)
            {
              Points[i].x = Round(RealPoints[i].x*xScale);
              Points[i].y = Round(RealPoints[i].y*yScale);
            }
       }
  void ApplyXScale(int X0, REAL xs=-1)
       {
         if(xs>0 && xScale!=xs)
           {
             xScale=xs;
             for(int i=0; i<nPoints; i++)
                 Points[i].x = Round((RealPoints[i].x-X0)*xScale);
           }
       }
  void ApplyYScale(int Y0, REAL ys=-1)
       {
         if(ys>0 && yScale!=ys)
           {
             yScale=ys;
             for(int i=0; i<nPoints; i++)
                 Points[i].y = Round(Y0-(Y0-RealPoints[i].y)*yScale);
           }
       }
  BOOL Init(int nknots)
       {
         int i, j;

         if(nknots<2)
            return FALSE;
         nKnots = nknots;
         if(KnotSize < nKnots)
           {
             Reset();
             KnotSize = nKnots = nknots;
             if(!Allocate())
                return FALSE;
           }
         nControls = nKnots - 1;
         nPoints = nKnots * 3 - 2;
         return TRUE;
       }
  BOOL Init(int nknots, REAL *xknots, REAL *yknots,
            REAL xscale=1, REAL yscale=1)
       {
         int i, j;

         if(  xscale<=0||yscale<=0||xknots==0||yknots==0
            ||!Init(nknots) 
           )
            return FALSE;
         xScale = xscale;
         yScale = yscale;
         nControls = nKnots - 1;
         nPoints = nKnots * 3 - 2;
         for(i=0; i<nKnots; i++)
             xKnots[i] = xknots[i];
         for(i=0; i<nKnots; i++)
             yKnots[i] = yknots[i];
         for(i=0; i<nKnots; i++)
            {
              int i3 = i*3;
              RealPoints[i3].x = xKnots[i];
              RealPoints[i3].y = yKnots[i];
            }
         if(nControls==1)
           {
             RealPoints[1].x=(2*RealPoints[0].x+RealPoints[3].x)/3.0;
             RealPoints[1].y=(2*RealPoints[0].y+RealPoints[3].y)/3.0;
             RealPoints[2].x= 2*RealPoints[1].x-RealPoints[0].x;
             RealPoints[2].y= 2*RealPoints[1].y-RealPoints[0].y;
             ApplyScale();
             return TRUE;
           }
         for(i = 1; i < nControls-1; ++i)
             rhs[i] = 4*xKnots[i] + 2*xKnots[i+1];
         rhs[0]=xknots[0] + 2*xknots[1];
         rhs[i]=(8*xknots[i] + xknots[nControls])/2.0;
         CalcXY(rx);
         for(i = 1; i < nControls-1; ++i)
             rhs[i] = 4*yKnots[i] + 2*yKnots[i+1];
         rhs[0]=yknots[0] + 2*yknots[1];
         rhs[i]=(8*yknots[i] + yknots[nControls])/2.0;
         CalcXY(ry);
         for(i=0; i<nControls; ++i)
            {
              j=i*3+1; // index of First control point
              RealPoints[j].x=rx[i];
              RealPoints[j].y=ry[i];
              j++; // index of Second control point
              if(i<nControls-1)
                {
                  RealPoints[j].x=2*xKnots[i+1]-rx[i+1];
                  RealPoints[j].y=2*yKnots[i+1]-ry[i+1];
                }
              else
                {
                  RealPoints[j].x=(xKnots[nControls]+rx[nControls-1])/2.0;
                  RealPoints[j].y=(yKnots[nControls]+ry[nControls-1])/2.0;
                }
            }
         ApplyScale();
         return TRUE;
       }
  void CalcXY(REAL *xy)
       {
         int i;
         REAL b = 2.0;

         xy[0] = rhs[0]/b;
         for(i=1; i<nControls; i++) // Decomposition and forward substitution.
            {
              tmp[i] = 1/b;
              b = (i<nControls-1? 4.0:3.5)-tmp[i];
              xy[i] = (rhs[i] - xy[i-1]) / b;
            }
         for(i=1; i<nControls; i++)
             xy[nControls-i-1] -= tmp[nControls-i]*xy[nControls-i]; // Backsubstitution.
       }
};
