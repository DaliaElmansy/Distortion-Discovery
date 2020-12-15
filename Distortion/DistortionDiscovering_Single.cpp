#include "stdafx.h"

#define VERTICESHEIGHT (HEATMAPWIDTH+2)
#define VERTICESWIDTH (HEATMAPHEIGHT+2)
#define VERTEXARRAYSIZE ((VERTICESHEIGHT)*(VERTICESWIDTH))
#define MESHSIZE (VERTEXARRAYSIZE<<1)
#define NRGRAPHHIEGHT 225
#define NRGRAPHWIDTH 327
#define NRPLOTWIDTH 270
#define NRPLOTHEIGHT 160
#define FreqBarMax 120

unsigned int WINAPI DistortionDiscovering_SingleThread();
unsigned int WINAPI DistortionDiscovering_SingleSyn2Thread();
BOOL DistortionDiscovering_SingleParamsChanged(int =0);
BOOL CollectDistortionDiscovering_SingleParams(int =0);
void EnableDistortionDiscovering_SingleDlgItems(BOOL);
void DisplayDistortionDiscovering_SingleParams(int =0);
void SaveDistortionDiscovering_SingleChanges();
//void SaveDistortionDiscovering_SingleParams();
void LoadDistortionDiscovering_Single();
void DrawNR(HDC, REAL);
void PaintDistortionDiscovering_Single(HDC);
void InitHeatMaps();
void DeleteHeatMaps();
void CalcHeatMaps();
void   CalcFreq();
BOOL CheckSyn2File();
void CopyVertexColor(TRIVERTEX&, TRIVERTEX&);
bool NotWhite(TRIVERTEX&);
ULONG Vertex(int,int);

int xStart=0, xEnd=30, xW=xEnd-xStart,
    MaxIFreq=0, MaxICorrFreq=0, MaxDistFreq=0;
BYTE BarIFreq[MAXFREQ], BarICorrFreq[3][MAXFREQ],
     BarDistFreq[3][MAXFREQ];
BOOL DistortionDiscovering_SingleLoaded,
     DistortionDiscovering_SingleNow=FALSE,
     DistortionDiscovering_SingleSyn2Now=FALSE,
     discovered, Syn2Found;
RECT rctHeatMap, rctHeatMapPlot;
HANDLE hDistortionDiscovering_SingleThread=0,
       hDistortionDiscovering_SingleSyn2Thread=0;
HBITMAP hHeatMaps[]={0,0,0};
HRGN hRgnNR=0, hRgnIC=0, hRgnDI=0, hRgnNR_Only=0;
RECT rctNR,rctIC,rctDI,rctNRPlot,rctICPlot,rctDIPlot,
     *RectsDD[3]={&rctNR,&rctIC,&rctDI},
     *RectsPlotDD[3]={&rctNRPlot,&rctICPlot,&rctDIPlot};
TRIVERTEX Verticese[3][VERTICESHEIGHT][VERTICESWIDTH];
GRADIENT_TRIANGLE Mesh[3][MESHSIZE];
char *NR_Fname[]={"Non-Redundancy_Plot_Both.png",
                  "Non-Redundancy_Plot_FP.png",
                  "Non-Redundancy_Plot_FN.png"};

// Message handler for Distortion Discovering dialog box.
INT_PTR CALLBACK DistortionDiscovering_SingleProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent, zDelta=0;
        char Buf[MAX_PATH+1];
	PAINTSTRUCT ps;
	HDC hdc;
        REAL x=0;
        BOOL Shift=0, Ctrl=0;
        UINT ThreadID=0;
        NMUPDOWN *mnud=0;

	switch (message)
	{
	case WM_INITDIALOG:
                hCurrentCursor=hWaitCursor;
                SetCursor(hWaitCursor);
                ds->LoadParamFile("DistortionDiscovering_SingleParameters.txt");
                if(!DistortionDiscovering_SingleLoaded)
                  {
                    LoadDistortionDiscovering_Single();
                  }
                CurrentDlg=hDlg;
                DisplayDistortionDiscovering_SingleParams();
                EditProc=(WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg,IDC_DATASET_NAME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
                SetWindowLongPtr(GetDlgItem(hDlg,IDC_ELAPSED_TIME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
                SetWindowLongPtr(GetDlgItem(hDlg,IDC_REMAINING_TIME), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
                SetWindowLongPtr(GetDlgItem(hDlg,IDC_PROGRESS_PERCENT), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
                SetWindowLongPtr(GetDlgItem(hDlg,IDC_STATUS), GWLP_WNDPROC, (LONG_PTR)ReadOnlyEditControl);
                if(discovered)
                  {
                    hCurrentCursor=hArrowCursor;
                    SetCursor(hCurrentCursor);
                    return 0;
                  }
                changed = discovered = FALSE;
                GetClientRect(hDlg, &rctNR);
                rctIC=rctNR;
                rctDI=rctNR;
                rctNR.right-=5;
                rctNR.left=rctNR.right-NRGRAPHWIDTH+1;
                rctNR.top=3;
                rctNR.bottom=rctNR.top+NRGRAPHHIEGHT+1;
                rctNRPlot.top=rctNR.top+29;
                rctNRPlot.right=rctNR.right-11;
                rctNRPlot.bottom=rctNRPlot.top+NRPLOTHEIGHT;
                rctNRPlot.left=rctNRPlot.right-NRPLOTWIDTH;
                rctIC.top=rctNR.bottom+28;
                rctIC.bottom=rctIC.top+155;
                rctIC.left+=5;
                rctIC.right-=5;
                rctICPlot.left=rctIC.left+5;
                rctICPlot.right=rctIC.right-5;
                rctICPlot.top=rctIC.top+3;
                rctICPlot.bottom=rctICPlot.top+150;
                rctDI.top=rctIC.bottom+5;
                rctDI.bottom=rctDI.top+155;
                rctDI.left+=5;
                rctDI.right-=5;
                rctDIPlot.left=rctDI.left+5;
                rctDIPlot.right=rctDI.right-5;
                rctDIPlot.top=rctDI.top+3;
                rctDIPlot.bottom=rctDIPlot.top+150;
                //if(hRgnNR==NULL)
                //   hRgnNR=CreateRectRgn(rctNRPlot.left-2, rctNRPlot.top-2,
                //                         rctNRPlot.right+2,  rctNRPlot.bottom);
                DistortionDiscovering_SingleNow=Aborted=
                DistortionDiscovering_SingleSyn2Now=FALSE;
                if(DistortionDiscovering_SingleLoaded)
                  {
                    discovered=TRUE;
                    DistortionDiscovering_SingleLoaded=FALSE;
                    CalcHeatMaps();
                    InvalidateRect(hDlg, 0, 0);
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
		switch (wmId)
		{
		case IDC_APPLY:
                     if(!DistortionDiscovering_SingleSyn2Now)
                       {
                         EnableWindow(GetDlgItem(hDlg, ID_CLOSE), 0);
                         EnableWindow(GetDlgItem(hDlg, IDC_APPLY), 0);
                         EnableDistortionDiscovering_SingleDlgItems(0);
                         hCurrentCursor=hWaitCursor;
                         crsr=SetCursor(hWaitCursor);
                         SetDlgItemText(hDlg, IDCANCEL, "Abort");
                         SaveDistortionDiscovering_SingleChanges();
                         LowLight(hDlg, RectsPlotDD, CountOf(RectsDD));
                         InvalidateRect(hDlg, 0, 0);
                         GetDumps(RectsDD, CountOf(RectsDD));
                       }
                     if(DistortionDiscovering_SingleSyn2Now
                        || CheckSyn2File()
                       )
                       {
                         DistortionDiscovering_SingleSyn2Now=FALSE;
                         DistortionDiscovering_SingleNow=TRUE;
                         hDistortionDiscovering_SingleThread=(HANDLE)_beginthreadex(0, 0,
                           (ThreadAPI)DistortionDiscovering_SingleThread,
                            0, 0, &ThreadID);
                       }
                     return 0;
                case IDC_LEFT:
                     if(xEnd<30)
                       {
                         xStart++;
                         xEnd++;
                         InvalidateRect(hDlg, 0, 0);
                       }
                     return 0;
                case IDC_RIGHT:
                     if(xStart>0)
                       {
                         xStart--;
                         xEnd--;
                         InvalidateRect(hDlg, 0, 0);
                       }
                     return 0;
                case IDC_MINUS:
                     if(xW<30)
                       {
                         xW++;
                         if(36-xStart<xEnd)
                           {
                             xEnd++;
                             if(xEnd>30)
                               {
                                 xEnd--;
                                 xStart--;
                               }
                           }
                         else
                           {
                             xStart--;
                             if(xStart<0)
                               {
                                 xStart++;
                                 xEnd++;
                               }
                           }
                         CalcHeatMaps();
                         InvalidateRect(hDlg, 0, 0);
                       }
                     return 0;
                case IDC_PLUS:
                     if(xW>10)
                       {
                         xW--;
                         if(36-xStart<xEnd&&xEnd>0)
                            xEnd--;
                         else xStart++;
                         CalcHeatMaps();
                         InvalidateRect(hDlg, 0, 0);
                       }
                     return 0;

                case IDC_FOCUS_ON_BOTH:
                case IDC_FOCUS_ON_FN:
                case IDC_FOCUS_ON_FP:
                     if(wmEvent==BN_CLICKED)
                     if(DistortionDiscovering_SingleParamsChanged(wmId))
                        InvalidateRect(hDlg,0,0);
                     return 0;
                //case IDC_MIN_I:
                //case IDC_MIN_I_CORR:
                //case IDC_MIN_MIS:
                //case IDC_MAX_MIS_RATIO:
                //case IDC_MAX_MIS_TOTAL:
                //case IDC_MIN_PRECISION:
                //     if(wmEvent==EN_KILLFOCUS)
                //        DistortionDiscovering_SingleParamsChanged(wmId);
                //     return 0;
                case IDC_PRINT_NR:
                     //if(DialogBox(hInst, MAKEINTRESOURCE(IDD_PLOT_AREA_DIAMETER), hDlg, PlotAreaDiameter))
                     SaveRectToFile(rctNR, NR_Fname[FocusOn], DrawNR, 1);
                     return 0;
                case ID_CLOSE:
                        EndDialog(hDlg, 0);
                        return 0;
		case IDCANCEL:
			if(DistortionDiscovering_SingleSyn2Now
                           || DistortionDiscovering_SingleNow
                          )
                          {
                            if(MessageBox(hDlg, "Abort Testing?",
                                                "Interruption...",
                                                MB_OKCANCEL|MB_ICONHAND|MB_DEFBUTTON2
                                         )!=IDOK
                               || !DistortionDiscovering_SingleNow //in case user didn't respond to messagebox until end Discovering
                              )
                               return 0;
                            SetDlgItemText(hDlg, IDCANCEL, "Cancel");
                            ds->Abort();
                            return 0;
                          }
			EndDialog(hDlg, 0);
			return 0;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
                PaintDistortionDiscovering_Single(hdc);
		EndPaint(hDlg, &ps);
		return 0;
	}
  return (INT_PTR)FALSE;
}

unsigned int WINAPI DistortionDiscovering_SingleThread()
{
  ds->SaveResults=true;
  ds->Calc_nParts(ds->nFactors);
  ProgressSize=ds->nParts;
  InitProgress();
  Status("Discovering...");
  Aborted = !(ds->Calc_1Factor(Progress));
  if(Aborted)
    {
      EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDCANCEL), 1);
      EnableDistortionDiscovering_SingleDlgItems(1);
      Aborted=FALSE;
      if(discovered)
        {
          LoadDistortionDiscovering_Single();
          if(DistortionDiscovering_SingleLoaded)
            {
              discovered=TRUE;
              CalcHeatMaps();
              InvalidateRect(CurrentDlg, 0, 0);
              DistortionDiscovering_SingleLoaded=FALSE;
            }
        }
      SendMessage(GetDlgItem(CurrentDlg, IDC_PROGRESS_BAR), PBM_SETPOS, (WPARAM) 0, 0);
      SetDlgItemText(CurrentDlg, IDC_PROGRESS_PERCENT,"");
      SetDlgItemText(CurrentDlg, IDC_ELAPSED_TIME,"");
      SetDlgItemText(CurrentDlg, IDC_REMAINING_TIME,"");
    }
  else
    {
      discovered=TRUE;
      CalcHeatMaps();
      Status("Saving...");
      ds->SaveDist_1Factor("Dist_Single.txt");
      ds->SaveDistortionDiscovering_SingleParamFile("DistortionDiscovering_SingleParameters.txt");
      CalcFreq();
      EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      EnableDistortionDiscovering_SingleDlgItems(1);
      SetDlgItemText(CurrentDlg, IDCANCEL, "Cancel");
    }
  DistortionDiscovering_SingleNow=FALSE;
  Status("");
  hCurrentCursor=crsr;
  //SetCursor(crsr);
  SendMessage(CurrentDlg, WM_SETCURSOR, 0,0);
  InvalidateRect(CurrentDlg, 0, 0);
  CloseHandle(hDistortionDiscovering_SingleThread);
  _endthreadex(0);
  return 0;
}

unsigned int WINAPI DistortionDiscovering_SingleSyn2Thread()
{
  ds->Calc_nParts(ds->TriangularMatrixSize(ds->nFactors));
  ProgressSize=ds->nParts;
  InitProgress();
  Status("Calculating Syn2...");
  Aborted = !(Syn2Found=ds->CalcSyn2(Progress));
  if(Aborted)
    {
      EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      EnableWindow(GetDlgItem(CurrentDlg, IDCANCEL), 1);
      EnableDistortionDiscovering_SingleDlgItems(1);
      if(discovered)
        {
          LoadDistortionDiscovering_Single();
          if(DistortionDiscovering_SingleLoaded)
            {
              discovered=TRUE;
              CalcHeatMaps();
              InvalidateRect(CurrentDlg, 0, 0);
              DistortionDiscovering_SingleLoaded=FALSE;
            }
        }
      SendMessage(GetDlgItem(CurrentDlg, IDC_PROGRESS_BAR), PBM_SETPOS, (WPARAM) 0, 0);
      SetDlgItemText(CurrentDlg, IDC_PROGRESS_PERCENT,"");
      SetDlgItemText(CurrentDlg, IDC_ELAPSED_TIME,"");
      SetDlgItemText(CurrentDlg, IDC_REMAINING_TIME,"");
      hCurrentCursor=crsr;
      //SetCursor(crsr);
      SendMessage(CurrentDlg, WM_SETCURSOR, 0,0);
      DistortionDiscovering_SingleSyn2Now=FALSE;
      Status("");
      InvalidateRect(CurrentDlg, 0, 0);
    }
  else
    {
      SendMessage(CurrentDlg, WM_COMMAND, (WPARAM) IDC_APPLY, 0);
      //EnableWindow(GetDlgItem(CurrentDlg, ID_CLOSE), 1);
      //EnableWindow(GetDlgItem(CurrentDlg, IDC_APPLY), 1);
      //EnableDistortionDiscovering_SingleDlgItems(1);
      //SetDlgItemText(CurrentDlg, IDCANCEL, "Cancel");
    }
  CloseHandle(hDistortionDiscovering_SingleSyn2Thread);
  _endthreadex(0);
  return 0;
}

void CopyVertexColor(TRIVERTEX& vDst, TRIVERTEX& vSrc)
{
  vDst.Red = vSrc.Red;
  vDst.Green = vSrc.Green;
  vDst.Blue = vSrc.Blue;
}
inline bool NotWhite(TRIVERTEX& v)
{
  return v.Red<0xFF00||v.Green<0xFF00||v.Blue<0xFF00;
}
inline ULONG Vertex(int i, int j)
{
  return i*VERTICESWIDTH+j;
}

void InitHeatMaps()
{
  int i;
  HDC hdc1=GetDC(0);

  for(i=0; i<3; i++)
      hHeatMaps[i]=CreateCompatibleBitmap(hdc1, NRPLOTWIDTH*3, NRPLOTHEIGHT);
  ReleaseDC(CurrentDlg, hdc1);
}
void DeleteHeatMaps()
{
  for(int i=0; i<3; i++)
   if(hHeatMaps[i]!=NULL)
      DeleteObject(hHeatMaps[i]);
}
void CalcHeatMaps()
{
  int i, j, k, l;
  ULONG nMesh[3];
  HDC hdc=GetDC(0), hdc1=CreateCompatibleDC(hdc);
  REAL w=(NRPLOTWIDTH*30.0/xW)/(VERTICESWIDTH-1),
       h=(REAL)NRPLOTHEIGHT/(VERTICESHEIGHT-1), xx, yy;
  RECT rct={0,0,Round(NRPLOTWIDTH*30.0/xW)+1, NRPLOTHEIGHT+1}, rctblk;

  for(l=0; l<3; l++)
     {
       SelectObject(hdc1, hHeatMaps[l]);
       FillRect(hdc1, &rct, hWhiteBrush);
       if(discovered)
         {
           REAL lgbase=log(pow(ds->MaxHeatMap+1, 1/255.0));

           for(i=1; i<VERTICESHEIGHT-1; i++)
              {
                Verticese[l][i][0].x=0;
                Verticese[l][i][0].y=Verticese[l][i][VERTICESWIDTH-1].y=NRPLOTHEIGHT-Round(i*h);
                Verticese[l][i][VERTICESWIDTH-1].x=NRPLOTWIDTH;
                Verticese[l][i][0].Alpha=
                Verticese[l][i][VERTICESWIDTH-1].Alpha=0xFF00;
              }
           for(j=0; j<VERTICESWIDTH; j++)
              {
                Verticese[l][0][j].x=
                Verticese[l][VERTICESHEIGHT-1][j].x=Round(j*w);
                Verticese[l][0][j].y=NRPLOTHEIGHT;
                Verticese[l][VERTICESHEIGHT-1][j].y=0;
                Verticese[l][0][j].Alpha=
                Verticese[l][VERTICESHEIGHT-1][j].Alpha=0xFF00;
              }
           for(i=1,yy=h/2.0; i<VERTICESHEIGHT-1; i++,yy+=h)
           for(j=1,xx=w/2.0; j<VERTICESWIDTH-1; j++,xx+=w)
              {
                Verticese[l][i][j].x=xx;
                Verticese[l][i][j].y=NRPLOTHEIGHT-yy;
                Verticese[l][i][j].Alpha=0xFF00;
              }
           for(i=0; i<HEATMAPHEIGHT; i++)
              {
                for(j=0; j<HEATMAPWIDTH; j++)
                   {
                     if(l==0)
                        k=ALLLONG(ds->Syn2_Score_HeatMap[i][j]);
                     else
                        k=XLONG(ds->Syn2_Score_HeatMap[i][j], l-1);
                     k=Round(log((REAL)(k+1))/lgbase);
                     if(k<0)
                        k=0;
                     if(k>255)
                        k=255;
                     k^=255;
                     k<<=8;
                     if(l==0) //Both: black
                       {
                         Verticese[l][j+1][i+1].Red=
                         Verticese[l][j+1][i+1].Green=
                         Verticese[l][j+1][i+1].Blue=k;
                       }
                     else if(l==1) // FP: BLue
                       {
                         Verticese[l][j+1][i+1].Red=
                         Verticese[l][j+1][i+1].Green=k;
                         Verticese[l][j+1][i+1].Blue=0xFF00;
                       }
                     else //if(l==2) FN: Red
                       {
                         Verticese[l][j+1][i+1].Red=0xFF00;
                         Verticese[l][j+1][i+1].Green=
                         Verticese[l][j+1][i+1].Blue=k;
                       }
                     Verticese[l][j+1][i+1].Alpha=0xFF00;
                   }
              }
           CopyVertexColor(Verticese[l][0][0],Verticese[l][1][1]);
           CopyVertexColor(Verticese[l][0][VERTICESWIDTH-1],Verticese[l][1][VERTICESWIDTH-2]);
           CopyVertexColor(Verticese[l][VERTICESHEIGHT-1][0],Verticese[l][VERTICESHEIGHT-2][1]);
           CopyVertexColor(Verticese[l][VERTICESHEIGHT-1][VERTICESWIDTH-1],Verticese[l][VERTICESHEIGHT-2][VERTICESWIDTH-2]);
           for(i=1; i<VERTICESHEIGHT-1; i++)
              {
                CopyVertexColor(Verticese[l][i][0],Verticese[l][i][1]);
                CopyVertexColor(Verticese[l][i][VERTICESWIDTH-1],Verticese[l][i][VERTICESWIDTH-2]);
              }
           for(j=1; j<VERTICESWIDTH-1; j++)
              {
                CopyVertexColor(Verticese[l][0][j],Verticese[l][1][j]);
                CopyVertexColor(Verticese[l][VERTICESHEIGHT-1][j],Verticese[l][VERTICESHEIGHT-2][j]);
              }
           nMesh[l]=0;
           for(i=1; i<VERTICESHEIGHT; i++)
           for(j=1; j<VERTICESWIDTH; j++)
              {
                if(   NotWhite(Verticese[l][i][j])
                   || NotWhite(Verticese[l][i-1][j-1])
                   || NotWhite(Verticese[l][i-1][j])
                  )
                  {
                    Mesh[l][nMesh[l]].Vertex1=Vertex(i,j);
                    Mesh[l][nMesh[l]].Vertex2=Vertex(i-1,j-1);
                    Mesh[l][nMesh[l]].Vertex3=Vertex(i-1,j);
                    nMesh[l]++;
                  }
                if(   NotWhite(Verticese[l][i][j])
                   || NotWhite(Verticese[l][i-1][j-1])
                   || NotWhite(Verticese[l][i][j-1])
                  )
                  {
                    Mesh[l][nMesh[l]].Vertex1=Vertex(i,j);
                    Mesh[l][nMesh[l]].Vertex2=Vertex(i-1,j-1);
                    Mesh[l][nMesh[l]].Vertex3=Vertex(i,j-1);
                    nMesh[l]++;
                  }
              }
           GdiGradientFill(hdc1, (TRIVERTEX*)Verticese[l],
                           VERTEXARRAYSIZE,
                           Mesh[l], nMesh[l],
                           GRADIENT_FILL_TRIANGLE);
         }
     }
  ReleaseDC(CurrentDlg, hdc);
  DeleteDC(hdc1);
}

void LoadDistortionDiscovering_Single()
{
  if(DistortionDiscovering_SingleLoaded=
        ds->LoadSyn2_Score_HeatMap()
     && ds->LoadIFreq_1F()
    )
      CalcFreq();
}

void CalcFreq()
{
  int i;

  MaxIFreq = MaxDistFreq = MaxICorrFreq = 0;
  for(i=0; i<MAXFREQ; i++)
     {
       if(MaxIFreq<ds->IFreq_1F[i])
          MaxIFreq=ds->IFreq_1F[i];
       if(MaxICorrFreq<LOWLONG(ds->ICorrFreq_1F[i]))
          MaxICorrFreq=LOWLONG(ds->ICorrFreq_1F[i]);
       if(MaxICorrFreq<HIGHLONG(ds->ICorrFreq_1F[i]))
          MaxICorrFreq=HIGHLONG(ds->ICorrFreq_1F[i]);
       if(MaxDistFreq<LOWLONG(ds->DistFreq_1F[i]))
          MaxDistFreq=LOWLONG(ds->DistFreq_1F[i]);
       if(MaxDistFreq<HIGHLONG(ds->DistFreq_1F[i]))
          MaxDistFreq=HIGHLONG(ds->DistFreq_1F[i]);
     }
  REAL Ratio=FreqBarMax/log(Max(MaxICorrFreq,MaxIFreq));
  for(i=0; i<MAXFREQ; i++)
     {
       BarIFreq[i]=log(ds->IFreq_1F[i])*Ratio;
       BarICorrFreq[1][i]=log(LOWLONG(ds->ICorrFreq_1F[i]))*Ratio;
       BarICorrFreq[2][i]=log(HIGHLONG(ds->ICorrFreq_1F[i]))*Ratio;
       BarDistFreq[1][i]=LOWLONG(ds->DistFreq_1F[i])*FreqBarMax/(REAL)MaxDistFreq;
       BarDistFreq[2][i]=HIGHLONG(ds->DistFreq_1F[i])*FreqBarMax/(REAL)MaxDistFreq;
     }
}

void DistortionDiscovering_Single()
{
  crsr=hCurrentCursor=hWaitCursor;
  SetCursor(hCurrentCursor);
  ds->FrequencyOnly=false;
  DistortionDiscovering_SingleLoaded=FALSE;
  DialogBox(hInst, MAKEINTRESOURCE(IDD_DISTORTION_DISCOVERING_SINGLE), hMainWnd, DistortionDiscovering_SingleProc);
  hCurrentCursor=hArrowCursor;
  SetCursor(hCurrentCursor);
}
void SaveDistortionDiscovering_SingleChanges()
{
  CollectDistortionDiscovering_SingleParams();
  ds->SaveDistortionDiscovering_SingleParamFile("DistortionDiscovering_SingleParameters.txt");
}
//void SaveDistortionDiscovering_SingleParams()
//{
//  ds->SaveDistortionDiscovering_SingleParamFile("DistortionDiscovering_SingleParameters.txt");
//  changed=FALSE;
//}

void DisplayDistortionDiscovering_SingleParams(int Item)
{
  char Buf[64];
  static int IDC[3]={IDC_FOCUS_ON_BOTH,
                     IDC_FOCUS_ON_FP,
                     IDC_FOCUS_ON_FN
                    };
  if(Item==IDC_DATASET_NAME||Item==0)
      SetDlgItemText(CurrentDlg, IDC_DATASET_NAME, DatasetName);
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
}
void EnableDistortionDiscovering_SingleDlgItems(BOOL e)
{
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
  EnableWindow(GetDlgItem(CurrentDlg, IDC_PRINT_NR), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_LEFT), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_RIGHT), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_MINUS), e);
  EnableWindow(GetDlgItem(CurrentDlg, IDC_PLUS), e);
}
BOOL CollectDistortionDiscovering_SingleParams(int Item)
{
  char Buf[64];
  BOOL ch=FALSE;
  int i;
  REAL x;

  //if(Item==IDC_MIN_I || Item==0)
  //  {
  //    GetDlgItemText(CurrentDlg, IDC_MIN_I, Buf, sizeof(Buf));
  //    x=atof(Buf);
  //    if(x!=ds->MinI)
  //      {
  //        ds->MinI=x;
  //        ch=TRUE;
  //      }
  //    if(Item!=0) return ch;
  //  }
  //if(Item==IDC_MIN_I_CORR || Item==0)
  //  {
  //    GetDlgItemText(CurrentDlg, IDC_MIN_I_CORR, Buf, sizeof(Buf));
  //    x=atof(Buf);
  //    if(x!=ds->MinICorr)
  //      {
  //        ds->MinICorr=x;
  //        ch=TRUE;
  //      }
  //    if(Item!=0) return ch;
  //  }
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
  //if(Item==IDC_FOCUS_ON_FP|| Item==IDC_FOCUS_ON_FN || Item==IDC_FOCUS_ON_BOTH || Item==0)
  //  {
  //    if(BST_CHECKED==IsDlgButtonChecked(CurrentDlg, IDC_FOCUS_ON_FP))
  //       i=Dataset::FP;  
  //    else if(BST_CHECKED==IsDlgButtonChecked(CurrentDlg, IDC_FOCUS_ON_FN))
  //       i=Dataset::FN;
  //    else i=Dataset::Both;
  //    if(i!=FocusOn)
  //       {
  //         FocusOn=(Dataset::_FocusOn)i;
  //         ch=TRUE;
  //         InvalidateRect(CurrentDlg,0,0);
  //       }
  //    if(Item!=0) return ch;
  //  }
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
  return ch;
}
BOOL DistortionDiscovering_SingleParamsChanged(int Item)
{
  char Buf[64];
  BOOL success, ch=FALSE;
  int i;
  REAL x;

  if(Item==IDC_MIN_I|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_I, Buf, sizeof(Buf));
      if(ds->MinI!=atof(Buf))
        {
          ch=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  if(Item==IDC_MIN_I_CORR|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_I_CORR, Buf, sizeof(Buf));
      if(ds->MinICorr!=atof(Buf))
        {
          ch=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  if(Item==IDC_MIN_MIS|| Item==0)
    {
      if(ds->MinMis!=GetDlgItemInt(CurrentDlg, IDC_MIN_MIS,&success,1))
        {
          ch=TRUE;
          if(Item!=0)
             return ch;
        }
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
        {
          ch=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  if(Item==IDC_MAX_MIS_TOTAL|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MAX_MIS_TOTAL, Buf, sizeof(Buf));
      if(ds->MaxMisTotal!=atof(Buf))
        {
          ch=TRUE;
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
  if(Item==IDC_MIN_PRECISION|| Item==0)
    {
      GetDlgItemText(CurrentDlg, IDC_MIN_PRECISION, Buf, sizeof(Buf));
      if(ds->MinPrecision!=atof(Buf))
        {
          ch=TRUE;
          if(Item!=0)
             return ch;
        }
    }
  return FALSE;
}
BOOL CheckSyn2File()
{
  UINT ThreadID=0;
  if(!(Syn2Found=CheckFileExists(ds->SynBinFname)))
    {
      if(MessageBox(CurrentDlg, "File \"Syn2.bin\" does not exist!\n"
                    "It's important for the Non-Redundancy graph."
                    "Do you want to calculate Syn2?",
                    "File \"Syn2.bin\" not found",MB_YESNO|MB_ICONINFORMATION
                   )==IDYES
        )
        {
          DistortionDiscovering_SingleSyn2Now=TRUE;
          InvalidateRect(CurrentDlg, 0, 0);
          hDistortionDiscovering_SingleSyn2Thread=(HANDLE)_beginthreadex(0, 0,
                       (ThreadAPI)DistortionDiscovering_SingleSyn2Thread,
                        0, 0, &ThreadID);
          return FALSE;
        }
    }
  return TRUE;
}

void DrawNR(HDC hdc, REAL scale)
{
  int i, j, is, ie;
  BOOL Screen=scale<0;
  REAL Scale=Screen?1.0:scale;
  REAL WIDTH=NRPLOTWIDTH/(REAL)xW;
  REAL x1, y1, x2, y2;
  RECT rctNRLegend=rctNRPlot, rct=rctNRPlot;
  char *lgnd[]={"0","0.1","0.2","0.3","0.4","0.5",
                "0.6","0.7","0.8","0.9","1"}, Buf[64];
  char *lgnd5[]={"-2","-1.5","-1","-0.5","0","0.5","1"};
  int len5[]={2,4,2,4,1,3,1};

  SetGraphicsMode(hdc, GM_ADVANCED);
  SetMapMode(hdc, MM_TEXT);
  if(!Screen)
     SetViewportOrgEx(hdc, -rctNR.left, -rctNR.top, 0);
  if(DistortionDiscovering_SingleSyn2Now
     || DistortionDiscovering_SingleNow
    )
    {
      PutDumps(hdc, RectsDD, CountOf(RectsDD));
      return;
    }

  rctNRLegend.top-=5;
  rctNRLegend.bottom+=17;
  rctNRLegend.left-=25;
  rctNRLegend.right+=1;
  SelectObject(hdc, hDkGreyPen);
  SelectObject(hdc, Screen?hLtLtGreyBrush:hWhiteBrush);
  Rectangle(hdc, rctNR.left, rctNR.top, rctNR.right, rctNR.bottom);
  FillRect(hdc, &rctNRLegend, hLegendBkBrush);

  rct.bottom++;
  rct.right++;
  FillRect(hdc, &rct, hWhiteBrush);
  SelectObject(hdc, hGridMinorPen);
  is=((xStart+4)/5)*5;
  ie=(xEnd/5)*5;
  y1=rct.top;
  y2=rct.bottom;
  for(i=is; i<=ie; i+=5)
     {
       x1=rctNRPlot.left+(i-xStart)/*/(REAL(xW))*/*WIDTH;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  x1=rct.left;
  x2=rct.right;
  for(i=1; i<10; i++)
     {
       y1=rctNRPlot.bottom-i/10.0*NRPLOTHEIGHT;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }
  SelectObject(hdc, hGridMajorPen);
  y1=rct.top;
  y2=rct.bottom;
  for(i=is; i<=ie; i+=10)
     {
       x1=rctNRPlot.left+(i-xStart)/*/(REAL(xW))*/*WIDTH;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  x1=rct.left;
  x2=rct.right;
  for(i=2; i<9; i++,i++)
     {
       y1=rctNRPlot.bottom-i/10.0*NRPLOTHEIGHT;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }

  SetBkMode(hdc, TRANSPARENT);
  SelectObject(hdc, hGridMajorPen);
  SetTextColor(hdc, 0);
  SelectObject(hdc, (HFONT)SendMessage(CurrentDlg, WM_GETFONT,0,0));

  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  y1=rctNRPlot.bottom;
  y2=rctNRPlot.bottom+3;
  for(i=is,j=is/5; i<=ie; i+=5,j++)
     {
       x1=rctNRPlot.left+(i-xStart)/*/(REAL(xW))*/*WIDTH;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
       TextOut(hdc, x1, y1+1, lgnd5[j], len5[j]);
     }
  SetTextAlign(hdc, TA_RIGHT|TA_BASELINE);
  x1=rctNRPlot.left-1;
  x2=rctNRPlot.left-4;
  for(i=0; i<=10; i++,i++)
     {
       y1=rctNRPlot.bottom-i/10.0*NRPLOTHEIGHT;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
       TextOut(hdc, x2-1, y1+4, lgnd[i], i==0||i==10?1:3);
     }

  if(discovered)
    {
      HDC hdc1=CreateCompatibleDC(hdc);
      SelectObject(hdc1, hHeatMaps[FocusOn]);
      BitBlt(hdc, rctNRPlot.left,rctNRPlot.top,
                 NRPLOTWIDTH, NRPLOTHEIGHT,
                 hdc1, xStart*WIDTH,0,
                 //xW/30.0*NRPLOTWIDTH,NRPLOTHEIGHT,
                 SRCAND);
      DeleteDC(hdc1);
    }
  SelectObject(hdc, hHighFont);
  SetTextColor(hdc, 0);
  x1=rctNRPlot.left+(NRPLOTWIDTH>>1);
  y1=rctNRLegend.bottom;
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  sprintf_s(Buf, "Marker=Spotter Synergy");
  TextOut(hdc, x1, y1, Buf, 22);
  SelectObject(hdc, hVFont);
  sprintf_s(Buf, "Score");
  x1=rctNRLegend.left-18;
  y1=((rctNRPlot.top+rctNRPlot.bottom)>>1);
  TextOut(hdc, x1, y1, Buf, 5);
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  SetTextColor(hdc, cBlackColor);
  SelectObject(hdc, hTitleFont);
  x1=((rctNR.right+rctNR.left)>>1);
  y1=rctNR.top+5;
  TextOut(hdc, x1, y1, "Spotter is not a Marker", 23);
}
// Paint Distortion Discovering Single
void PaintDistortionDiscovering_Single(HDC hdc)
{
  int i, j, w, W;
  REAL x1, y1, X, x2, y2;
  char *lgnd[]={"0","0.1","0.2","0.3","0.4","0.5",
                "0.6","0.7","0.8","0.9","1"}, Buf[64];
  HBRUSH hBarBrushs[3]={hMagentaBrush, hBlueBrush, hRedBrush};
  COLORREF cBarColors[3]={cMagentaColor, cBlueColor, cRedColor};

  DrawNR(hdc, -1.0);
  SelectObject(hdc, hDkDkGreyPen);
  SelectObject(hdc, hLtLtGreyBrush);
  Rectangle(hdc, rctIC.left, rctIC.top, rctIC.right, rctIC.bottom);
  FillRect(hdc, &rctICPlot, hWhiteBrush);
  MoveToEx(hdc, rctIC.right, rctIC.top, 0);
  LineTo(hdc, rctIC.right, rctIC.bottom);
  LineTo(hdc, rctIC.left, rctIC.bottom);

  Rectangle(hdc, rctDI.left, rctDI.top, rctDI.right, rctDI.bottom);
  FillRect(hdc, &rctDIPlot, hWhiteBrush);
  MoveToEx(hdc, rctDI.right, rctDI.top, 0);
  LineTo(hdc, rctDI.right, rctDI.bottom);
  LineTo(hdc, rctDI.left, rctDI.bottom);
  if(!discovered)
     return;
  SetGraphicsMode(hdc, GM_ADVANCED);
  SetMapMode(hdc, MM_TEXT);
  SetBkMode(hdc, TRANSPARENT);
  SetTextAlign(hdc, TA_CENTER|VTA_TOP);
  SetTextColor(hdc, cBlackColor);

  //IFreq
  SelectObject(hdc, hSmallFont);
  SelectObject(hdc, hGreyBrush);
  SelectObject(hdc, hDkDkGreyPen);
  y2=rctICPlot.bottom-29;
  w=11;
  for(i=0; i<MAXFREQ; i++)
     {
       x1=rctICPlot.left+i*w;
       x2=x1+w-2;
       y1=y2-BarIFreq[i];
       Rectangle(hdc, x1, y1, x2, y2);
       if(!(i&1))
          TextOut(hdc, x1, y2, lgnd[i>>1], i==0?1:3);
     }
  TextOut(hdc, x1+w, y2, "1", 1);
  SelectObject(hdc, hHighFont);
  TextOut(hdc, Round(rctICPlot.left+MAXFREQ*w/2.0),
                y2+12, "Log MI Frequency", 16);
  MoveToEx(hdc, rctICPlot.left, y2, 0);
  LineTo(hdc, rctICPlot.right, y2);
  SelectObject(hdc, hLtLtGreyBrush);
  x1=rctICPlot.left+MAXFREQ*w+7;
  Rectangle(hdc, x1-5, rctIC.top, x1, y2);

  //ICorrFreq
  SelectObject(hdc, hSmallFont);
  X=rctICPlot.left+MAXFREQ*w+7;
  W=18;
  for(j=1; j<3; j++)
     {
       SelectObject(hdc, hBarBrushs[j]);
       for(i=0; i<MAXFREQ; i++)
          {
            x1=X+i*W+(j==2?(W>>1)-1:0);
            x2=x1+(j==0?W-2:(W>>1)-1);
            y1=y2-BarICorrFreq[j][i];
            Rectangle(hdc, x1, y1, x2, y2);
          }
      if(j>0)
         Rectangle(hdc, X+(j==1?20:57), y2+15,
                        X+(j==1?30:67), y2+25);
    }
  for(i=0; i<=MAXFREQ; i++,i++)
      TextOut(hdc, X+i*W, y2, lgnd[i>>1], i==0||i==MAXFREQ?1:3);
  SelectObject(hdc, hHeavyFont);
  TextOut(hdc, X+W*Round(MAXFREQ>>1),
                y2+12, "Log Corrected MI Frequency", 26);
  SetTextColor(hdc, cBlueColor);
  TextOut(hdc, X+9, y2+12, "FP", 2);
  SetTextColor(hdc, cRedColor);
  TextOut(hdc, X+46, y2+12, "FN", 2);

  //DistFreq
  SelectObject(hdc, hSmallFont);
  SetTextColor(hdc, cBlackColor);
  X=rctDIPlot.left;
  y2=rctDIPlot.bottom-29;
  W=29;
  for(j=1; j<3; j++)
     {
       SelectObject(hdc, hBarBrushs[j]);
       for(i=0; i<MAXFREQ; i++)
          {
            x1=X+i*W+(j==2?(W>>1)-1:0);
            x2=x1+(j==0?W-2:(W>>1)-1);
            y1=y2-BarDistFreq[j][i];
            Rectangle(hdc, x1, y1, x2, y2);
          }
       Rectangle(hdc, X+(j==0?105:(j==1?20:57)), y2+15,
                      X+(j==0?117:(j==1?30:67)), y2+25);
    }
  for(i=0; i<=MAXFREQ; i++,i++)
      TextOut(hdc, X+i*W, y2, lgnd[i>>1], i==0||i==MAXFREQ?1:3);
  SelectObject(hdc, hHeavyFont);
  TextOut(hdc, X+W*Round(MAXFREQ>>1),
                y2+12, "Frequencies of Distortion Instance Scores", 41);
  //SetTextColor(hdc, cBarColors[0]);
  //TextOut(hdc, X+90, y2+12, "Both", 4);
  SetTextColor(hdc, cBarColors[1]);
  TextOut(hdc, X+9, y2+12, "FP", 2);
  SetTextColor(hdc, cBarColors[2]);
  TextOut(hdc, X+46, y2+12, "FN", 2);
}