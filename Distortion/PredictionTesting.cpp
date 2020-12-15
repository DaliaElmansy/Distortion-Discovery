#include "stdafx.h"

#define GRAPHHIEGHT 275
#define GRAPHWIDTH 250
#define PLOTSIZE 200

unsigned int WINAPI PredictionTestingThread(void *);
void DisplayPredictionTestingParams(int =0);
void EnablePredictionTestingDlgItems(BOOL);
BOOL CollectPredictionTestingParams(int =0);
BOOL PredictionTestingParamsChanged(int =0);
void SavePredictionTestingChanges();
void SavePredictionTestingParams();
//void LoadPredictionTesting();
void PaintPredictionTesting(HDC);
void CalcPredictorsXY();

BOOL PredictionTestingNow=FALSE;
Bezier BzrDist[3][2], BzrNoDist;
REAL xROCKnots[MAXROCSTEPS+2], yROCKnots[MAXROCSTEPS+2];
UINT TopDist, TopNoDist;
HANDLE hPredictionTestingThread=0;
HRGN hRgnTop=NULL, hRgnROC=NULL, hRgnROC_Only=NULL;
RECT rctClient, rctROC, rctTop, rctTopPlot, rctROCPlot,
     *RectsPT[2]={&rctROC, &rctTop},
     *RectsPlotPT[2]={&rctROCPlot, &rctTopPlot};
char *ROC_FileName[3]={"ROC_Curves-Both.png","ROC_Curves-FP.png","ROC_Curves-FN.png"};
//HBITMAP hTexture;

// Message handler for Prediction box.
INT_PTR CALLBACK PredictionTestingProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent, zDelta=0;
        char Buf[MAX_PATH+1];
	PAINTSTRUCT ps;
	HDC hdc;
        REAL x=0;
        BOOL Shift=0, Ctrl=0;
        UINT ThreadID=0;
        NMUPDOWN *mnud=0;
        HDC hdcTexture;

	switch (message)
	{
	case WM_INITDIALOG:
             hCurrentCursor=hWaitCursor;
             SetCursor(hWaitCursor);
             if(!tested)
               {
                 TopDist=BST_CHECKED;
                 TopNoDist=BST_UNCHECKED;
               }
             CurrentDlg=hDlg;
             ds->LoadParamFile("PredictionTestingParameters.txt");
             DisplayPredictionTestingParams();
             EditProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg,IDC_DATASET_NAME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_TOP_AUC_DIST), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_TOP_AUC_NODIST), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_TOP_PREDICTION_RATE_DIST), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_TOP_PREDICTION_RATE_NODIST), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_TOP_COUNT_DIST), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_TOP_COUNT_NODIST), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_ELAPSED_TIME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_REMAINING_TIME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_PROGRESS_PERCENT), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             SetWindowLongPtr(GetDlgItem(hDlg,IDC_STATUS), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
             changed = tested = FALSE;
             GetClientRect(hDlg, &rctClient);
             rctROC.right=(rctClient.right-5);
             rctROC.left=rctROC.right-GRAPHWIDTH+1;
             rctROC.top=3;
             rctROC.bottom=rctROC.top+GRAPHHIEGHT+1;
             rctROCPlot.top=rctROC.top+33;
             rctROCPlot.right=rctROC.right-5;
             rctROCPlot.bottom=rctROCPlot.top+PLOTSIZE;
             rctROCPlot.left=rctROCPlot.right-PLOTSIZE;
             rctTop=rctROC;
             rctTop.top+=7+GRAPHHIEGHT;
             rctTop.bottom+=7+GRAPHHIEGHT;
             rctTopPlot.top=rctTop.top+33;
             rctTopPlot.right=rctTop.right-5;
             rctTopPlot.bottom=rctTopPlot.top+PLOTSIZE+1;
             rctTopPlot.left=rctTopPlot.right-PLOTSIZE+1;
             if(hRgnTop==NULL)
                hRgnTop=CreateRectRgn(rctTopPlot.left-2, rctTopPlot.top-2,
                                      rctTopPlot.right+2, rctTopPlot.bottom);
             if(hRgnROC==NULL)
                hRgnROC=CreateRectRgn(rctROCPlot.left-2, rctROCPlot.top-2,
                                      rctROCPlot.right+2,  rctROCPlot.bottom);
             PredictionTestingNow=Aborted=FALSE;
             if(ds->LoadPredictionTesting())
               {
                 CalcPredictorsXY();
                 InvalidateRect(hDlg, 0, 0);
                 tested = true;
               }
             hCurrentCursor=hArrowCursor;
             SetCursor(hCurrentCursor);
             return 0;
	case WM_NOTIFY:
             mnud = (NMUPDOWN*)lParam;
             Shift=GetKeyState(VK_SHIFT)>>15;
             Ctrl=GetKeyState(VK_CONTROL)>>15;
             switch(wParam)
                   {
                     case IDC_SPIN_TOP_AUC_DIST:
                          if(   FocusOn>0
                             && mnud->hdr.code==UDN_DELTAPOS
                            )
                            {
                              x=Ctrl?0.1:(Shift?0.01:0.001);
                              if(mnud->iDelta>0)
                                 x=-x;
                              ds->TopAUC_Dist[FocusOn]+=x;
                              if(ds->TopAUC_Dist[FocusOn]<0)
                                 ds->TopAUC_Dist[FocusOn]=0;
                              if(ds->TopAUC_Dist[FocusOn]>1)
                                 ds->TopAUC_Dist[FocusOn]=1;
                              DisplayPredictionTestingParams(IDC_TOP_AUC_DIST);
                              ds->CalcCombined();
                              CalcPredictorsXY();
                              InvalidateRect(hDlg, 0, 0);
                              changed=TRUE;
                              return 1;
                            }
                          break;
                     case IDC_SPIN_TOP_PREDICTIONRATE_DIST:
                          if(   FocusOn>0
                             && mnud->hdr.code==UDN_DELTAPOS
                            )
                            {
                              x=Ctrl?0.1:(Shift?0.01:0.001);
                              if(mnud->iDelta>0)
                                 x=-x;
                              ds->TopPredictionRate_Dist[FocusOn]+=x;
                              if(ds->TopPredictionRate_Dist[FocusOn]<0)
                                 ds->TopPredictionRate_Dist[FocusOn]=0;
                              if(ds->TopPredictionRate_Dist[FocusOn]>1)
                                 ds->TopPredictionRate_Dist[FocusOn]=1;
                              DisplayPredictionTestingParams(IDC_TOP_PREDICTION_RATE_DIST);
                              ds->CalcCombined();
                              CalcPredictorsXY();
                              InvalidateRect(hDlg, 0, 0);
                              changed=TRUE;
                              return 1;
                            }
                          break;
                     case IDC_SPIN_TOP_AUC_NODIST:
                          if(mnud->hdr.code==UDN_DELTAPOS)
                            {
                              x=Ctrl?0.1:(Shift?0.01:0.001);
                              if(mnud->iDelta>0)
                                 x=-x;
                              ds->TopAUC_NoDist+=x;
                              if(ds->TopAUC_NoDist<0)
                                 ds->TopAUC_NoDist=0;
                              if(ds->TopAUC_NoDist>1)
                                 ds->TopAUC_NoDist=1;
                              DisplayPredictionTestingParams(IDC_TOP_AUC_NODIST);
                              ds->CalcCombined();
                              CalcPredictorsXY();
                              InvalidateRect(hDlg, 0, 0);
                              changed=TRUE;
                              return 1;
                            }
                          break;
                     case IDC_SPIN_TOP_PREDICTIONRATE_NODIST:
                          if(mnud->hdr.code==UDN_DELTAPOS)
                            {
                              x=Ctrl?0.1:(Shift?0.01:0.001);
                              if(mnud->iDelta>0)
                                 x=-x;
                              ds->TopPredictionRate_NoDist+=x;
                              if(ds->TopPredictionRate_NoDist<0)
                                 ds->TopPredictionRate_NoDist=0;
                              if(ds->TopPredictionRate_NoDist>1)
                                 ds->TopPredictionRate_NoDist=1;
                              DisplayPredictionTestingParams(IDC_TOP_PREDICTION_RATE_NODIST);
                              ds->CalcCombined();
                              CalcPredictorsXY();
                              InvalidateRect(hDlg, 0, 0);
                              changed=TRUE;
                              return 1;
                            }
                          break;
                   }
                break;
	case WM_SETCURSOR:
                if(hCurrentCursor!=0)
                   SetCursor(hCurrentCursor);
                return TRUE;
        case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDC_APPLY:
                     EnableWindow(GetDlgItem(hDlg, ID_CLOSE), 0);
                     EnableWindow(GetDlgItem(hDlg, IDC_APPLY), 0);
                     EnablePredictionTestingDlgItems(0);
                     hCurrentCursor=hWaitCursor;
                     crsr=SetCursor(hWaitCursor);
                     SetDlgItemText(hDlg, IDCANCEL, "Abort");
                     PredictionTestingNow=TRUE;
                     LowLight(hDlg, RectsPlotPT, CountOf(RectsPlotPT));
                     InvalidateRect(hDlg, 0, 0);
                     GetDumps(RectsPT, CountOf(RectsPT));
                     SavePredictionTestingChanges();
                     hPredictionTestingThread=(HANDLE)_beginthreadex(0, 0,
                       (ThreadAPI)PredictionTestingThread,
                        0, 0, &ThreadID);
                     return 0;
                case IDC_TOP_DIST:
                     CheckDlgButton(CurrentDlg, IDC_TOP_DIST, TopDist=TRUE);
                     CheckDlgButton(CurrentDlg, IDC_TOP_NODIST, TopNoDist=FALSE);
                     if(tested)
                        InvalidateRect(hDlg, 0, 0);
                     return 0;     
                case IDC_TOP_NODIST:
                     CheckDlgButton(CurrentDlg, IDC_TOP_NODIST, TopNoDist=TRUE);
                     CheckDlgButton(CurrentDlg, IDC_TOP_DIST, TopDist=FALSE);
                     if(tested)
                        InvalidateRect(hDlg, 0, 0);
                     return 0;     
                case IDC_FOCUS_ON_BOTH:
                case IDC_FOCUS_ON_FN:
                case IDC_FOCUS_ON_FP:
                     if(wmEvent==BN_CLICKED)
                     if(PredictionTestingParamsChanged(wmId))
                       {
                         SetDlgItemInt(hDlg, IDC_TOP_COUNT_DIST, ds->nTopPredictorsDist[FocusOn], 0);
                         DisplayPredictionTestingParams(IDC_TOP_PREDICTION_RATE_DIST);
                         DisplayPredictionTestingParams(IDC_TOP_AUC_DIST);
                         InvalidateRect(hDlg, 0, 0);
                       }
                     return 0;
                case IDC_PRINT_ROC:
                     if(DialogBox(hInst, MAKEINTRESOURCE(IDD_SCALE), hDlg, ScaleProc))
                        SaveRectToFile(rctROC, ROC_FileName[FocusOn], DrawROC);
                     return 0;
                case ID_CLOSE:
                        SavePredictionTestingParams();
                        EndDialog(hDlg, 0);
                        return 0;
		case IDCANCEL:
			if(PredictionTestingNow)
                          {
                                if(MessageBox(hDlg, "Abort Testing?",
                                                    "Interruption...",
                                                    MB_OKCANCEL|MB_ICONHAND|MB_DEFBUTTON2
                                             )!=IDOK
                                   || !PredictionTestingNow //in case user didn't respond to messagebox until end Testing
                                  )
                                   return 0;
                            SetDlgItemText(hDlg, IDCANCEL, "Cancel");
                            ds->Abort();
                            return 0;
                          }
                        SavePredictionTestingParams();
			EndDialog(hDlg, 0);
			return 0;
		}
		break;
	//case WM_ERASEBKGND:
 //            hdc = GetDC(hDlg);//(HDC)LOWORD(wParam);
 //            FillRect(hdc, &rctClient, hLtLtGreyBrush);
 //            //hdcTexture=CreateCompatibleDC(hdc);
 //            //SelectObject(hdcTexture, hTexture);
 //            //BitBlt(hdc, 0, 0, rctClient.right, rctClient.bottom,
 //            //       hdcTexture, 0, 0, SRCCOPY);
 //            //DeleteDC(hdcTexture);
 //            ReleaseDC(hDlg, hdc);
 //            return TRUE;
        case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
                PaintPredictionTesting(hdc);
		EndPaint(hDlg, &ps);
		return 0;
	}
	return (INT_PTR)FALSE;
}

unsigned int WINAPI PredictionTestingThread(void *param)
{
  ProgressSize=ds->nShuffles;
  InitProgress();
  Status("Testing...");
  Aborted = !(ds->PredictionTesting(Progress));
  if(Aborted)
    {
      EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDCANCEL), 1);
      EnablePredictionTestingDlgItems(1);
      Aborted=FALSE;
      if(tested && ds->LoadPredictionTesting())
         CalcPredictorsXY();
      SendMessage(GetDlgItem(CurrentDlg, IDC_PROGRESS_BAR), PBM_SETPOS, (WPARAM) 0, 0);
      SetDlgItemText(CurrentDlg, IDC_PROGRESS_PERCENT,"");
      SetDlgItemText(CurrentDlg, IDC_ELAPSED_TIME,"");
      SetDlgItemText(CurrentDlg, IDC_REMAINING_TIME,"");
    }
  else
    {
      CalcPredictorsXY();
      tested=TRUE;
      ds->SavePredictionTesting("PredictionsDist.txt", "PredictionsNoDist.txt");
      EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      EnablePredictionTestingDlgItems(1);
      SetDlgItemText(CurrentDlg, IDCANCEL, "Cancel");
    }
  Status("");
  PredictionTestingNow=FALSE;
  hCurrentCursor=crsr;
  SendMessage(CurrentDlg, WM_SETCURSOR, 0,0);
  InvalidateRect(CurrentDlg, 0, 0);
  CloseHandle(hPredictionTestingThread);
  _endthreadex(0);
  return 0;
}

void DisplayPredictionTestingParams(int Item)
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
  if(Item==IDC_MIN_I||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinI);
      SetDlgItemText(CurrentDlg, IDC_MIN_I, Buf);
    }
  if(Item==IDC_MIN_I_CORR||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinICorr);
      SetDlgItemText(CurrentDlg, IDC_MIN_I_CORR, Buf);
    }
  if(Item==IDC_MIN_MIS||Item==0)
     SetDlgItemInt(CurrentDlg, IDC_MIN_MIS, ds->MinMis, 0);
 
  if(Item==IDC_FOCUS_ON_FP||IDC_FOCUS_ON_FN||IDC_FOCUS_ON_BOTH||Item==0)
     CheckRadioButton(CurrentDlg, IDC_FOCUS_ON_FP, IDC_FOCUS_ON_BOTH, IDC[FocusOn]);
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
  if(Item==IDC_MIN_PRECISION||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinPrecision);
      SetDlgItemText(CurrentDlg, IDC_MIN_PRECISION, Buf);
    }
  if(Item==IDC_MIN_COHESION||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinCohesion);
      SetDlgItemText(CurrentDlg, IDC_MIN_COHESION, Buf);
    }
  if(Item==IDC_MIN_SCORE||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinDistScore);
      SetDlgItemText(CurrentDlg, IDC_MIN_SCORE, Buf);
    }
  if(Item==IDC_MIN_AUC_GAIN||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinAUC_Gain);
      SetDlgItemText(CurrentDlg, IDC_MIN_AUC_GAIN, Buf);
    }
  if(Item==IDC_MIN_PREDICTION_RATE||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinPredictionRate);
      SetDlgItemText(CurrentDlg, IDC_MIN_PREDICTION_RATE, Buf);
    }
  if(Item==IDC_MIN_AUC||Item==0)
    {
      sprintf_s(Buf, "%g", ds->MinAUC);
      SetDlgItemText(CurrentDlg, IDC_MIN_AUC, Buf);
    }
  if(Item==IDC_TOP_PREDICTION_RATE_DIST||Item==0)
    {
      if(FocusOn==0)
         Buf[0]=0;
      else sprintf_s(Buf, "%5.3f", ds->TopPredictionRate_Dist[FocusOn]);
      SetDlgItemText(CurrentDlg, IDC_TOP_PREDICTION_RATE_DIST, Buf);
    }
  if(Item==IDC_TOP_AUC_DIST||Item==0)
    {
      if(FocusOn==0)
         Buf[0]=0;
      else sprintf_s(Buf, "%5.3f", ds->TopAUC_Dist[FocusOn]);
      SetDlgItemText(CurrentDlg, IDC_TOP_AUC_DIST, Buf);
    }
  if(Item==IDC_TOP_PREDICTION_RATE_NODIST||Item==0)
    {
      sprintf_s(Buf, "%5.3f", ds->TopPredictionRate_NoDist);
      SetDlgItemText(CurrentDlg, IDC_TOP_PREDICTION_RATE_NODIST, Buf);
    }
  if(Item==IDC_TOP_AUC_NODIST||Item==0)
    {
      sprintf_s(Buf, "%5.3f", ds->TopAUC_NoDist);
      SetDlgItemText(CurrentDlg, IDC_TOP_AUC_NODIST, Buf);
    }
  CheckDlgButton(CurrentDlg, IDC_TOP_DIST, TopDist);
  CheckDlgButton(CurrentDlg, IDC_TOP_NODIST, TopNoDist);
}

void EnablePredictionTestingDlgItems(BOOL e)
{
  EnableWindow(GetDlgItem(CurrentDlg, IDC_N_SHUFFLES), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_I), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_I_CORR), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_MIS), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_FOCUS_ON_FP), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_FOCUS_ON_FN), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_FOCUS_ON_BOTH), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MAX_MIS_RATIO), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MAX_MIS_TOTAL), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_SPOTTED), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_SPOTTING_RATE), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_PRECISION), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_COHESION), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_SCORE), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_AUC_GAIN), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_PREDICTION_RATE), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MIN_AUC), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_PREDICTION_RATE_DIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_AUC_DIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_PREDICTION_RATE_NODIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_AUC_NODIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_DIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_NODIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_PRINT_ROC), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_COUNT_DIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_TOP_COUNT_NODIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_SPIN_TOP_AUC_DIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_SPIN_TOP_AUC_NODIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_SPIN_TOP_PREDICTIONRATE_DIST), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_SPIN_TOP_PREDICTIONRATE_NODIST), e);
}

BOOL CollectPredictionTestingParams(int Item)
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
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }

  if(Item==IDC_MIN_I || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_I, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinI)
        {
          ds->MinI=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_I_CORR || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_I_CORR, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinICorr)
        {
          ds->MinICorr=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_MIS || Item==0)
    {
      i=GetDlgItemInt(CurrentDlg, IDC_MIN_MIS,NULL,1);
      if(i!=ds->MinMis)
        {
          ds->MinMis=i;
          ch=TRUE;
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
          ch=TRUE;
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
          ch=TRUE;
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
  if(Item==IDC_MIN_PRECISION || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_PRECISION, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinPrecision)
        {
          ds->MinPrecision=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_COHESION || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_COHESION, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinCohesion)
        {
          ds->MinCohesion=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_SCORE || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_SCORE, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinDistScore)
        {
          ds->MinDistScore=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_AUC_GAIN|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_AUC_GAIN, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinAUC_Gain)
        {
          ds->MinAUC_Gain=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_PREDICTION_RATE || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_PREDICTION_RATE, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinPredictionRate)
        {
          ds->MinPredictionRate=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_MIN_AUC || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_AUC, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->MinAUC)
        {
          ds->MinAUC=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_TOP_PREDICTION_RATE_DIST || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_PREDICTION_RATE_DIST, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->TopPredictionRate_Dist[FocusOn])
        {
          ds->TopPredictionRate_Dist[FocusOn]=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_TOP_AUC_DIST || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_AUC_DIST, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->TopAUC_Dist[FocusOn])
        {
          ds->TopAUC_Dist[FocusOn]=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_TOP_PREDICTION_RATE_NODIST || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_PREDICTION_RATE_NODIST, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->TopPredictionRate_NoDist)
        {
          ds->TopPredictionRate_NoDist=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  if(Item==IDC_TOP_AUC_NODIST || Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_AUC_NODIST, Buf, sizeof(Buf));
      x=atof(Buf);
      if(x!=ds->TopAUC_NoDist)
        {
          ds->TopAUC_NoDist=x;
          ch=TRUE;
        }
      if(Item!=0) return ch;
    }
  return ch;
}

BOOL PredictionTestingParamsChanged(int Item)
{
  char Buf[64];
  BOOL success, ch=FALSE;
  int i;
  REAL x;

  if(Item==IDC_N_SHUFFLES|| Item==0)
    {
      if(ds->nShuffles!=GetDlgItemInt(CurrentDlg, IDC_N_SHUFFLES,&success,1))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_I|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_I, Buf, sizeof(Buf));
      if(ds->MinI!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_I_CORR|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_I_CORR, Buf, sizeof(Buf));
      if(ds->MinICorr!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_MIS|| Item==0)
    {
      if(ds->MinMis!=GetDlgItemInt(CurrentDlg, IDC_MIN_MIS,&success,1))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_FOCUS_ON_FP||Item==IDC_FOCUS_ON_FN||Item==IDC_FOCUS_ON_BOTH||Item==0)
    {
      if(BST_CHECKED==IsDlgButtonChecked(CurrentDlg, IDC_FOCUS_ON_FP))
        {
          if(FocusOn!=Dataset::FP)
             FocusOn=Dataset::FP;
          if(Item!=0)
             return TRUE;
        }
      else if(BST_CHECKED==IsDlgButtonChecked(CurrentDlg, IDC_FOCUS_ON_FN))
             {
               if(FocusOn!=Dataset::FN)
                  FocusOn=Dataset::FN;
               if(Item!=0)
             return TRUE;
             }
      else if(FocusOn!=Dataset::Both)
              FocusOn=Dataset::Both;
           if(Item!=0)
             return TRUE;
    }
  if(Item==IDC_MAX_MIS_RATIO|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MAX_MIS_RATIO, Buf, sizeof(Buf));
      if(ds->MaxMisRatio!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MAX_MIS_TOTAL|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MAX_MIS_TOTAL, Buf, sizeof(Buf));
      if(ds->MaxMisTotal!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
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
  if(Item==IDC_MIN_PRECISION|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_PRECISION, Buf, sizeof(Buf));
      if(ds->MinPrecision!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_COHESION|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_COHESION, Buf, sizeof(Buf));
      if(ds->MinCohesion!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_SCORE|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_SCORE, Buf, sizeof(Buf));
      if(ds->MinDistScore!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_AUC_GAIN|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_AUC_GAIN, Buf, sizeof(Buf));
      if(ds->MinAUC_Gain!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_PREDICTION_RATE|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_PREDICTION_RATE, Buf, sizeof(Buf));
      if(ds->MinPredictionRate!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_MIN_AUC|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_AUC, Buf, sizeof(Buf));
      if(ds->MinAUC!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_TOP_PREDICTION_RATE_DIST|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_PREDICTION_RATE_DIST, Buf, sizeof(Buf));
      if(ds->TopPredictionRate_Dist[FocusOn]!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_TOP_AUC_DIST|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_AUC_DIST, Buf, sizeof(Buf));
      if(ds->TopAUC_Dist[FocusOn]!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_TOP_PREDICTION_RATE_NODIST|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_PREDICTION_RATE_NODIST, Buf, sizeof(Buf));
      if(ds->TopPredictionRate_NoDist!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  if(Item==IDC_TOP_AUC_NODIST|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_TOP_AUC_NODIST, Buf, sizeof(Buf));
      if(ds->TopAUC_NoDist!=atof(Buf))
         ch=TRUE;
      if(Item!=0)
         return ch;
    }
  return FALSE;
}

void PredictionTesting()
{
  crsr=hCurrentCursor=hWaitCursor;
  SetCursor(hCurrentCursor);
  //hTexture = (HBITMAP)LoadImageA(hInst, MAKEINTRESOURCE(IDB_TEXTURE_1),0,0,0,0);//"texture1.bmp" 
  DialogBox(hInst, MAKEINTRESOURCE(IDD_PREDICTION_TESTING), hMainWnd, PredictionTestingProc);
  ds->Testing=false;
  hCurrentCursor=hArrowCursor;
  SetCursor(hCurrentCursor);
  //DeleteBitmap(hTexture);
}
void SavePredictionTestingChanges()
{
  CollectPredictionTestingParams();
  ds->SavePredictionTestingParamFile("PredictionTestingParameters.txt");
}
void SavePredictionTestingParams()
{
  if(changed)
    {
      ds->SavePredictionTestingParamFile("PredictionTestingParameters.txt");
      changed=FALSE;
    }
}

void CalcPredictorsXY()
{
  int i, j, maxn, x0, y0;

  nPredictorsDist = ds->nPredictorsDist;
  nPredictorsNoDist = ds->nPredictorsNoDist;
  maxn = nPredictorsDist + nPredictorsNoDist;
  if(SizePredictors<maxn)
    {
      SizePredictors=maxn+100;
      DeleteMatrix(Predictors_X, 3);
      DeleteMatrix(Predictors_Y, 3);
      Predictors_X=NewMatrix<int>(3, SizePredictors);
      Predictors_Y=NewMatrix<int>(3, SizePredictors);
    }
  x0=rctTopPlot.left;
  y0=rctTopPlot.bottom;
  for(i=0; i<maxn; i++)
     {
       //if(FocusOn==0 || ds->pPredictorsDist[i]->Class==FocusOn-1)
       if(i<nPredictorsDist)
         {
           Predictors_X[1][i]=x0+Round(ds->pPredictorsDist[i]->Counters[1].PredictionRate*PLOTSIZE);
           Predictors_Y[1][i]=y0-Round(ds->pPredictorsDist[i]->Counters[1].AUC*PLOTSIZE);
         }
       if(i<nPredictorsNoDist)
         {
           Predictors_X[2][i]=x0+Round(ds->pPredictorsNoDist[i]->Counters[2].PredictionRate*PLOTSIZE);
           Predictors_Y[2][i]=y0-Round(ds->pPredictorsNoDist[i]->Counters[2].AUC*PLOTSIZE);
         }
     }
  CalcROC();
  SetDlgItemInt(CurrentDlg, IDC_TOP_COUNT_DIST, ds->nTopPredictorsDist[FocusOn], 0);
  SetDlgItemInt(CurrentDlg, IDC_TOP_COUNT_NODIST, ds->nTopPredictorsNoDist, 0);
}

void CalcROC()
{
  int i, d, f;

  nKnots = ds->nROC_Steps;
  
  for(i=0; i<nKnots; i++)
     {
       xROCKnots[i]=rctROCPlot.right-Round(ds->CombinedNoDist.Specificity[i]*PLOTSIZE);
       yROCKnots[i]=rctROCPlot.bottom-Round(ds->CombinedNoDist.Sensitivity[i]*PLOTSIZE);
     }
  BzrNoDist.Init(nKnots, xROCKnots, yROCKnots);
  for(f=0; f<3; f++)
  for(d=0; d<2; d++)
     {
       for(i=0; i<nKnots; i++)
          {
            xROCKnots[i]=rctROCPlot.right-Round(ds->CombinedDist[f][d].Specificity[i]*PLOTSIZE);
            yROCKnots[i]=rctROCPlot.bottom-Round(ds->CombinedDist[f][d].Sensitivity[i]*PLOTSIZE);
          }
        BzrDist[f][d].Init(nKnots, xROCKnots, yROCKnots);
     }
}

void DrawROC(HDC hdc, REAL Scale)
{
  int i, j, _20=20*Scale, _2=2*Scale, _3=3*Scale,
      nPoints=BzrNoDist.nPoints, Ink=10.01+Scale*178;
  REAL x1, y1, x2, y2;
  RECT rctROC_Only={0,0,GRAPHWIDTH*Scale+1,GRAPHHIEGHT*Scale+1},
       rctROCPlotOnly={(GRAPHWIDTH-PLOTSIZE-6)*Scale, 33*Scale,
                       (GRAPHWIDTH-6)*Scale,(PLOTSIZE+33)*Scale},
       rctROCLegend={rctROCPlotOnly.left-23*Scale,
                     rctROCPlotOnly.top-5*Scale,
                     rctROCPlotOnly.right+3*Scale,
                     rctROCPlotOnly.bottom+23*Scale};
  char *lgnd[]={"0","0.1","0.2","0.3","0.4","0.5",
                "0.6","0.7","0.8","0.9","1"},
       Buf[64];
  POINT Points[3][(MAXROCSTEPS+1)*3+1];

  if(!tested)
      return;

  SetGraphicsMode(hdc, GM_ADVANCED);
  SetMapMode(hdc, MM_TEXT);
  
  //ScaleRect(rctROC_Only, Scale);
  //ScaleRect(rctROCPlotOnly, Scale);
  //ScaleRect(rctROCLegend, Scale);

  for(i=0; i<3; i++)
  for(j=0; j<nPoints; j++)
     {
       Bezier &bzr=i==2?BzrNoDist:BzrDist[FocusOn][i];
       Points[i][j].x=(bzr.Points[j].x-rctROCPlot.left)*Scale;
       Points[i][j].x+=rctROCPlotOnly.left;
       Points[i][j].y=(bzr.Points[j].y-rctROCPlot.top)*Scale;
       Points[i][j].y+=rctROCPlotOnly.top;
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

  hRgnROC_Only=CreateRectRgn(rctROCPlotOnly.left-2, rctROCPlotOnly.top-2,
                      rctROCPlotOnly.right+2,  rctROCPlotOnly.bottom);

  FillRect(hdc, &rctROC_Only, hWhiteBrush);
  SelectObject(hdc, Scale>=1?hBlackPen2:hBlackPen);
  MoveToEx(hdc, rctROC_Only.left+1, rctROC_Only.top+1, 0);
  LineTo(hdc, rctROC_Only.right-2, rctROC_Only.top+1);
  LineTo(hdc, rctROC_Only.right-2, rctROC_Only.bottom-2);
  LineTo(hdc, rctROC_Only.left+1, rctROC_Only.bottom-2);
  LineTo(hdc, rctROC_Only.left+1, rctROC_Only.top-1);
  FillRect(hdc, &rctROCLegend, hLtLtLtGreyBrush);//hLegendBkBrush);
  FillRect(hdc, &rctROCPlotOnly, hWhiteBrush);//hPlotBkBrush);
  SelectObject(hdc, hGridMinorPen);
  x1=rctROCPlotOnly.left-_3;
  x2=rctROCPlotOnly.right;
  for(i=1; i<10; i++)
     {
       y1=rctROCPlotOnly.bottom-i/10.0*PLOTSIZE*Scale;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }
  y1=rctROCPlotOnly.top;
  y2=rctROCPlotOnly.bottom;
  for(i=0; i<10; i++)
     {
       x1=rctROCPlotOnly.left+i/10.0*PLOTSIZE*Scale;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  SetBkMode(hdc, TRANSPARENT);
  SelectObject(hdc, hGridMajorPen);
  SetTextAlign(hdc, TA_RIGHT|TA_BASELINE|TA_NOUPDATECP);
  SetTextColor(hdc, 0);
  SelectObject(hdc, hScaleFont);

  x1=rctROCPlotOnly.left-_3;
  x2=rctROCPlotOnly.right;
  for(i=0; i<=10; i++,i++)
     {
       y1=rctROCPlotOnly.bottom-i/10.0*PLOTSIZE*Scale;
       if(i>0&&i<10)
         {
           MoveToEx(hdc, x1, y1, 0);
           LineTo(hdc, x2, y1);
         }
       TextOut(hdc, x1-Scale, y1+5*Scale, lgnd[i], i==0||i==10?1:3);
     }
  SetTextAlign(hdc, TA_CENTER|TA_TOP|TA_NOUPDATECP);
  y1=rctROCPlotOnly.top;
  y2=rctROCPlotOnly.bottom+_3;
  for(i=0; i<=10; i++,i++)
     {
       x1=rctROCPlotOnly.left+i/10.0*PLOTSIZE*Scale;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
       TextOut(hdc, x1, y2+Scale, lgnd[10-i], i==0||i==10?1:3);
     }
  SelectObject(hdc, hBlackPen);
  MoveToEx(hdc, rctROCPlotOnly.left,  rctROCPlotOnly.top, 0);
  LineTo(hdc,   rctROCPlotOnly.left,  rctROCPlotOnly.bottom);
  LineTo(hdc,   rctROCPlotOnly.right, rctROCPlotOnly.bottom);
  LineTo(hdc,   rctROCPlotOnly.right, rctROCPlotOnly.top);
  LineTo(hdc,   rctROCPlotOnly.left,  rctROCPlotOnly.top);
  SelectClipRgn(hdc, hRgnROC_Only);
  SelectObject(hdc, hNormalFont);
  SelectObject(hdc, hGreyPlotPens[int(0.0001+Scale)]);
  PolyBezier(hdc, Points[2], nPoints);
  x1=rctROCPlotOnly.right-133*Scale;
  x2=x1+20*Scale;
  y1=rctROCPlotOnly.bottom-30*Scale+20*Scale;
  MoveToEx(hdc, x1, y1, 0);
  LineTo(hdc, x2, y1);
  sprintf_s(Buf, "Not Dist: AUC=%4.2f",ds->CombinedNoDist.AUC);
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
  x1=(rctROCPlotOnly.left+rctROCPlotOnly.right)>>1;
  y1=((rctROCLegend.bottom+rctROC_Only.bottom)>>1)+_3+1;
  sprintf_s(Buf, "Specificity");
  TextOut(hdc, x1, y1, Buf, 11);
  SelectObject(hdc, hVLegendFont);
  x1=((rctROCLegend.left+rctROC_Only.left)>>1)+5*Scale+1;
  y1=(rctROCPlotOnly.top+rctROCPlotOnly.bottom)>>1;
  sprintf_s(Buf, "Sensitivity");
  TextOut(hdc, x1, y1, Buf, 11);
  SetTextColor(hdc, cBlackColor);
  SelectObject(hdc, hTitleFont);
  x1=(rctROC_Only.left+rctROC_Only.right)>>1;
  y1=((rctROC_Only.top+rctROCLegend.top)>>1)+8*Scale+1;
  TextOut(hdc, x1, y1, "ROC Curves", 10);
  DeleteObject(hRgnROC_Only);
}
// Paint Prediction Testing
void PaintPredictionTesting(HDC hdc)
{
  int i, j, dot=2;
  REAL x1, y1, x2, y2;
  RECT rctTopLegend=rctTopPlot, rctROCLegend=rctROCPlot;
  HPEN hDistPens[]={hLtBluePen, hDkBluePen},
       hNoDistPens[]={hGreyPen, hDkGreyPen};
  HBRUSH hDistBrushes[]={hLtBlueBrush, hDkBlueBrush},
         hNoDistBrushes[]={hGreyBrush, hDkGreyBrush};
  char *lgnd[]={"0","0.1","0.2","0.3","0.4","0.5",
                "0.6","0.7","0.8","0.9","1"},
       Buf[64];

  SetGraphicsMode(hdc, GM_ADVANCED);
  SetMapMode(hdc, MM_TEXT);

  if(PredictionTestingNow)
    {
      PutDumps(hdc, RectsPT, CountOf(RectsPT));
      return;
    }

  rctTopLegend.top-=5;
  rctTopLegend.bottom+=21;
  rctTopLegend.left-=23;
  rctTopLegend.right+=1;
  rctROCLegend.top-=5;
  rctROCLegend.bottom+=21;
  rctROCLegend.left-=23;
  rctROCLegend.right+=1;
  SelectObject(hdc, hDkGreyPen);
  SelectObject(hdc, hLtLtGreyBrush);
  Rectangle(hdc, rctTop.left, rctTop.top, rctTop.right, rctTop.bottom);
  Rectangle(hdc, rctROC.left, rctROC.top, rctROC.right, rctROC.bottom);
  FillRect(hdc, &rctTopLegend, hLegendBkBrush);
  FillRect(hdc, &rctROCLegend, hLegendBkBrush);
  FillRect(hdc, &rctTopPlot, hPlotBkBrush);
  FillRect(hdc, &rctROCPlot, hPlotBkBrush);
  SelectObject(hdc, hGridMinorPen);
  x1=rctTopPlot.left-3;
  x2=rctTopPlot.right;
  for(i=0; i<10; i++)
     {
       y1=rctTopPlot.bottom-i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }
  y1=rctTopPlot.top;
  y2=rctTopPlot.bottom+3;
  for(i=1; i<10; i++)
     {
       x1=rctTopPlot.left+i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  x1=rctROCPlot.left-3;
  x2=rctROCPlot.right;
  for(i=1; i<10; i++)
     {
       y1=rctROCPlot.bottom-i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }
  y1=rctROCPlot.top;
  y2=rctROCPlot.bottom;
  for(i=0; i<10; i++)
     {
       x1=rctROCPlot.left+i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  SetBkMode(hdc, TRANSPARENT);
  SelectObject(hdc, hGridMajorPen);
  SetTextAlign(hdc, TA_RIGHT|TA_BASELINE);
  SetTextColor(hdc, 0);
  //SetBkColor(hdc, LegendBkColor);
  SelectObject(hdc, (HFONT)SendMessage(CurrentDlg, WM_GETFONT,0,0));

  x1=rctTopPlot.left-4;
  x2=rctTopPlot.right+1;
  for(i=0; i<=10; i++,i++)
     {
       y1=rctTopPlot.bottom-i/10.0*PLOTSIZE-1;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
       TextOut(hdc, x1, y1+4, lgnd[i], i==0||i==10?1:3);
     }
  for(i=0; i<=10; i++,i++)
     {
       y1=rctROCPlot.bottom-i/10.0*PLOTSIZE-1;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
       TextOut(hdc, x1, y1+4, lgnd[i], i==0||i==10?1:3);
     }
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  y1=rctTopPlot.top;
  y2=rctTopPlot.bottom+3;
  for(i=0; i<=10; i++,i++)
     {
       x1=rctTopPlot.left+i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
       TextOut(hdc, x1, y2, lgnd[i], i==0||i==10?1:3);
     }
  y1=rctROCPlot.top;
  y2=rctROCPlot.bottom+3;
  for(i=0; i<=10; i++,i++)
     {
       x1=rctROCPlot.left+i/10.0*PLOTSIZE;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
       TextOut(hdc, x1, y2, lgnd[10-i], i==0||i==10?1:3);
     }

  if(tested && TopDist)
    {
      for(i=0; i<nPredictorsDist; i++)
         {
           int f=ds->pPredictorsDist[i]->Class+1;
           if(FocusOn==0||f==FocusOn)
             {
               BOOL top=
                    (   ds->pPredictorsDist[i]->Counters[1].AUC>=ds->TopAUC_Dist[f]
                     && ds->pPredictorsDist[i]->Counters[1].PredictionRate>=ds->TopPredictionRate_Dist[f]
                    );
               //SetPixel(hdc, Predictors_X[1][i], Predictors_Y[1][i], clr);
               if(top||FocusOn>0)
                 {
                   SelectObject(hdc, hDistPens[top]);
                   SelectObject(hdc, hDistBrushes[top]);
                   Ellipse(hdc, Predictors_X[1][i]-dot, Predictors_Y[1][i]-dot,
                                Predictors_X[1][i]+dot, Predictors_Y[1][i]+dot);
                 }
             }
         }
      if(FocusOn>0)
        {
          SelectObject(hdc, hDkBluePen);
          i=Round(rctTopPlot.bottom-ds->TopAUC_Dist[FocusOn]*PLOTSIZE);
          MoveToEx(hdc, rctTopPlot.right, i, 0);
          LineTo(hdc, rctTopPlot.left, i);
          i=Round(rctTopPlot.left+ds->TopPredictionRate_Dist[FocusOn]*PLOTSIZE);
          MoveToEx(hdc, i, rctTopPlot.top, 0);
          LineTo(hdc, i, rctTopPlot.bottom);
        }
    }
  
  if(tested && TopNoDist)
    {
      for(i=0; i<nPredictorsNoDist; i++)
         {
           BOOL top =
             (   ds->pPredictorsNoDist[i]->Counters[2].AUC>=ds->TopAUC_NoDist
              && ds->pPredictorsNoDist[i]->Counters[2].PredictionRate>=ds->TopPredictionRate_NoDist
             );
           //SetPixel(hdc, Predictors_X[2][i], Predictors_Y[2][i], clr);
           SelectObject(hdc, hNoDistPens[top]);
           SelectObject(hdc, hNoDistBrushes[top]);
           Ellipse(hdc, Predictors_X[2][i]-dot, Predictors_Y[2][i]-dot,
                        Predictors_X[2][i]+dot, Predictors_Y[2][i]+dot);
         }
      SelectObject(hdc, hDkGreyPen);
      i=Round(rctTopPlot.bottom-ds->TopAUC_NoDist*PLOTSIZE);
      MoveToEx(hdc, rctTopPlot.right, i, 0);
      LineTo(hdc, rctTopPlot.left, i);
      i=Round(rctTopPlot.left+ds->TopPredictionRate_NoDist*PLOTSIZE);
      MoveToEx(hdc, i, rctTopPlot.top, 0);
      LineTo(hdc, i, rctTopPlot.bottom);
    }
  
if(tested)
  {
    nPoints=BzrNoDist.nPoints;
    SelectClipRgn(hdc, hRgnROC);
    SelectObject(hdc, hGreyPlotPen2);
    PolyBezier(hdc, BzrNoDist.Points, nPoints);
    x1=rctROCPlot.left+(PLOTSIZE>>1)-40;
    x2=x1+20;
    y1=rctROCPlot.bottom-(PLOTSIZE>>2)+40;
    MoveToEx(hdc, x1, y1, 0);
    LineTo(hdc, x2, y1);
    sprintf_s(Buf, "Not Dist: AUC=%4.2f",ds->CombinedNoDist.AUC);
    SetTextColor(hdc, cDkGreyColor);
    SetTextAlign(hdc, TA_LEFT|VTA_TOP);
    TextOut(hdc, x2+5, y1-7, Buf, 18);
    SelectObject(hdc, hRedPlotPen3);
    PolyBezier(hdc, BzrDist[FocusOn][0].Points, nPoints);
    y1=rctROCPlot.bottom-(PLOTSIZE>>2);
    MoveToEx(hdc, x1, y1, 0);
    LineTo(hdc, x2, y1);
    sprintf_s(Buf, "Before Corr: AUC=%4.2f",ds->CombinedDist[FocusOn][0].AUC);
    SetTextColor(hdc, cDkRedColor);
    TextOut(hdc, x2+5, y1-7, Buf, 21);
    SelectObject(hdc, hBluePlotPen3);
    PolyBezier(hdc, BzrDist[FocusOn][1].Points, nPoints);
    y1+=20;
    MoveToEx(hdc, x1, y1, 0);
    LineTo(hdc, x2, y1);
    sprintf_s(Buf, "After Corr: AUC=%4.2f",ds->CombinedDist[FocusOn][1].AUC);
    SetTextColor(hdc, cDkBlueColor);
    TextOut(hdc, x2+5, y1-7, Buf, 20);
    SelectClipRgn(hdc, 0);
  }
  SelectObject(hdc, hHighFont);
  SetTextColor(hdc, 0);
  x1=rctROCPlot.left+(PLOTSIZE>>1);
  y1=rctROCLegend.bottom+2;
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  sprintf_s(Buf, "Specificity");
  TextOut(hdc, x1, y1, Buf, 11);
  sprintf_s(Buf, "Prediction Rate");
  y1=rctTopLegend.bottom+2;
  TextOut(hdc, x1, y1, Buf, 15);
  SelectObject(hdc, hVFont);
  sprintf_s(Buf, "AUC");
  y1=((rctTopPlot.top+rctTopPlot.bottom)>>1);
  x1=rctTopLegend.left-17;
  TextOut(hdc, x1, y1, Buf, 3);
  sprintf_s(Buf, "Sensitivity");
  x1=rctROCLegend.left-18;
  y1=((rctROCPlot.top+rctROCPlot.bottom)>>1);
  TextOut(hdc, x1, y1, Buf, 11);
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  SetTextColor(hdc, cBlackColor);
  SelectObject(hdc, hTitleFont);
  x1=((rctROC.right+rctROC.left)>>1);
  y1=rctROC.top+5;
  TextOut(hdc, x1, y1, "ROC Curves", 10);
  x1=((rctTop.right+rctTop.left)>>1);
  y1=rctTop.top+5;
  TextOut(hdc, x1, y1, "Top Predictors", 14);
}
