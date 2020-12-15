#include "stdafx.h"

#define GRAPHHIEGHT 375
#define GRAPHWIDTH 350
#define PLOTSIZE 300

unsigned int WINAPI PermutationTestingThread(void *);
void DisplayPermutationTestingParams(int =0);
void EbablePermutationTestingDlgItems(BOOL);
BOOL CollectPermutationTestingParams(int =0);
BOOL PermutationTestingParamsChanged(int =0);
void SavePermutationTestingChanges(BOOL =FALSE);
void SavePermutationTestingParams();
void LoadPermutationTesting();
void PaintPermutationTesting(HDC);
void ApplyYScale();

BOOL PermutationTestingLoaded,
     PermutationTestingNow=FALSE, permtstd=FALSE;
Bezier bzrAct[3], bzrPerm[3], bzrPVal[3],
       bzrMinPerm[3], bzrMaxPerm[3];
POINT MinMaxPermFreq[3][((MAXFREQ*3)-2)<<1];
REAL xActKnots[MAXFREQ],  yActKnots[MAXFREQ],
     xPermKnots[MAXFREQ], yPermKnots[MAXFREQ],
     xPValKnots[MAXFREQ], yPValKnots[MAXFREQ],
     xMinPermKnots[MAXFREQ], yMinPermKnots[MAXFREQ],
     xMaxPermKnots[MAXFREQ], yMaxPermKnots[MAXFREQ],
     Sig2Score[3], Sig5Score[3],
     YScales[10]={1.0,1.11,1.25,1.43,1.66,2.0,
                  2.5,3.333,5.0,10.0};
int YScale;
HRGN hRgnSig=NULL;
HANDLE hPermutationTestingThread=0;
RECT rctSig, /*rctTop, rctTopPlot,*/ rctSigPlot,
     *RectsPermT[]={&rctSig/*, &rctTop*/},
     *RectsPlotPermT[]={&rctSigPlot/*, &rctTopPlot*/};
//Dataset::_FocusOn FocusOn=Dataset::Both;

// Message handler for Permutation box.
INT_PTR CALLBACK PermutationTestingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent, zDelta=0;
  char Buf[MAX_PATH+1];
  PAINTSTRUCT ps;
  HDC hdc;
  REAL x=0;
  BOOL Shift=0, Ctrl=0;
  UINT ThreadID=0;
  NMUPDOWN *mnud=0;
  
  switch(message)
        {
	  case WM_INITDIALOG:
               hCurrentCursor=hWaitCursor;
               SetCursor(hWaitCursor);
               YScale=0;
               ds->LoadParamFile("PermutationTestingParameters.txt");
               LoadPermutationTesting();
               CurrentDlg=hDlg;
               DisplayPermutationTestingParams();
               EditProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg,IDC_DATASET_NAME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
               SetWindowLongPtr(GetDlgItem(hDlg,IDC_ELAPSED_TIME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
               SetWindowLongPtr(GetDlgItem(hDlg,IDC_REMAINING_TIME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
               SetWindowLongPtr(GetDlgItem(hDlg,IDC_PROGRESS_PERCENT), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
               SetWindowLongPtr(GetDlgItem(hDlg,IDC_STATUS), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
               if(permtstd)
                 {
                   ApplyYScale();
                   hCurrentCursor=hArrowCursor;
                   SetCursor(hCurrentCursor);
                   return 0;
                 }
               changed = permtstd = FALSE;
               GetClientRect(hDlg, &rctSig);
               rctSig.right-=5;
               rctSig.left=rctSig.right-GRAPHWIDTH+1;
               rctSig.top=3;
               rctSig.bottom=rctSig.top+GRAPHHIEGHT+1;
               rctSigPlot.top=rctSig.top+33;
               rctSigPlot.right=rctSig.right-5;
               rctSigPlot.bottom=rctSigPlot.top+PLOTSIZE;
               rctSigPlot.left=rctSigPlot.right-PLOTSIZE;
               if(hRgnSig==NULL)
                  hRgnSig=CreateRectRgn(rctSigPlot.left, rctSigPlot.top,
                                        rctSigPlot.right,  rctSigPlot.bottom);
               PermutationTestingNow=Aborted=FALSE;
               if(PermutationTestingLoaded)
                 {
                   CalcSig();
                   InvalidateRect(hDlg, 0, 0);
                   permtstd=TRUE;
                   PermutationTestingLoaded=FALSE;
                 }
               hCurrentCursor=hArrowCursor;
               SetCursor(hCurrentCursor);
               return 0;
	  case WM_SETCURSOR:
               if(hCurrentCursor!=0)
                  SetCursor(hCurrentCursor);
               return TRUE;
          case WM_COMMAND:
               wmId    = LOWORD(wParam);
               wmEvent = HIWORD(wParam);
               switch(wmId)
                     {
                       case IDC_APPLY:
                            EnableWindow(GetDlgItem(hDlg, ID_CLOSE), 0);
                            EnableWindow(GetDlgItem(hDlg, IDC_APPLY), 0);
                            EbablePermutationTestingDlgItems(0);
                            hCurrentCursor=hWaitCursor;
                            crsr=SetCursor(hWaitCursor);
                            SetDlgItemText(hDlg, IDCANCEL, "Abort");
                            PermutationTestingNow=TRUE;
                            InvalidateRect(hDlg, 0, 0);
                            SavePermutationTestingChanges(TRUE);
                            LowLight(hDlg, RectsPlotPermT, CountOf(RectsPlotPermT));
                            GetDumps(RectsPermT, CountOf(RectsPermT));
                            hPermutationTestingThread=(HANDLE)_beginthreadex(0, 0,
                              (ThreadAPI)PermutationTestingThread,
                               0, 0, &ThreadID);
                            return 0;
                       case IDC_FOCUS_ON_BOTH:
                       case IDC_FOCUS_ON_FN:
                       case IDC_FOCUS_ON_FP:
                            if(wmEvent==BN_CLICKED)
                            if(PermutationTestingParamsChanged(wmId))
                               InvalidateRect(hDlg,0,0);
                            return 0;
                       case IDC_N_SHUFFLES:
                       case IDC_MIN_MIS:
                       case IDC_MAX_MIS_RATIO:
                       case IDC_MAX_MIS_TOTAL:
                            if(wmEvent==EN_KILLFOCUS)
                               PermutationTestingParamsChanged(wmId);
                            return 0;
                       case IDC_PRINT_ROC:
                            if(DialogBox(hInst, MAKEINTRESOURCE(IDD_SCALE), hDlg, ScaleProc))
                               SaveRectToFile(rctSig, "Significance.png", DrawSig);
                            return 0;
                       case IDC_PLUS:
                            if(YScale<CountOf(YScales)-1)
                              {
                                YScale++;
                                ApplyYScale();
                              }
                            return 0;
                       case IDC_MINUS:
                            if(YScale>0)
                              {
                                YScale--;
                                ApplyYScale();
                              }
                            return 0;
                       case ID_CLOSE:
                            //SavePermutationTestingChanges();
                            EndDialog(hDlg, 0);
                            return 0;
                       case IDCANCEL:
                            if(PermutationTestingNow)
                              {
                                SetDlgItemText(hDlg, IDCANCEL, "Cancel");
                                ds->Abort();
                                return 0;
                              }
                            //SavePermutationTestingChanges();
                            EndDialog(hDlg, 0);
                            return 0;
                     }
               break;
	  case WM_PAINT:
               hdc = BeginPaint(hDlg, &ps);
               PaintPermutationTesting(hdc);
               EndPaint(hDlg, &ps);
               return 0;
	}
  return (INT_PTR)FALSE;
}

unsigned int WINAPI PermutationTestingThread(void *param)
{
  ProgressSize=ds->nShuffles+1;
  InitProgress();
  Status("Testing...");
  //ds->FocusOn=Dataset::Both;
  Aborted = !(ds->PermutationTesting(Progress));
  if(Aborted)
    {
      EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDCANCEL), 1);
      EbablePermutationTestingDlgItems(1);
      Aborted=FALSE;
      if(permtstd)
        {
          LoadPermutationTesting();
          if(PermutationTestingLoaded)
            {
              CalcSig();
              InvalidateRect(CurrentDlg, 0, 0);
              permtstd=TRUE;
              PermutationTestingLoaded=FALSE;
            }
        }
      SendMessage(GetDlgItem(CurrentDlg, IDC_PROGRESS_BAR), PBM_SETPOS, (WPARAM) 0, 0);
      SetDlgItemText(CurrentDlg, IDC_PROGRESS_PERCENT,"");
      SetDlgItemText(CurrentDlg, IDC_ELAPSED_TIME,"");
      SetDlgItemText(CurrentDlg, IDC_REMAINING_TIME,"");
    }
  else
    {
      CalcSig();
      //UpdateWindow(hDlg);
      permtstd=TRUE;
      ds->SavePermutationTesting();
      ds->SavePermutationTestingParamFile("PermutationTestingParameters.txt");
      EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      EbablePermutationTestingDlgItems(1);
      SetDlgItemText(CurrentDlg, IDCANCEL, "Cancel");
    }
  Status("");
  PermutationTestingNow=FALSE;
  hCurrentCursor=crsr;
  //SetCursor(crsr);
  SendMessage(CurrentDlg, WM_SETCURSOR, 0,0);
  InvalidateRect(CurrentDlg, 0, 0);
  CloseHandle(hPermutationTestingThread);
  _endthreadex(0);
  return 0;
}

void DisplayPermutationTestingParams(int Item)
{
  char Buf[64];
  static int IDC[3]={IDC_FOCUS_ON_BOTH,
                     IDC_FOCUS_ON_FP,
                     IDC_FOCUS_ON_FN
                    };

  if(Item==IDC_DATASET_NAME||Item==0)
      SetDlgItemText(CurrentDlg, IDC_DATASET_NAME, DatasetName);
  if(Item==IDC_N_SHUFFLES||Item==0)
      SetDlgItemInt(CurrentDlg, IDC_N_SHUFFLES, ds->nShuffles, 0);
  if(Item==IDC_MIN_MIS||Item==0)
     SetDlgItemInt(CurrentDlg, IDC_MIN_MIS, ds->MinMis, 0);
  if(Item==IDC_MAX_MIS_RATIO||Item==0)
   {
     sprintf_s(Buf, "%g", ds->MaxMisRatio);
     SetDlgItemText(CurrentDlg, IDC_MAX_MIS_RATIO, Buf);
   }
  if(Item==IDC_MAX_MIS_TOTAL||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MaxMisTotal);
      SetDlgItemText(CurrentDlg, IDC_MAX_MIS_TOTAL, Buf);
    }
  if(Item==IDC_MIN_SPOTTED||Item==0)
     SetDlgItemInt(CurrentDlg, IDC_MIN_SPOTTED, ds->MinSpotted, 0);
  if(Item==IDC_MIN_SPOTTING_RATE||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinSpottingRate);
      SetDlgItemText(CurrentDlg, IDC_MIN_SPOTTING_RATE, Buf);
    }
  if(Item==IDC_FOCUS_ON_FP||IDC_FOCUS_ON_FN||IDC_FOCUS_ON_BOTH||Item==0)
     CheckRadioButton(CurrentDlg, IDC_FOCUS_ON_FP, IDC_FOCUS_ON_BOTH, IDC[FocusOn]);
}

void EbablePermutationTestingDlgItems(BOOL e)
{
  EnableWindow(GetDlgItem(CurrentDlg, IDC_N_SHUFFLES), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_MIS), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_FOCUS_ON_FP), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_FOCUS_ON_FN), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_FOCUS_ON_BOTH), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MAX_MIS_RATIO), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MAX_MIS_TOTAL), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_SPOTTED), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_SPOTTING_RATE), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_PLUS), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MINUS), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_PRINT_ROC), e);
}

BOOL CollectPermutationTestingParams(int Item)
{
  char Buf[64];
  BOOL ch=FALSE;
  int i;
  REAL x;

  if(Item==IDC_N_SHUFFLES || Item==0)
    {
      i=GetDlgItemInt(CurrentDlg, IDC_N_SHUFFLES,NULL,1);
      if(i!=ds->nShuffles)
        {
          ds->nShuffles=i;
          ch=changed=TRUE;
        }
      if(Item!=0) return ch;
    }

  if(Item==IDC_MIN_MIS || Item==0)
    {
      i=GetDlgItemInt(CurrentDlg, IDC_MIN_MIS,NULL,1);
      if(i!=ds->MinMis)
        {
          ds->MinMis=i;
          ch=changed=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MAX_MIS_RATIO || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MAX_MIS_RATIO, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MaxMisRatio)
        {
          ds->MaxMisRatio=x;
          ch=changed=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MAX_MIS_TOTAL || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MAX_MIS_TOTAL, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MaxMisTotal)
        {
          ds->MaxMisTotal=x;
          ch=changed=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_SPOTTED || Item==0)
    {
      i=GetDlgItemInt(CurrentDlg, IDC_MIN_SPOTTED,NULL,1);
      if(i!=ds->MinSpotted)
        {
          ds->MinSpotted=i;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_SPOTTING_RATE || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_SPOTTING_RATE, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinSpottingRate)
        {
          ds->MinSpottingRate=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  return ch;
}

BOOL PermutationTestingParamsChanged(int Item)
{
  char Buf[64];
  BOOL success, ch=FALSE;
  int i;
  REAL x;

  if(Item==IDC_N_SHUFFLES|| Item==0)
    {
      if(ds->nShuffles!=GetDlgItemInt(CurrentDlg, IDC_N_SHUFFLES,&success,1))
        {
          ch=changed=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  if(Item==IDC_MIN_MIS|| Item==0)
    {
      if(ds->MinMis!=GetDlgItemInt(CurrentDlg, IDC_MIN_MIS,&success,1))
        {
          ch=changed=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  if(Item==IDC_MAX_MIS_RATIO|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MAX_MIS_RATIO, Buf, sizeof(Buf));
      if(ds->MaxMisRatio!=atof(Buf))
        {
          ch=changed=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  if(Item==IDC_MAX_MIS_TOTAL|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MAX_MIS_TOTAL, Buf, sizeof(Buf));
      if(ds->MaxMisTotal!=atof(Buf))
        {
          ch=changed=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  if(Item==IDC_MIN_SPOTTED|| Item==0)
    {
      if(ds->MinSpotted!=GetDlgItemInt(CurrentDlg, IDC_MIN_SPOTTED,&success,1))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_SPOTTING_RATE|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_SPOTTING_RATE, Buf, sizeof(Buf));
      if(ds->MinSpottingRate!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_FOCUS_ON_FP||Item==IDC_FOCUS_ON_FN||Item==IDC_FOCUS_ON_BOTH||Item==0)
    {
      if(BST_CHECKED==IsDlgButtonChecked(CurrentDlg, IDC_FOCUS_ON_FP))
        {
          if(FocusOn!=Dataset::FP)
            {
              FocusOn=Dataset::FP;
              ch=TRUE;
              if(Item!=0)
                 return ch;
            }
        }
      else if(BST_CHECKED==IsDlgButtonChecked(CurrentDlg, IDC_FOCUS_ON_FN))
        {
          if(FocusOn!=Dataset::FN)
            {
              FocusOn=Dataset::FN;
              ch=TRUE;
              if(Item!=0)
                 return ch;
            }
        }
      else if(FocusOn!=Dataset::Both)
            {
              FocusOn=Dataset::Both;
              ch=TRUE;
              if(Item!=0)
                 return ch;
            }
    }
  return FALSE;
}

void LoadPermutationTesting()
{
  PermutationTestingLoaded=ds->LoadPermutationTesting();
}
// Permutation Testing
void PermutationTesting()
{
  crsr=hCurrentCursor=hWaitCursor;
  SetCursor(hCurrentCursor);
  permtstd = FALSE;
  DialogBox(hInst, MAKEINTRESOURCE(IDD_PERMUTATION_TESTING), hMainWnd, PermutationTestingProc);
  ds->Permuting=false;
  hCurrentCursor=hArrowCursor;
  SetCursor(hCurrentCursor);
}
void SavePermutationTestingChanges(BOOL mndtory)
{
  if(changed &&
     (mndtory||IDYES==MessageBox(CurrentDlg, "Save Changes?", "Parameters Changed", MB_YESNO|MB_ICONQUESTION))
    )
    {
      CollectPermutationTestingParams();
      SavePermutationTestingParams();
    }
}
void SavePermutationTestingParams()
{
  ds->SavePermutationTestingParamFile("PermutationTestingParameters.txt");
  changed=FALSE;
}

void CalcSig()
{
  int i, d, k;
  REAL Step=(REAL)PLOTSIZE/(MAXFREQ-1), x,
       stp=1.0/MAXFREQ;

  nKnots = MAXFREQ;
  nPoints=nKnots*3-2;
  
  for(d=0; d<3; d++)
     {
       for(i=0; i<MAXFREQ; i++)
          {
            xActKnots[i]=xPermKnots[i]=xPValKnots[i]=
            xMinPermKnots[i]=xMaxPermKnots[i]=
                      rctSigPlot.left+Round(i*Step);
            yActKnots[i]=rctSigPlot.bottom-Round(ds->CumulativeFreqActual[d][i]*PLOTSIZE);
            yPermKnots[i]=rctSigPlot.bottom-Round(ds->DistFreqAve[d][i]*PLOTSIZE);
            yPValKnots[i]=rctSigPlot.bottom-Round(ds->pValues[d][i]*PLOTSIZE);
            yMinPermKnots[i]=rctSigPlot.bottom-Round(ds->MinCumFreqPerm[d][i]*PLOTSIZE);
            yMaxPermKnots[i]=rctSigPlot.bottom-Round(ds->MaxCumFreqPerm[d][i]*PLOTSIZE);
          }
       bzrAct[d].Init(nKnots, xActKnots, yActKnots);
       bzrPerm[d].Init(nKnots, xPermKnots, yPermKnots);
       bzrPVal[d].Init(nKnots, xPValKnots, yPValKnots);
       bzrMinPerm[d].Init(nKnots, xMinPermKnots, yMinPermKnots);
       bzrMaxPerm[d].Init(nKnots, xMaxPermKnots, yMaxPermKnots);
       for(i=0; i<nPoints; i++)
           MinMaxPermFreq[d][i]=bzrMinPerm[d].Points[i];
       for(; i<nPoints<<1; i++)
           MinMaxPermFreq[d][i]=bzrMaxPerm[d].Points[(nPoints<<1)-1-i];
       POINT *p=bzrPVal[d].Points+bzrPVal[d].nPoints;
       p[0].x=rctSigPlot.right;
       p[0].y=rctSigPlot.bottom;
       p[1].x=rctSigPlot.left;
       p[1].y=rctSigPlot.bottom;
       p[2].x=rctSigPlot.left;
       p[2].y=yPValKnots[0];
       Sig2Score[d]=Sig5Score[d]=1.0;
       for(i=MAXFREQ-1,x=1; i>=0; i++, x-=stp)
        if(ds->pValues[d][i]>=0.02)
           break;
       Sig2Score[d]=x;
       for(; i>=0; i++, x-=stp)
        if(ds->pValues[d][i]>=0.05)
           break;
       Sig5Score[d]=x;
     }
  ApplyYScale();
}

void ApplyYScale()
{
  int i;

  for(int d=0; d<3; d++)
     {
       bzrAct[d]. ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       bzrPerm[d].ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       bzrPVal[d].ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       bzrAct[d]. ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       bzrPerm[d].ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       bzrPVal[d].ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       bzrMinPerm[d].ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       bzrMaxPerm[d].ApplyYScale(rctSigPlot.bottom, YScales[YScale]);
       for(i=0; i<nPoints; i++)
           MinMaxPermFreq[d][i]=bzrMinPerm[d].Points[i];
       for(; i<nPoints<<1; i++)
           MinMaxPermFreq[d][i]=bzrMaxPerm[d].Points[(nPoints<<1)-1-i];
     }
  InvalidateRect(CurrentDlg, 0, 0);
}

void DrawSig(HDC hdc, REAL scale)
{
  int i, j;
  BOOL Screen=scale<0;
  REAL Scale=Screen?1.0:scale;
  int _20=20*Scale, _2=2*Scale, _3=3*Scale,
      nPoints=bzrAct->nPoints, Ink=10.01+Scale*178;
  REAL x1, y1, x2, y2;
  RECT rctSig_Only={0,0,GRAPHWIDTH*Scale+1,GRAPHHIEGHT*Scale+1},
       rctSigPlotOnly={(GRAPHWIDTH-PLOTSIZE-6)*Scale, 33*Scale,
                       (GRAPHWIDTH-6)*Scale,(PLOTSIZE+33)*Scale},
       rctSigLegend={rctSigPlotOnly.left-23*Scale,
                     rctSigPlotOnly.top-5*Scale,
                     rctSigPlotOnly.right+3*Scale,
                     rctSigPlotOnly.bottom+23*Scale};
  char *lgnd[]={"0","0.1","0.2","0.3","0.4","0.5",
                "0.6","0.7","0.8","0.9","1"},
       Buf[64];
  POINT Points[3][(MAXROCSTEPS+1)*3+1];

  if(!permtstd)
      return;

  SetGraphicsMode(hdc, GM_ADVANCED);
  SetMapMode(hdc, MM_TEXT);
  
  //ScaleRect(rctSig_Only, Scale);
  //ScaleRect(rctSigPlotOnly, Scale);
  //ScaleRect(rctSigLegend, Scale);

  for(i=0; i<3; i++)
  for(j=0; j<nPoints; j++)
     {
       Points[i][j].x=(bzrAct[i].Points[j].x-rctSigPlot.left)*Scale;
       Points[i][j].x+=rctSigPlotOnly.left;
       Points[i][j].y=(bzrAct[i].Points[j].y-rctSigPlot.top)*Scale;
       Points[i][j].y+=rctSigPlotOnly.top;
     }

  if(Ink<100)
     Ink=100;
  if(Ink>900)
     Ink=900;

  HFONT hNormalFont = CreateFont(14*Scale, 0, 0, 0, Scale>=1?FW_NORMAL:(Scale>=.5?FW_LIGHT:FW_EXTRALIGHT), 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");
  HFONT hScaleFont = CreateFont(14*Scale, 0, 0, 0, Scale>=1?FW_NORMAL:(Scale>=.5?FW_LIGHT:FW_EXTRALIGHT), 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");
  HFONT hTitleFont = CreateFont(24*Scale, 0, 0, 0, Scale>=2.0?FW_HEAVY:(Scale>=1?FW_BOLD:FW_NORMAL), 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");
  HFONT hHLegendFont = CreateFont(18*Scale, 0, 0, 0, Ink, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");
  HFONT hVLegendFont = CreateFont(18*Scale, 0, 900, 900, Ink, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");

  hRgnROC_Only=CreateRectRgn(rctSigPlotOnly.left-2, rctSigPlotOnly.top-2,
                      rctSigPlotOnly.right+2,  rctSigPlotOnly.bottom);

  FillRect(hdc, &rctSig_Only, hWhiteBrush);
  SelectObject(hdc, Scale>=1?hBlackPen2:hBlackPen);
  MoveToEx(hdc, rctSig_Only.left+1, rctSig_Only.top+1, 0);
  LineTo(hdc, rctSig_Only.right-2, rctSig_Only.top+1);
  LineTo(hdc, rctSig_Only.right-2, rctSig_Only.bottom-2);
  LineTo(hdc, rctSig_Only.left+1, rctSig_Only.bottom-2);
  LineTo(hdc, rctSig_Only.left+1, rctSig_Only.top-1);
  FillRect(hdc, &rctSigLegend, hLtLtLtGreyBrush);//hLegendBkBrush);
  FillRect(hdc, &rctSigPlotOnly, hWhiteBrush);//hPlotBkBrush);
  SelectObject(hdc, hGridMinorPen);
  x1=rctSigPlotOnly.left-_3;
  x2=rctSigPlotOnly.right;
  for(i=1; i<10; i++)
     {
       y1=rctSigPlotOnly.bottom-i/10.0*PLOTSIZE*Scale;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }
  y1=rctSigPlotOnly.top;
  y2=rctSigPlotOnly.bottom;
  for(i=0; i<10; i++)
     {
       x1=rctSigPlotOnly.left+i/10.0*PLOTSIZE*Scale;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  SetBkMode(hdc, TRANSPARENT);
  SelectObject(hdc, hGridMajorPen);
  SetTextAlign(hdc, TA_RIGHT|TA_BASELINE|TA_NOUPDATECP);
  SetTextColor(hdc, 0);
  SelectObject(hdc, hScaleFont);

  x1=rctSigPlotOnly.left-_3;
  x2=rctSigPlotOnly.right;
  for(i=0; i<=10; i++,i++)
     {
       y1=rctSigPlotOnly.bottom-i/10.0*PLOTSIZE*Scale;
       if(i>0&&i<10)
         {
           MoveToEx(hdc, x1, y1, 0);
           LineTo(hdc, x2, y1);
         }
       TextOut(hdc, x1-Scale, y1+5*Scale, lgnd[i], i==0||i==10?1:3);
     }
  SetTextAlign(hdc, TA_CENTER|TA_TOP|TA_NOUPDATECP);
  y1=rctSigPlotOnly.top;
  y2=rctSigPlotOnly.bottom+_3;
  for(i=0; i<=10; i++,i++)
     {
       x1=rctSigPlotOnly.left+i/10.0*PLOTSIZE*Scale;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
       TextOut(hdc, x1, y2+Scale, lgnd[i], i==0||i==10?1:3);
     }
  SelectObject(hdc, hBlackPen);
  MoveToEx(hdc, rctSigPlotOnly.left,  rctSigPlotOnly.top, 0);
  LineTo(hdc,   rctSigPlotOnly.left,  rctSigPlotOnly.bottom);
  LineTo(hdc,   rctSigPlotOnly.right, rctSigPlotOnly.bottom);
  LineTo(hdc,   rctSigPlotOnly.right, rctSigPlotOnly.top);
  LineTo(hdc,   rctSigPlotOnly.left,  rctSigPlotOnly.top);
  
  SelectClipRgn(hdc, hRgnROC_Only);
  SelectObject(hdc, hNormalFont);
  SelectObject(hdc, hGreyPlotPens[int(0.0001+Scale)]);
  PolyBezier(hdc, Points[2], nPoints);
  x1=rctSigPlotOnly.right-133*Scale;
  x2=x1+20*Scale;
  y1=rctSigPlotOnly.bottom-30*Scale+20*Scale;
  MoveToEx(hdc, x1, y1, 0);
  LineTo(hdc, x2, y1);
  sprintf_s(Buf, "Not Distorted: AUC=%4.2f",ds->CombinedNoDist.AUC);
  SetTextColor(hdc, cDkGreyColor);
  SetTextAlign(hdc, TA_LEFT|VTA_TOP);
  TextOut(hdc, x2+5*Scale, y1-7*Scale, Buf, 18);
  SelectObject(hdc, hRedPlotPens[1+int(0.0001+Scale)]);
  PolyBezier(hdc, Points[0], nPoints);
  y1-=20*Scale;
  MoveToEx(hdc, x1, y1, 0);
  LineTo(hdc, x2, y1);
  sprintf_s(Buf, "Before Corr: AUC=%4.2f",ds->CombinedDist[FocusOn][0].AUC);
  SetTextColor(hdc, cDkRedColor);
  TextOut(hdc, x2+5*Scale, y1-7*Scale, Buf, 21);
  SelectObject(hdc, hBluePlotPens[1+int(0.0001+Scale)]);
  PolyBezier(hdc, Points[1], nPoints);
  y1+=10*Scale;
  MoveToEx(hdc, x1, y1, 0);
  LineTo(hdc, x2, y1);
  sprintf_s(Buf, "After Corr: AUC=%4.2f",ds->CombinedDist[FocusOn][1].AUC);
  SetTextColor(hdc, cDkBlueColor);
  TextOut(hdc, x2+5*Scale, y1-7*Scale, Buf, 20);
  SelectClipRgn(hdc, 0);
  SelectObject(hdc, hHLegendFont);
  SetTextColor(hdc, 0);
  SetTextAlign(hdc, TA_CENTER|TA_BASELINE|TA_NOUPDATECP);
  x1=(rctSigPlotOnly.left+rctSigPlotOnly.right)>>1;
  y1=((rctSigLegend.bottom+rctSig_Only.bottom)>>1)+_3+1;
  sprintf_s(Buf, "Specificity");
  TextOut(hdc, x1, y1, Buf, 11);
  SelectObject(hdc, hVLegendFont);
  x1=((rctSigLegend.left+rctSig_Only.left)>>1)+5*Scale+1;
  y1=(rctSigPlotOnly.top+rctSigPlotOnly.bottom)>>1;
  sprintf_s(Buf, "Sensitivity");
  TextOut(hdc, x1, y1, Buf, 11);
  SetTextColor(hdc, cBlackColor);
  SelectObject(hdc, hTitleFont);
  x1=(rctSig_Only.left+rctSig_Only.right)>>1;
  y1=((rctSig_Only.top+rctSigLegend.top)>>1)+8*Scale+1;
  TextOut(hdc, x1, y1, "ROC Curves", 10);
  DeleteObject(hRgnROC_Only);
}
// Paint Permutation Testing
void PaintPermutationTesting(HDC hdc)
{
  int i, j, dot=2;
  REAL x1, y1, x2, y2;
  RECT rctSigLegend=rctSigPlot;//, rctTopLegend=rctTopPlot;
  REAL Minors[]={10.0,10.0,10.0,10.0,10.0,20.0,20.0,50.0,50.0,100.0};
  char *lgnd[]={"0","0.1","0.2","0.3","0.4","0.5",
                "0.6","0.7","0.8","0.9","1"},
       Buf[64];

  if(PermutationTestingNow)
    {
      PutDumps(hdc, RectsPermT, CountOf(RectsPermT));
      return;
    }

  SetGraphicsMode(hdc, GM_ADVANCED);
  SetMapMode(hdc, MM_TEXT);

  rctSigLegend.top-=5;
  rctSigLegend.bottom+=21;
  rctSigLegend.left-=23;
  rctSigLegend.right+=1;
  SelectObject(hdc, hDkGreyPen);
  SelectObject(hdc, hLtLtGreyBrush);
  Rectangle(hdc, rctSig.left, rctSig.top, rctSig.right, rctSig.bottom);
  FillRect(hdc, &rctSigLegend, hLegendBkBrush);
  FillRect(hdc, &rctSigPlot, hWhiteBrush);//hPlotBkBrush);
  if(permtstd)
    {
      SelectClipRgn(hdc, hRgnSig);
      SelectObject(hdc, hLtLtGreyBrush);
      SelectObject(hdc, hLtGreyPen);
      Polygon(hdc, bzrPVal[FocusOn].Points, nPoints+3);
      HBITMAP hbmp1=CreateCompatibleBitmap(hdc, PLOTSIZE, PLOTSIZE);
      HDC hdc1=CreateCompatibleDC(hdc);
      SelectObject(hdc1, hbmp1);
      SetViewportOrgEx(hdc1, -rctSigPlot.left, -rctSigPlot.top, 0);
      FillRect(hdc1, &rctSigPlot, hWhiteBrush);
      SelectObject(hdc1, hLtLtRedBrush);
      SelectObject(hdc1, hLtRedPen);
      Polygon(hdc1, MinMaxPermFreq[FocusOn], nPoints<<1);
      BitBlt(hdc, rctSigPlot.left, rctSigPlot.top,
                  PLOTSIZE, PLOTSIZE,
             hdc1, rctSigPlot.left, rctSigPlot.top, SRCAND);
      DeleteDC(hdc1);
      DeleteObject(hbmp1);
      SelectClipRgn(hdc, 0);
    }
  SelectObject(hdc, hGridMinorPen);
  x1=rctSigPlot.left-3;
  x2=rctSigPlot.right;
  for(i=1; i<100; i++)
     {
       y1=Round(rctSigPlot.bottom-i*PLOTSIZE*YScales[YScale]/Minors[YScale]);
       if(y1==rctSigPlot.top-1||y1==rctSigPlot.top+1)
          y1=rctSigPlot.top;
       if(y1<rctSigPlot.top)
          break;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }
  y1=rctSigPlot.top;
  y2=rctSigPlot.bottom;
  for(i=0; i<10; i++)
     {
       x1=rctSigPlot.left+i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  SetBkMode(hdc, TRANSPARENT);
  SelectObject(hdc, hGridMajorPen);
  SetTextAlign(hdc, TA_RIGHT|TA_BASELINE);
  SetTextColor(hdc, cBlackColor);
  SelectObject(hdc, (HFONT)SendMessage(CurrentDlg, WM_GETFONT,0,0));

  x1=rctSigPlot.left-4;
  x2=rctSigPlot.right+1;
  for(i=0; i<=10; i++)
     {
       y1=rctSigPlot.bottom-i/10.0*YScales[YScale]*PLOTSIZE;
       if(y1<rctSigPlot.top)
          break;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
       TextOut(hdc, x1, y1+4, lgnd[i], i==0||i==10?1:3);
       if(YScale<5)
          i++;
     }
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  y1=rctSigPlot.top;
  y2=rctSigPlot.bottom+3;
  for(i=0; i<=10; i++,i++)
     {
       x1=rctSigPlot.left+i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
       TextOut(hdc, x1, y2, lgnd[i], i==0||i==10?1:3);
     }

if(permtstd)
  {
    SelectClipRgn(hdc, hRgnSig);
    SetTextAlign(hdc, TA_LEFT|VTA_TOP);
    x1=rctSigPlot.right-71;
    x2=x1+20;
    y2=rctSigPlot.top-5;

    SelectObject(hdc, hRedPlotPen2);
    PolyBezier(hdc, bzrPerm[FocusOn].Points, nPoints);
    y1=y2+30;
    MoveToEx(hdc, x1, y1, 0);
    LineTo(hdc, x2, y1);
    sprintf_s(Buf, "Permuted");
    SetTextColor(hdc, cDkRedColor);
    TextOut(hdc, x2+5, y1-7, Buf, 8);

    SelectObject(hdc, hBluePlotPen2);
    PolyBezier(hdc, bzrAct[FocusOn].Points, nPoints);
    y1=y2+15;
    MoveToEx(hdc, x1, y1, 0);
    LineTo(hdc, x2, y1);
    sprintf_s(Buf, "Actual");
    SetTextColor(hdc, cDkBlueColor);
    TextOut(hdc, x2+5, y1-7, Buf, 6);

    SelectObject(hdc, hGreyPlotPen0);
    //PolyBezier(hdc, bzrPVal[FocusOn].Points, nBzr);
    y1=y2+45;
    SelectObject(hdc, hGreyPlotPen2);
    MoveToEx(hdc, x1, y1, 0);
    LineTo(hdc, x2, y1);
    sprintf_s(Buf, "p-value");
    SetTextColor(hdc, cDkGreyColor);
    TextOut(hdc, x2+5, y1-7, Buf, 7);

    SelectClipRgn(hdc, 0);
  }
  SelectObject(hdc, hHighFont);
  SetTextColor(hdc, 0);
  x1=rctSigPlot.left+(PLOTSIZE>>1);
  y1=rctSigLegend.bottom+2;
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  sprintf_s(Buf, "Score");
  TextOut(hdc, x1, y1, Buf, 5);
  SelectObject(hdc, hVFont);
  sprintf_s(Buf, "Probability");
  x1=rctSigLegend.left-18;
  y1=((rctSigPlot.top+rctSigPlot.bottom)>>1);
  TextOut(hdc, x1, y1, Buf, 11);
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  SetTextColor(hdc, cBlackColor);
  SelectObject(hdc, hTitleFont);
  x1=((rctSig.right+rctSig.left)>>1);
  y1=rctSig.top+5;
  TextOut(hdc, x1, y1, "Significance Curve", 18);
}
