// Distortion.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

static int __cdecl CompDatasetPRec(const void *r1, const void *r2)
{
  int i=strcmp((*((DatasetRec**)r1))->Name, (*((DatasetRec**)r2))->Name);
  return i>0?1:(i<0?-1:0);
}

// Global Variables:
HINSTANCE hInst;
HWND hMainWnd, CurrentDlg;
TCHAR szTitle[MAX_LOADSTRING];		// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];	// the main window class name
Dataset *ds = NULL;
char DataFolder[MAX_PATH+1], DatasetName[MAX_PATH+1],
     DatasetPath[MAX_PATH+1];
DatasetRec **DatasetList=0;
int  DataFolderLen, DatasetPathLen, nDatasets;
int **Predictors_X=NULL, **Predictors_Y=NULL,
    nPredictorsDist=0, nPredictorsNoDist=0,
    SizePredictors=0;
BOOL Loaded, tested, ModelExists;
Dataset::_FocusOn FocusOn=Dataset::Both;
int nKnots, nPoints;
HCURSOR crsr=0, hWaitCursor=LoadCursor(NULL, IDC_WAIT),
        hArrowCursor=LoadCursor(NULL, IDC_ARROW),
        hCurrentCursor=hArrowCursor;
COLORREF LegendBkColor = RGB(233, 233, 243),
         LowLightColor = RGB(17,17,33),
         HighLightColor = RGB(255,55,55),
         cDlgBkColor = RGB(211,211,179),
         cListBkColor= RGB(229,229,219),
         cBlackColor = RGB(0,0,0),
         cDkDkGreyColor= RGB(35,35,35),
         cDkGreyColor= RGB(75,75,79),
         cGreyColor= RGB(127,127,131),
         cLtGreyColor= RGB(179,179,183),
         cLtLtGreyColor= RGB(219,219,223),
         cLtLtLtGreyColor= RGB(245,245,250),
         cLtRedColor= RGB(255,159,159),
         cLtLtRedColor= RGB(255,203,203),
         cLtLtLtRedColor= RGB(255,220,255),
         cRedColor= RGB(235,52,52),
         cDkRedColor= RGB(215,12,12),
         cLtGreenColor= RGB(159,255,159),
         cGreenColor= RGB(52,235,52),
         cDkGreenColor= RGB(12,215,12),
         cLtLtLtBlueColor= RGB(220,220,255),
         cLtLtBlueColor= RGB(203,203,255),
         cLtBlueColor= RGB(159,159,255),
         cBlueColor= RGB(52,52,235),
         cDkBlueColor= RGB(12,12,215),
         cWhiteColor= RGB(255,255,255),
         cMagentaColor= RGB(255,150,255);
HPEN AllPens[50];
HPEN hGreyPlotPen0 =AllPens[0]= CreatePen(PS_SOLID, 0, cGreyColor);
HPEN hBluePlotPen0 =AllPens[1]= CreatePen(PS_SOLID, 0, cBlueColor);
HPEN hRedPlotPen0  =AllPens[2]= CreatePen(PS_SOLID, 0, cRedColor);
HPEN hGreenPlotPen0=AllPens[3]= CreatePen(PS_SOLID, 0, cGreenColor);

HPEN hGreyPlotPen1 =AllPens[4]= CreatePen(PS_SOLID, 1, cGreyColor);
HPEN hBluePlotPen1 =AllPens[5]= CreatePen(PS_SOLID, 1, cBlueColor);
HPEN hRedPlotPen1  =AllPens[6]= CreatePen(PS_SOLID, 1, cRedColor);
HPEN hGreenPlotPen1=AllPens[7]= CreatePen(PS_SOLID, 1, cGreenColor);

HPEN hGreyPlotPen2 =AllPens[8]= CreatePen(PS_SOLID, 2, cGreyColor);
HPEN hBluePlotPen2 =AllPens[9]= CreatePen(PS_SOLID, 2, cBlueColor);
HPEN hRedPlotPen2  =AllPens[10]= CreatePen(PS_SOLID, 2, cRedColor);
HPEN hGreenPlotPen2=AllPens[11]= CreatePen(PS_SOLID, 2, cGreenColor);

HPEN hGreyPlotPen3 =AllPens[12]= CreatePen(PS_SOLID, 3, cGreyColor);
HPEN hBluePlotPen3 =AllPens[13]= CreatePen(PS_SOLID, 3, cBlueColor);
HPEN hRedPlotPen3  =AllPens[14]= CreatePen(PS_SOLID, 3, cRedColor);
HPEN hGreenPlotPen3=AllPens[15]= CreatePen(PS_SOLID, 3, cGreenColor);

HPEN hGreyPlotPen4 =AllPens[16]= CreatePen(PS_SOLID, 4, cGreyColor);
HPEN hBluePlotPen4 =AllPens[17]= CreatePen(PS_SOLID, 4, cBlueColor);
HPEN hRedPlotPen4  =AllPens[18]= CreatePen(PS_SOLID, 4, cRedColor);
HPEN hGreenPlotPen4=AllPens[19]= CreatePen(PS_SOLID, 4, cGreenColor);

HPEN hGreyPlotPen5 =AllPens[20]= CreatePen(PS_SOLID, 5, cGreyColor);
HPEN hBluePlotPen5 =AllPens[21]= CreatePen(PS_SOLID, 5, cBlueColor);
HPEN hRedPlotPen5  =AllPens[22]= CreatePen(PS_SOLID, 5, cRedColor);
HPEN hGreenPlotPen5=AllPens[23]= CreatePen(PS_SOLID, 5, cGreenColor);

HPEN hGreyPlotPen6 =AllPens[24]= CreatePen(PS_SOLID, 6, cGreyColor);
HPEN hBluePlotPen6 =AllPens[25]= CreatePen(PS_SOLID, 6, cBlueColor);
HPEN hRedPlotPen6  =AllPens[26]= CreatePen(PS_SOLID, 6, cRedColor);
HPEN hGreenPlotPen6=AllPens[27]= CreatePen(PS_SOLID, 6, cGreenColor);

HPEN hGreyPlotPens[] ={hGreyPlotPen0, hGreyPlotPen1, hGreyPlotPen2, hGreyPlotPen3, hGreyPlotPen4, hGreyPlotPen5, hGreyPlotPen6};
HPEN hBluePlotPens[] ={hBluePlotPen0, hBluePlotPen1, hBluePlotPen2, hBluePlotPen3, hBluePlotPen4, hBluePlotPen5, hBluePlotPen6};
HPEN hRedPlotPens[]  ={hRedPlotPen0,  hRedPlotPen1,  hRedPlotPen2,  hRedPlotPen3,  hRedPlotPen4,  hRedPlotPen5,  hRedPlotPen6};
HPEN hGreenPlotPens[]={hGreenPlotPen0,hGreenPlotPen1,hGreenPlotPen2,hGreenPlotPen3,hGreenPlotPen4,hGreenPlotPen5,hGreenPlotPen6};

HPEN hGreyPen       =AllPens[28]= CreatePen(PS_SOLID, 0, cGreyColor);
HPEN hDkGreyPen     =AllPens[29]= CreatePen(PS_SOLID, 0, cDkGreyColor);
HPEN hDkDkGreyPen   =AllPens[30]= CreatePen(PS_SOLID, 0, cDkDkGreyColor);
HPEN hLtGreyPen     =AllPens[31]= CreatePen(PS_SOLID, 0, cLtGreyColor);
HPEN hLtLtGreyPen   =AllPens[32]= CreatePen(PS_SOLID, 0, cLtLtGreyColor);
HPEN hLtLtLtGreyPen =AllPens[33]= CreatePen(PS_SOLID, 0, cLtLtLtGreyColor);
HPEN hDkBluePen     =AllPens[34]= CreatePen(PS_SOLID, 0, cDkBlueColor);
HPEN hLtBluePen     =AllPens[35]= CreatePen(PS_SOLID, 0, cLtBlueColor);
HPEN hDkRedPen      =AllPens[36]= CreatePen(PS_SOLID, 0,  cDkRedColor);
HPEN hLtRedPen      =AllPens[37]= CreatePen(PS_SOLID, 0,  cLtRedColor);
HPEN hBlackPen      =AllPens[38]= CreatePen(PS_SOLID, 0,  cBlackColor);
HPEN hBlackPen2     =AllPens[39]= CreatePen(PS_SOLID, 2, cBlackColor);
HPEN hWhitePen      =AllPens[40]= CreatePen(PS_SOLID, 0,  cWhiteColor);
HPEN hWhitePen2     =AllPens[41]= CreatePen(PS_SOLID, 2, cWhiteColor);
HPEN hGreyPen2      =AllPens[42]= CreatePen(PS_SOLID, 2, cGreyColor);
HPEN hLtLtGreyPenDot=AllPens[43]=AllPens[0]= CreatePen(PS_DOT, 0, cLtLtGreyColor);
HPEN hGridMinorPen  =AllPens[44]= CreatePen(PS_SOLID, 0, RGB(231,231,241));
HPEN hGridMajorPen  =AllPens[45]= CreatePen(PS_SOLID, 0, RGB(205,205,195));
HPEN hGridPen       =AllPens[46]= CreatePen(PS_SOLID, 0, RGB(250,250,255));//RGB(218,220,221));
HPEN hLegendPen     =AllPens[47]= CreatePen(PS_SOLID, 0, RGB(0,0,0));
HPEN hLtLtRedPen    =AllPens[37]= CreatePen(PS_SOLID, 0,  cLtLtRedColor);
HPEN hLtLtLtRedPen    =AllPens[37]= CreatePen(PS_SOLID, 0,  cLtLtLtRedColor);

HBRUSH AllBrushes[23];
HBRUSH hWhiteBrush     =AllBrushes[0]= CreateSolidBrush(cWhiteColor);
HBRUSH hBkBrush        =AllBrushes[1]= CreateSolidBrush(RGB(245,245,239));
HBRUSH hGreyBrush      =AllBrushes[2]= CreateSolidBrush(cGreyColor);
HBRUSH hDkGreyBrush    =AllBrushes[3]= CreateSolidBrush(cDkGreyColor);
HBRUSH hDkDkGreyBrush  =AllBrushes[4]= CreateSolidBrush(cDkDkGreyColor);
HBRUSH hLtGreyBrush    =AllBrushes[5]= CreateSolidBrush(cLtGreyColor);
HBRUSH hLtLtGreyBrush  =AllBrushes[6]=  CreateSolidBrush(cLtLtGreyColor);
HBRUSH hLtLtLtGreyBrush=AllBrushes[7]=  CreateSolidBrush(cLtLtLtGreyColor);
HBRUSH hDkBlueBrush    =AllBrushes[8]= CreateSolidBrush(cDkBlueColor);
HBRUSH hBlueBrush      =AllBrushes[9]= CreateSolidBrush(cBlueColor);
HBRUSH hLtBlueBrush    =AllBrushes[10]= CreateSolidBrush(cLtBlueColor);
HBRUSH hLtLtBlueBrush  =AllBrushes[11]= CreateSolidBrush(cLtLtBlueColor);
HBRUSH hLtLtLtBlueBrush=AllBrushes[12]= CreateSolidBrush(cLtLtLtBlueColor);
HBRUSH hDkRedBrush     =AllBrushes[13]= CreateSolidBrush(cDkRedColor);
HBRUSH hRedBrush       =AllBrushes[14]= CreateSolidBrush(cRedColor);
HBRUSH hLtRedBrush     =AllBrushes[15]= CreateSolidBrush(cLtRedColor);
HBRUSH hLtLtRedBrush   =AllBrushes[16]= CreateSolidBrush(cLtLtRedColor);
HBRUSH hLtLtLtRedBrush =AllBrushes[17]= CreateSolidBrush(cLtLtLtRedColor);
HBRUSH hMagentaBrush   =AllBrushes[18]= CreateSolidBrush(cMagentaColor);
HBRUSH hDlgBkBrush     =AllBrushes[19]= CreateSolidBrush(cDlgBkColor);
HBRUSH hListBkBrush    =AllBrushes[20]= CreateSolidBrush(cListBkColor);
HBRUSH hLegendBkBrush  =AllBrushes[21]= CreateSolidBrush(LegendBkColor);
HBRUSH hPlotBkBrush    =AllBrushes[22]= CreateSolidBrush(RGB(245,245,239));

HFONT AllFonts[8];
HFONT hTextFont  =AllFonts[0]= CreateFont(5, 3, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "MS Sans Serif");
HFONT hDataFont  =AllFonts[1]= CreateFont(20, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");
HFONT hSmallFont =AllFonts[2]= CreateFont(14, 0, 0, 0, FW_LIGHT, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");
HFONT hDataHiFont =AllFonts[3]= CreateFont(22, 0, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "calibri");
HFONT hHighFont   =AllFonts[4]= CreateFont(0, 6, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "MS Sans Serif");
HFONT hHeavyFont  =AllFonts[5]= CreateFont(0, 6, 0, 0, FW_HEAVY, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "MS Sans Serif");
HFONT hVFont      =AllFonts[6]= CreateFont(0, 6, 900, 900, FW_BOLD, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "MS Sans Serif");
HFONT hTitleFont  =AllFonts[7]= CreateFont(20, 20, 0, 0, FW_BLACK, 0, 0, 0, ANSI_CHARSET,
                         OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS,
                         PROOF_QUALITY, DEFAULT_PITCH|FF_MODERN, "MS Sans Serif");
BOOL Aborted=FALSE, changed=FALSE;
int ProgressSize, ProgressCounter;
time_t StartRunTime, ElapsedTime, RemainingTime;
char BigBuffer[8192], FindBuf[128];
HDC hdc_Dump;
HBITMAP hbmp_Dump;
int CellWidth=85, CellHeight=23,
    ColZeroWidth=139, HeaderHeight=117, StatusBarHeight=27,
    CurrentSelection=0,
    hPos=0, hPage=10, hMin=0, hMax=1,
    vPos=0, vPage=10, vMin=0, vMax=1;
REAL Scale=2.0;
int xSortF[]={ColZeroWidth-77,ColZeroWidth-77,
              ColZeroWidth-15,ColZeroWidth-15},
    ySortF[]={HeaderHeight+CellHeight+15,HeaderHeight+CellHeight+9,
              HeaderHeight+CellHeight+15,HeaderHeight+CellHeight+9
             },
    dx[2][2]={{4,8},{4,8}},
    dy[2][2]={{4,0},{-4,0}};
int *Index=0, CurrentIndex=0;
WNDPROC EditProc;

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	//UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	int i, j, k;
        MSG msg;
        COLORREF c;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BML, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if(!InitInstance (hInstance, nCmdShow))
          {
            return FALSE;
          }

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DISTORTION));
        GetDataFolder();
        DatasetList = NewMatrix<DatasetRec>(MAXLIST, 1);
        InitCommonControls();
        HDC hdc=GetDC(hMainWnd);
        hbmp_Dump=CreateCompatibleBitmap(hdc, 1000, 1000);
        hdc_Dump=CreateCompatibleDC(hdc);
        SelectObject(hdc_Dump, hbmp_Dump);
        ReleaseDC(hMainWnd, hdc);
        InitHeatMaps();
        SendMessage(hMainWnd, WM_SETCURSOR, 0,0);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

        CloseDataset();
        DeleteHeatMaps();
        for(i=0; i<CountOf(AllPens); i++)
            DeleteObject(AllPens[i]);
        for(i=0; i<CountOf(AllBrushes); i++)
            DeleteObject(AllBrushes[i]);
        for(i=0; i<CountOf(AllFonts); i++)
            DeleteObject(AllFonts[i]);
        if(Predictors_X)
           DeleteMatrix(Predictors_X, 3);
        if(Predictors_Y)
        DeleteMatrix(Predictors_Y, 3);
        if(hRgnTop!=NULL)
           DeleteObject(hRgnTop);
        if(hRgnROC!=NULL)
           DeleteObject(hRgnROC);
        if(hRgnSig!=NULL)
           DeleteObject(hRgnSig);
        if(hRgnNR!=NULL)
           DeleteObject(hRgnTop);
        if(hRgnIC!=NULL)
           DeleteObject(hRgnROC);
        if(hRgnDI!=NULL)
           DeleteObject(hRgnROC);
	DeleteMatrix(DatasetList, MAXLIST);
        DeleteObject(hbmp_Dump);
        DeleteDC(hdc_Dump);
        return (int) msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));//IDI_DISTORTION));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(hLtLtGreyBrush);//(COLOR_WINDOW+1)
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DISTORTION);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));//

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable
   hMainWnd=CreateWindow(szWindowClass,szTitle,
            WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
            CW_USEDEFAULT,0,CW_USEDEFAULT,0,NULL,NULL,
            hInstance, NULL);

   if (!hMainWnd)
   {
      return FALSE;
   }

   ShowWindow(hMainWnd, nCmdShow);
   UpdateWindow(hMainWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  int wmId, wmEvent, zDelta=0;
  int w, h, i, j, x, y;
  SCROLLINFO ScrlInf;
  PAINTSTRUCT ps;
  HDC hdc;
  BOOL Ctrl;
  
  switch (message)
  {
  //case WM_CREATE:
  //     w=((CREATESTRUCT*)lParam)->cx;
  //     h=((CREATESTRUCT*)lParam)->cy;
  //     return 0;
  case WM_HSCROLL:
       ScrlInf.cbSize=sizeof(SCROLLINFO);
       ScrlInf.fMask=SIF_POS;
       GetScrollInfo(hWnd, SB_HORZ, &ScrlInf);
       switch(LOWORD(wParam))
             {
               case SB_LEFT: ScrlInf.nPos=0;
                    break;
               case SB_RIGHT: ScrlInf.nPos=hMax;
                    break;
               case SB_LINELEFT: ScrlInf.nPos--;
                    break;
               case SB_LINERIGHT: ScrlInf.nPos++;
                    break;
               case SB_PAGELEFT: ScrlInf.nPos-=hPage;
                    break;
               case SB_PAGERIGHT: ScrlInf.nPos+=hPage;
                    break;
               case SB_THUMBPOSITION:
               case SB_THUMBTRACK:
                    ScrlInf.nPos=HIWORD(wParam);
                    break;
               default: return 0;
             }
       SetScrollInfo(hWnd, SB_HORZ, &ScrlInf,1);
       GetScrollInfo(hWnd, SB_HORZ, &ScrlInf);
       hPos=ScrlInf.nPos;
       InvalidateRect(hWnd, 0, 1);
       return 0;
  case WM_VSCROLL:
       ScrlInf.cbSize=sizeof(SCROLLINFO);
       ScrlInf.fMask=SIF_POS;
       GetScrollInfo(hWnd, SB_VERT, &ScrlInf);
       switch(LOWORD(wParam))
             {
               case SB_TOP: ScrlInf.nPos=0;
                            CurrentSelection=0;
                    break;
               case SB_BOTTOM: ScrlInf.nPos=vMax;
                               CurrentSelection=vPage>0?vPage-1:0;
                    break;
               case SB_LINEUP: ScrlInf.nPos--;
                    break;
               case SB_LINEDOWN: ScrlInf.nPos++;
                    break;
               case SB_PAGEUP: ScrlInf.nPos-=vPage;
                    break;
               case SB_PAGEDOWN: ScrlInf.nPos+=vPage;
                    break;
               case SB_THUMBPOSITION:
               case SB_THUMBTRACK:
                    ScrlInf.nPos=HIWORD(wParam);
                    break;
               default: return 0;
             }
       SetScrollInfo(hWnd, SB_VERT, &ScrlInf,1);
       GetScrollInfo(hWnd, SB_VERT, &ScrlInf);
       //CurrentSelection+=ScrlInf.nPos-vPos;
       //if(CurrentSelection<0)
       //   CurrentSelection=0;
       //else if(CurrentSelection>=vPage)
       //   CurrentSelection=vPage-1;
       vPos = ScrlInf.nPos;
       if(vPos+CurrentSelection>vMax)
          CurrentSelection=vMax-vPos;
       if(CurrentSelection<0)
          CurrentSelection=0;
       InvalidateRect(hWnd, 0, 1);
       return 0;
  case WM_SIZE:
       if(wParam==SIZE_MAXIMIZED||wParam==SIZE_RESTORED)
         {
           w=LOWORD(lParam);
           h=HIWORD(lParam);
           ScrlInf.cbSize=sizeof(SCROLLINFO);
           ScrlInf.fMask=SIF_PAGE|SIF_RANGE|SIF_POS|SIF_DISABLENOSCROLL;
           hPage=ScrlInf.nPage=(w-ColZeroWidth)/CellWidth;
           hMin=vMin=ScrlInf.nMin=0;
           hMax=ScrlInf.nMax=Loaded&&ds!=0?ds->nSamples-1:0;
           if(hPos+hPage-1>hMax)
             {
               hPos=hMax-hPage+1;
               if(hPos<0)
                  hPos=0;
             }
           ScrlInf.nPos = hPos;
           SetScrollInfo(hWnd, SB_HORZ, &ScrlInf,1);
           i=vPos;
           vPage=ScrlInf.nPage=(h-HeaderHeight-StatusBarHeight)/CellHeight-2;
           if(ScrlInf.nPage<0)
              ScrlInf.nPage=0;
           vMax=ScrlInf.nMax=Loaded&&ds!=0?ds->nFactors-1:0;
           if(vPos+vPage-1>vMax)
             {
               vPos=vMax-vPage+1;
               if(vPos<0)
                  vPos=0;
             }
           ScrlInf.nPos = vPos;
           SetScrollInfo(hWnd, SB_VERT, &ScrlInf,1);
           ScrlInf.fMask=SIF_POS;
           GetScrollInfo(hWnd, SB_VERT, &ScrlInf);
           CurrentSelection+=i-vPos;
           if(CurrentSelection>=vPage)
              CurrentSelection=vPage-1;
           if(vPos+CurrentSelection>vMax)
              CurrentSelection=vMax-vPos;
           if(CurrentSelection<0)
              CurrentSelection=0;
         }
       return 0;
  case WM_LBUTTONDOWN:
       x = GET_X_LPARAM(lParam);
       y = GET_Y_LPARAM(lParam);
       for(i=0,j=0; i<4; i++,j^=1)
        if(   x>=xSortF[i]-1 && x<=xSortF[i]+dx[j][1]+1
           && (j?y<=ySortF[i]+1&&y>=ySortF[i]+dy[j][0]-1
                :y>=ySortF[i] &&y<=ySortF[i]+dy[j][0]
              )
          )
           break;
       if(i<4)
         {
           i++;
           if(CurrentIndex==i)
              CurrentIndex=0;
           else CurrentIndex=i;
           SetIndex();
           SendMessage(hWnd, WM_VSCROLL, SB_TOP, 0);
         }
       else
         {
           i=(HIWORD(lParam)-HeaderHeight)/CellHeight-2;
           if(   i!=CurrentSelection && i>=0
              && i+vPos<=vMax && i<vPage
             )
             {
               CurrentSelection=i;
               InvalidateRect(hMainWnd, 0, 1);
             }
         }
       return 0;
  case WM_KEYDOWN:
       Ctrl=(GetKeyState(VK_CONTROL)&0x8000)==0?FALSE:TRUE;
       switch(wParam)
             {
              case 0x46:
                   if(Loaded && Ctrl)
                      SendMessage(hWnd, WM_COMMAND, ID_SEARCH_FIND, 0);
                   break;
              case VK_UP:
                   if(!Ctrl && CurrentSelection>0)
                     {
                       CurrentSelection--;
                       InvalidateRect(hMainWnd, 0, 1);
                     }
                   else if(vPos>0)
                       SendMessage(hMainWnd, WM_VSCROLL, SB_LINEUP, 0);
                   break;
              case VK_DOWN:
                   if(!Ctrl && CurrentSelection<vPage-1
                      && CurrentSelection<vMax
                     )
                     {
                       CurrentSelection++;
                       InvalidateRect(hMainWnd, 0, 1);
                     }
                   else if(vPos+vPage<vMax)
                       SendMessage(hMainWnd, WM_VSCROLL, SB_LINEDOWN, 0);
                   break;
              case VK_LEFT:
                   if(hPos>0)
                   SendMessage(hMainWnd, WM_HSCROLL,
                      Ctrl?SB_PAGELEFT:SB_LINELEFT, 0);
                   break;
              case VK_RIGHT:
                   if(hPos+hPage-1<hMax)
                   SendMessage(hMainWnd, WM_HSCROLL,
                      Ctrl?SB_PAGERIGHT:SB_LINERIGHT, 0);
                   break;
              case VK_HOME:
                   SendMessage(hMainWnd,
                      Ctrl?WM_VSCROLL:WM_HSCROLL, SB_TOP, 0);
                   break;
              case VK_END:
                   SendMessage(hMainWnd, 
                      Ctrl?WM_VSCROLL:WM_HSCROLL, SB_BOTTOM, 0);
                   break;
              case VK_PRIOR:
                   if(Ctrl)
                     {
                       CurrentSelection=0;
                       InvalidateRect(hMainWnd, 0, 1);
                     }
                   else SendMessage(hMainWnd, WM_VSCROLL, SB_PAGEUP, 0);
                   break;
              case VK_NEXT:
                   if(Ctrl)
                     {
                       CurrentSelection=vPage>0?vPage-1:0;
                       InvalidateRect(hMainWnd, 0, 1);
                     }
                   else SendMessage(hMainWnd, WM_VSCROLL, SB_PAGEDOWN, 0);
                   break;
              default: return DefWindowProc(hWnd, message, wParam, lParam);
             }
        return 0;
  case WM_MOUSEWHEEL:
       wmId=GET_KEYSTATE_WPARAM(wParam);
       Ctrl=MK_CONTROL&wmId;
       zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
       if(   zDelta>0 && vPos>0
          || zDelta<0 && vPos<vMax-vPage+1
         )
         {
           if(Ctrl&&vPos>0)
              SendMessage(hMainWnd, WM_VSCROLL, zDelta>0?SB_PAGEUP:SB_PAGEDOWN, 0);
           else
              SendMessage(hMainWnd, WM_VSCROLL, zDelta>0?SB_LINEUP:SB_LINEDOWN, 0);
         }
       return 0;
  case WM_COMMAND:
       wmId    = LOWORD(wParam);
       wmEvent = HIWORD(wParam);
       // Parse the menu selections:
       switch (wmId)
       {
       case ID_SEARCH_FIND:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_FIND), hWnd, FindProc);
            if(FindBuf[0]!=0)
              {
                j=FindIndexStri(FindBuf, ds->FactorNameIndex, ds->FactorNames, ds->nFactors, i);
                if(j!=0&&(CurrentIndex<1||CurrentIndex>2))
                  {
                    CurrentIndex=1;
                    SetIndex();
                  }
                if(j<0 && i>0)
                   i--;
                int ii=i, *indx;
                if(CurrentIndex!=1)
                  {
                    ii=ds->FactorNameIndex[i];
                    if(CurrentIndex==2)
                       ii=ds->nFactors-ii-1;
                    else
                      {
                        if(CurrentIndex==0)
                           indx=ds->FactorNoIndex;
                        else if(CurrentIndex==3)
                           indx=ds->FactorMIIndex;
                        else indx=ds->FactorMIIndexDsc;
                        for(ii=0; ii<ds->nFactors; ii++)
                         if(indx[ii]==i)//TODO: use IoI (Index of Index_
                            break;
                        if(ii>=ds->nFactors)
                          {
                            MessageBox(hMainWnd, "Indexing Error", "Error!",MB_OK);
                            exit(1);
                          }
                      }
                  }
                SendMessage(hMainWnd, WM_VSCROLL, SB_THUMBPOSITION|(ii<<16), 0);
                CurrentSelection=i-vPos;
                if(CurrentSelection<0)
                   CurrentSelection=0;
              }
            break;
       case ID_FILE_OPEN_DATASET:
                   OpenDataset();
                   break;
       case ID_FILE_CLOSE_DATASET:
                   CloseDataset();
                   break;
       case ID_FILE_NEW_DATASET:
                   break;
       case ID_PREDICTION_TESTING:
                   PredictionTesting();
                   break;
       case ID_DISTORTION_DISCOVERING_SINGLE:
                   DistortionDiscovering_Single();
                   return 0;
       case ID_DISTORTION_DISCOVERING_PAIR:
                   //DistortionDiscovering_Pair();
                   return 0;
       case ID_SIMULATION:
                   Simulation();
                   break;
       case ID_PERMUTATION_TESTING:
            PermutationTesting();
            break;
       case ID_SIGNIFICANCETESTING_PERMUTATIONS_PAIR:
            //Permutations_Pair();
            break;
       case IDM_ABOUT:
       	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutProc);
       	break;
       case IDM_EXIT:
       	DestroyWindow(hWnd);
       	break;
       default:
       	return DefWindowProc(hWnd, message, wParam, lParam);
       }
       break;
  case WM_PAINT:
  	hdc = BeginPaint(hWnd, &ps);
  	PaintMainWnd(hdc);
  	EndPaint(hWnd, &ps);
  	break;
  case WM_DESTROY:
  	PostQuitMessage(0);
  	break;
  default:
  	return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}
INT_PTR CALLBACK AboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, 0);
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK SelectDatasetProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);

	static int i=0, wmId, wmEvent;

	switch (message)
	{
	case WM_INITDIALOG:
                SetDlgItemText(hDlg, IDC_DATA_FOLDER, DataFolder);
                GetDatasetNames(GetDlgItem(hDlg, IDC_DATASET_NAMES));
                if(nDatasets>0)
                   SendDlgItemMessage(hDlg, IDC_DATASET_NAMES, LB_SETCURSEL,0,0);
                return (INT_PTR)TRUE;

        case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
                case IDC_DATASET_NAMES:
                     if(wmEvent==LBN_SELCHANGE)
                       {
                         i=SendDlgItemMessage(hDlg, IDC_DATASET_NAMES, LB_GETCURSEL, 0, 0);
                         if(i>=0)
                            SetDlgItemText(hDlg, IDC_HEADER, DatasetList[i]->Header);
                         else SetDlgItemText(hDlg, IDC_HEADER, "");
                         break;
                       }
                     if(wmEvent!=LBN_DBLCLK)
                        break;
                case IDOK:
                   i=SendDlgItemMessage(hDlg, IDC_DATASET_NAMES, LB_GETCURSEL, 0, 0);
                   if(i>=0)
                     {
                       SendDlgItemMessage(hDlg, IDC_DATASET_NAMES, LB_GETTEXT, (WPARAM)i, (LPARAM)DatasetName);
                       ModelExists=DatasetList[i]->ModelExists;
                     }
                   EndDialog(hDlg, 0);
		      break;
		case IDCANCEL:
			DatasetName[0] = 0;
                        EndDialog(hDlg, 0);
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK SimulationProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);

	static int i=0, wmId, wmEvent;

	switch (message)
	{
	case WM_INITDIALOG:
                CurrentDlg=hDlg;
                LoadModel();
                SetDlgItemText(hDlg, IDC_MODEL, BigBuffer);
                return (INT_PTR)TRUE;

        case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
                case ID_SIMULATE:
                     if(ModelChanged())
                        SaveModel();
                     Simulate();
                     //InvalidateRect(hMainWnd,0,1);
                     break;
                case ID_SAVE_MODEL:
                     SaveModel();
                     break;
                case IDOK:
                     if(   ModelChanged()
                        && IDYES==MessageBox(hDlg, "Save changes?", "Unsaved changes!", MB_YESNO)
                       )
                        SaveModel();
		case IDCANCEL:
                     EndDialog(hDlg, 0);
		     break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK FindProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		FindBuf[0]=0;
                //SetFocus(GetDlgItem(hDlg,IDC_FIND));
                return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
		  if(LOWORD(wParam) == IDOK)
                     GetDlgItemText(hDlg, IDC_FIND, FindBuf, sizeof(FindBuf));
                  EndDialog(hDlg, 0);
		  return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK ScaleProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);
  char buf[128];
	switch (message)
	{
	case WM_INITDIALOG:
                sprintf_s(buf, "%-5.3lf", Scale);
                SetDlgItemText(hDlg, IDC_SCALE, buf);
                return (INT_PTR)TRUE;

	case WM_COMMAND:
		if(LOWORD(wParam)==IDOK)
		  {
                    GetDlgItemText(hDlg, IDC_SCALE, buf, sizeof(buf));
                    Scale=atof(buf);
                    if(Scale<0.5)
                       Scale=0.5;
                    if(Scale>5.0)
                       Scale=5.0;
                    EndDialog(hDlg, 1);
                    return (INT_PTR)TRUE;
                  }
                else if(LOWORD(wParam)==IDCANCEL)
                  {
                    EndDialog(hDlg, 0);
                    return (INT_PTR)TRUE;
                  }
		break;
	}
    return (INT_PTR)FALSE;
}
LRESULT CALLBACK ReadOnlyEditControl(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//UNREFERENCED_PARAMETER(lParam);
  switch(message)
        {
          case WM_SETFOCUS:
               SetFocus((HWND)wParam);
               return (INT_PTR)FALSE;
        }
  return CallWindowProc(EditProc, hWnd, message, wParam, lParam);
}

//// FileOpen
//char* FileOpen(HWND hOwner = hMainWnd)
//{
//  static char Tmp[MAX_PATH+1];
//  BOOL Result = false;
//  OPENFILENAME ofn;
//
//  ZeroMemory(&ofn, sizeof(ofn));
//  ofn.lStructSize = sizeof(ofn);
//  ofn.hwndOwner = hOwner;
//  ofn.lpstrFile = Tmp;
//  ofn.lpstrFile[0] = '\0';
//  ofn.nMaxFile = CountOf(Tmp);
//  ofn.lpstrFilter = "Text\0*.txt\0All\0*.*\0";
//  ofn.nFilterIndex = 0;
//  ofn.lpstrFileTitle = NULL;
//  ofn.nMaxFileTitle = 0;
//  ofn.lpstrInitialDir = NULL;
//  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
//  Result = GetOpenFileName(&ofn);
//  return Result? Tmp: NULL;
//}

// Neither "." nor ".."
bool Dots(char *s)
{
  return s[0]=='.' && (s[1]==0 || s[1]=='.' && s[2]==0);
}

// Get Dataset Name
void GetDatasetNames(HWND hList)
{
  WIN32_FIND_DATA data;
  HANDLE hFind=NULL;
  FILE *f;
  char Buf[MAX_SOURCE+3], *p, *q;
  BOOL ModelExists=FALSE, Simulated=FALSE;
  int i;

  DatasetName[0] = 0;
  nDatasets = 0;
  SendMessage(hList, LB_RESETCONTENT, 0, 0);
  strcpy_s(DatasetPath, DataFolder);
  strcat_s(DatasetPath, "*.*");
  hFind = FindFirstFile(DatasetPath, &data);
  if(hFind != INVALID_HANDLE_VALUE)
    {
      do{
          if(   !Dots(data.cFileName)
             && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            )
            {
              DatasetPath[DataFolderLen]=0;
              strcat(DatasetPath, data.cFileName);
              strcat(DatasetPath, "\\");
              DatasetPathLen=strlen(DatasetPath);
              ModelExists=CheckFileExists(ModelFileName);
              strcat(DatasetPath, data.cFileName);
              strcat(DatasetPath, ".txt");
              Simulated = FALSE;
              DatasetList[nDatasets]->Header[0]=0;
              if(!fopen_s(&f, DatasetPath, "rt"))
                {
                  p=DatasetList[nDatasets]->Header;
                  *p=0;
                  q=p+MAXHEADER;
                  for(i=0; i<6; i++)
                     {
                       fgets(p, q-p, f);
                       while(*p)
                              p++;
                       if(p<q-1&&q-p<MAXHEADER)
                         {
                           *--p = '\r';
                           *++p = '\n';
                           *++p = 0;
                         }
                     }
                  Simulated = Dataset::CheckSimulated(DatasetList[nDatasets]->Header);
                  fclose(f);
                }
              if(Simulated && !ModelExists)
                 continue;
              if(ModelExists)
                 Simulated = TRUE;
              DatasetList[nDatasets]->ModelExists=ModelExists;
              if(ModelExists||f!=0/*dataset file exists*/)
                 strcpy_s(DatasetList[nDatasets++]->Name,
                           data.cFileName);
            }
        } while (FindNextFile(hFind, &data) != 0);
      FindClose(hFind);
      Sort<DatasetRec*>(DatasetList, nDatasets, CompDatasetPRec);
      for(i=0; i<nDatasets; i++)
          SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)DatasetList[i]->Name);
   }
}

void Status(char *status)
{
  SetDlgItemText(CurrentDlg, IDC_STATUS, status);
}

// Set Main Window Title
void SetMainWindowTitle()
{
  char Buf[361];

  if(ds==NULL)
     SetWindowText(hMainWnd, szTitle);
  else {
         strcpy_s(Buf, DatasetName);
         strcat_s(Buf, " - ");
         strcat_s(Buf, szTitle);
         SetWindowText(hMainWnd, Buf);
       }
}

void SetIndex()
{
  Index= CurrentIndex==0?ds->FactorNoIndex:
        (CurrentIndex==1?ds->FactorNameIndex:
        (CurrentIndex==2?ds->FactorNameIndexDsc:
        (CurrentIndex==3?ds->FactorMIIndex
                        :ds->FactorMIIndexDsc)));
}
// ReOpen Dataset
void ReOpenDataset()
{OpenDataset(false);}
// Open Dataset
void OpenDataset(BOOL Navigate)
{
  char Buf[MAX_PATH+1], SimBuf[MAX_PATH+1];
  SCROLLINFO si;

  crsr=SetCursor(hWaitCursor);
  if(ds!=NULL)
     delete ds;
  SetCursor(hWaitCursor);
  tested=discovered=permtstd=FALSE;
  ds = new Dataset;
  if(Navigate)
     DialogBox(hInst, MAKEINTRESOURCE(IDD_SELECT_DATASET), hMainWnd, SelectDatasetProc);
  if(DatasetName[0] != 0)
    {
      strcpy_s(DatasetPath, DataFolder);
      strcat_s(DatasetPath, DatasetName);
      strcat_s(DatasetPath, "\\");
      SetCurrentDirectory(DatasetPath);
      DatasetPathLen=strlen(DatasetPath);
      strcpy_s(Buf, DatasetPath);
      strcat_s(Buf, DatasetName);
      strcat_s(Buf, ".txt");
      if(PathFileExists(Buf))
         Loaded=ds->Load(Buf);
      if(!Loaded && ModelExists)
         ds->Simulated = true;
      if(Loaded && ds!=NULL)
        {
          CurrentSelection=0;
          SetIndex();
          si.cbSize=sizeof(SCROLLINFO);
          si.fMask=SIF_RANGE;
          si.nMin=0;
          si.nMax=ds->nSamples-1;
          SetScrollInfo(hMainWnd, SB_HORZ, &si, 1);
          si.nMax=ds->nFactors-1;
          SetScrollInfo(hMainWnd, SB_VERT, &si, 1);
          RECT rct;
          GetClientRect(hMainWnd, &rct);
          SendMessage(hMainWnd, WM_SIZE, SIZE_RESTORED, rct.right|(rct.bottom<<16));
          SendMessage(hMainWnd, WM_HSCROLL, SB_LEFT, 0);
          SendMessage(hMainWnd, WM_VSCROLL, SB_TOP, 0);
          InvalidateRect(hMainWnd, 0, 1);
        }
      ActiveDataset(1);
    }
  else CloseDataset();
  SetCursor(crsr);
}
BOOL CheckFileExists(char *fname)
{// checks if file fnames exists in DatasetPath
  strcat_s(DatasetPath, fname);
  BOOL exst=PathFileExists(DatasetPath);
  DatasetPath[DatasetPathLen]=0;
  return exst;
}
// Toggle Dataset State
void ActiveDataset(int active)
{
  MENUITEMINFO mii;
  UINT State[2]={MFS_DISABLED, MFS_ENABLED};
  HMENU hMenu = GetMenu(hMainWnd);

  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  //GetMenuItemInfo(GetMenu(hMainWnd), ID_FILE_CLOSE_DATASET, 0, &mii);
  mii.fState = State[active];
  SetMenuItemInfo(hMenu, ID_FILE_CLOSE_DATASET, 0, &mii);
  SetMenuItemInfo(hMenu, ID_PREDICTION_TESTING, 0, &mii);
  SetMenuItemInfo(hMenu, ID_PERMUTATION_TESTING, 0, &mii);
  SetMenuItemInfo(hMenu, ID_DISTORTION_DISCOVERING_SINGLE, 0, &mii);
  //SetMenuItemInfo(hMenu, ID_DISTORTION_DISCOVERING_PAIR, 0, &mii);
  SetMenuItemInfo(hMenu, ID_SEARCH_FIND, 0, &mii);
  SetMenuItemInfo(hMenu, ID_SIGNIFICANCETESTING_PERMUTATIONS_SINGLE, 0, &mii);
  //SetMenuItemInfo(hMenu, ID_SIGNIFICANCETESTING_PERMUTATIONS_PAIR, 0, &mii);

  // Simulation Specific
  mii.fState = State[active&&ds->Simulated];
  SetMenuItemInfo(hMenu, ID_SIMULATION, 0, &mii);
  SetMainWindowTitle();
}

// Close Dataset
void CloseDataset()
{
  SCROLLINFO si;
  crsr=SetCursor(hWaitCursor);
  if(ds!=NULL)
     delete ds;
  ds = NULL;
  Loaded=tested=discovered=FALSE;
  DatasetName[0] = 0;
  ActiveDataset(0);
  si.cbSize=sizeof(SCROLLINFO);
  si.fMask=SIF_RANGE;
  si.nMin=si.nMax=0;
  SetScrollInfo(hMainWnd, SB_HORZ, &si, 1);
  SetScrollInfo(hMainWnd, SB_VERT, &si, 1);
  InvalidateRect(hMainWnd, 0, 1);
  SetCursor(crsr);
}

// Locate Data Folder
void GetDataFolder()
{
  int i, j;
  char buf[MAX_PATH];

  GetModuleFileName(NULL, buf, sizeof(buf));
  for(i=j=0; buf[i]!=0; i++)
     {
       DataFolder[i] = buf[i];
       if(buf[i]=='\\')
          j = i+1;
     }
  strcpy_s(DataFolder+j, sizeof(DataFolder)-j, "Data\\");
  CreateDirectory(DataFolder, NULL);
  DataFolderLen=strlen(DataFolder);
}

void InitProgress()
{
  HWND hpb=GetDlgItem(CurrentDlg, IDC_PROGRESS_BAR);

  SendMessage(hpb, PBM_SETRANGE, 0, MAKELPARAM(0, ProgressSize));
  SendMessage(hpb, PBM_SETSTEP, (WPARAM) 1, 0);
  SendMessage(hpb, PBM_SETPOS, (WPARAM) 0, 0);
  SetDlgItemText(CurrentDlg, IDC_ELAPSED_TIME, "");
  SetDlgItemText(CurrentDlg, IDC_REMAINING_TIME, "");
  SetDlgItemText(CurrentDlg, IDC_PROGRESS_PERCENT, "0%");
  ProgressCounter=0;
  StartRunTime=clock();
}
void Progress()
{
  int eSec=0, rSec=0, Min=0, Hr=0, Day=0;
  char buf[64];
  REAL e, r;

  ProgressCounter++;
  SendMessage(GetDlgItem(CurrentDlg, IDC_PROGRESS_BAR),
              PBM_STEPIT, 0, 0);
  eSec=(clock()-StartRunTime);
  rSec=(REAL)eSec*(ProgressSize-ProgressCounter)/ProgressCounter;
  eSec/=CLOCKS_PER_SEC;
  rSec/=CLOCKS_PER_SEC;
  if(eSec>=60)
    {
      Min=eSec/60;
      eSec%=60;
      if(Min>=60)
        {
          Hr=Min/60;
          Min%=60;
          if(Hr>=24)
            {
              Day=Hr/24;
              Hr%=24;
            }
        }
    }
  sprintf_s(buf, "%2d - %2d:%2d:%2d",Day,Hr,Min,eSec);
  SetDlgItemText(CurrentDlg, IDC_ELAPSED_TIME, buf);
  Day=Hr=Min=0;
  if(rSec>=60)
    {
      Min=rSec/60;
      rSec%=60;
      if(Min>=60)
        {
          Hr=Min/60;
          Min%=60;
          if(Hr>=24)
            {
              Day=Hr/24;
              Hr%=24;
            }
        }
    }
  sprintf_s(buf, "%2d - %2d:%2d:%2d",Day,Hr,Min,rSec);
  SetDlgItemText(CurrentDlg, IDC_REMAINING_TIME, buf);
  sprintf_s(buf, "%d%%", Round(ProgressCounter*100.0/ProgressSize));
  SetDlgItemText(CurrentDlg, IDC_PROGRESS_PERCENT, buf);
}

void LoadModel()
{
  FILE *f;
  char c, *p, *q, buf[MAX_PATH+1];

  strcpy_s(buf, DatasetPath);
  strcat_s(buf, ModelFileName);
  BigBuffer[0]=0;
  p=BigBuffer;
  q=p+sizeof(BigBuffer);
  if(0==fopen_s(&f, buf, "rt"))
    {
      while(EOF!=(c=getc(f)))
           {
             if(p==q)
               {
                 MessageBox(0,"Too Long Model\nMaximum 8191"
                              " character allowed","Error",
                            MB_OK|MB_ICONERROR);
                 fclose(f);
                 BigBuffer[0]=0;
                 return;
               }
             if(c=='\n')
                *p++='\r';
             *p++=c;
           }
      fclose(f);
    }
}
void SaveModel()
{
  FILE *f;
  char *p, buf[MAX_PATH+1];

  strcpy_s(buf, DatasetPath);
  strcat_s(buf, ModelFileName);
  if(0==fopen_s(&f, buf, "wt"))
    {
      GetDlgItemText(CurrentDlg, IDC_MODEL, BigBuffer, sizeof(BigBuffer));
      for(p=BigBuffer; *p!=0; p++)
       if(*p!='\r')
          putc(*p, f);
      fclose(f);
    }
  else MessageBox(0,"Failed to create Model file.","Error",
                  MB_OK|MB_ICONERROR);
}
BOOL ModelChanged()
{
  static char Buf[8192];

  GetDlgItemText(CurrentDlg, IDC_MODEL, Buf, sizeof(Buf));
  return 0!=stricmp(Buf, BigBuffer);
}

void Simulate()
{
  char Buf[MAX_PATH+1];

  crsr=SetCursor(hWaitCursor);
  strcpy_s(Buf, DatasetPath);
  strcat_s(Buf, ModelFileName);
  strcat_s(DatasetPath, DatasetName);
  strcat_s(DatasetPath, ".txt");
  delete ds;
  ds = new Dataset;
  ds->Simulate(Buf, DatasetPath);
  DatasetPath[DatasetPathLen]=0;
  SetCursor(crsr);
  ReOpenDataset();
}
void Simulation()
{
  DialogBox(hInst, MAKEINTRESOURCE(IDD_SIMULATION), hMainWnd, SimulationProc);
}

void LowLight(HWND hWnd, RECT **Rect, int n)
{
  int i, maxW=0, maxH=0;
  HDC hdc=GetDC(hWnd), hdc1=CreateCompatibleDC(hdc);
  RECT MaxRect;

  for(i=0; i<n; i++)
     {
       if(maxW<Rect[i]->right-Rect[i]->left+2)
          maxW=Rect[i]->right-Rect[i]->left+2;
       if(maxH<Rect[i]->bottom-Rect[i]->top+2)
          maxH=Rect[i]->bottom-Rect[i]->top+2;
     }
  HBITMAP hbmp=CreateCompatibleBitmap(hdc, maxW, maxH);
  SelectObject(hdc1, hbmp);
  MaxRect.left=0;
  MaxRect.right=maxW;
  MaxRect.top=0;
  MaxRect.bottom=maxH;
  FillRect(hdc1, &MaxRect, hLtGreyBrush);//(HBRUSH)GetStockObject(GRAY_BRUSH));
  for(i=0; i<n; i++)
      BitBlt(hdc, Rect[i]->left,  Rect[i]->top,
                  Rect[i]->right- Rect[i]->left+1,
                  Rect[i]->bottom-Rect[i]->top+1,
                  hdc1, 0, 0, SRCAND);//, SRCERASE);
  DeleteObject(hbmp);
  ReleaseDC(hWnd, hdc);
  DeleteDC(hdc1);
}

//HDC BeginPrint(HWND hWnd)
//{
//  static PRINTDLG pd ;
//
//  pd.lStructSize         = sizeof (PRINTDLG) ;
//  pd.hwndOwner           = hWnd ;
//  pd.hDevMode            = NULL ;
//  pd.hDevNames           = NULL ;
//  pd.hDC                 = NULL ;
//  pd.Flags               = PD_ALLPAGES | PD_COLLATE | PD_RETURNDC ;
//  pd.nFromPage           = 0 ;
//  pd.nToPage             = 0 ;
//  pd.nMinPage            = 0 ;
//  pd.nMaxPage            = 0 ;
//  pd.nCopies             = 1 ;
//  pd.hInstance           = NULL ;
//  pd.lCustData           = 0L ;
//  pd.lpfnPrintHook       = NULL ;
//  pd.lpfnSetupHook       = NULL ;
//  pd.lpPrintTemplateName = NULL ;
//  pd.lpSetupTemplateName = NULL ;
//  pd.hPrintTemplate      = NULL ;
//  pd.hSetupTemplate      = NULL ;
//  if(!PrintDlg(&pd)) return 0;
//  Escape(pd.hDC,STARTDOC,0,0,0);
//  return pd.hDC;
//}
//void EndPrint(HDC hdc)
//{
//  Escape(hdc,NEWFRAME,0,0,0);
//  Escape(hdc,ENDDOC,0,0,0);
//  DeleteDC(hdc);
//}

void SaveRectToFile(RECT &Rect, char *Fname,
                    void (Draw)(HDC,REAL), REAL scale)
{
  int w=(Rect.right-Rect.left+1)*scale,
      h=(Rect.bottom-Rect.top+1)*scale;
  HDC hdc=GetDC(CurrentDlg), hdc1=CreateCompatibleDC(hdc);
  HBITMAP hbmp=CreateCompatibleBitmap(hdc, w, h);

  SelectObject(hdc1, hbmp);
  Draw(hdc1, scale);
  //BitBlt(hdc1, 0, 0, w, h, hdc, rctROC.left, rctROC.top, SRCCOPY);//, SRCERASE);
  strcat_s(DatasetPath, Fname);
  SaveBitmap(hbmp, DatasetPath);
  DatasetPath[DatasetPathLen]=0;
  DeleteObject(hbmp);
  ReleaseDC(CurrentDlg, hdc);
  DeleteDC(hdc1);
}

void GetDumps(RECT **Rects, int nRects)
{
  HDC hdc=GetDC(CurrentDlg);
  RECT **q;

  for(q=Rects+nRects; Rects<q; Rects++)
      BitBlt(hdc_Dump,
             (*Rects)->left, (*Rects)->top,
             (*Rects)->right-(*Rects)->left+1,
             (*Rects)->bottom-(*Rects)->top+1,
             hdc,
             (*Rects)->left, (*Rects)->top,
             SRCCOPY);
  ReleaseDC(CurrentDlg, hdc);
}
void PutDumps(HDC hdc, RECT **Rects, int nRects)
{
  RECT **q;

  for(q=Rects+nRects; Rects<q; Rects++)
      BitBlt(hdc,
             (*Rects)->left, (*Rects)->top,
             (*Rects)->right-(*Rects)->left+1,
             (*Rects)->bottom-(*Rects)->top+1,
             hdc_Dump,
             (*Rects)->left, (*Rects)->top,
             SRCCOPY);
}

// Paint Main Window
void PaintMainWnd(HDC hdc)
{
  if(!Loaded||ds==NULL)
     return;
  //SCROLLINFO si;
  RECT rct;
  char buf[MAX_TITLE+32], *p;
  SIZE Size;
  int indx=vPos+CurrentSelection;

  SetGraphicsMode(hdc, GM_ADVANCED);
  SetMapMode(hdc, MM_TEXT);

  GetClientRect(hMainWnd, &rct);
  int cL=0, cR=rct.right, cT=rct.top, cB=rct.bottom;
  int s1=hPos, s2=s1+hPage;
  if(s2>ds->nSamples)
     s2=ds->nSamples;
  int f1=vPos, f2=f1+vPage;
  if(f2>ds->nFactors)
     f2=ds->nFactors;
  int i, j, k, x1, x2, y1, y2,
      pw=ColZeroWidth+(s2-s1)*CellWidth,
      ph=(f2-f1)*CellHeight;
  x1=0;
  x2=pw;
  SelectObject(hdc, hGreyBrush);
  rct.top=HeaderHeight;
  rct.bottom=HeaderHeight+(CellHeight<<1);
  rct.left=ColZeroWidth;
  rct.right=pw;
  FillRect(hdc, &rct, hGreyBrush);
  rct.top=rct.bottom;
  rct.bottom+=ph;
  rct.left=0;
  rct.right=ColZeroWidth;
  FillRect(hdc, &rct, hGreyBrush);

  rct.top=HeaderHeight+(2+CurrentSelection)*CellHeight;
  rct.bottom=rct.top+CellHeight;
  rct.left=0;
  rct.right=pw;
  FillRect(hdc, &rct, hBlueBrush);

  SelectObject(hdc, hGridPen);//hLtLtGreyPenDot);
  for(i=f1, j=0; i<f2+2; i++,j++)
     {
       y1=HeaderHeight+j*CellHeight;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x2, y1);
     }
  y1=HeaderHeight;
  y2=HeaderHeight+(vPage+2)*CellHeight;
  for(i=s1, j=0; i<s2; i++,j++)
     {
       x1=ColZeroWidth+j*CellWidth;
       MoveToEx(hdc, x1, y1, 0);
       LineTo(hdc, x1, y2);
     }
  SetBkMode(hdc, TRANSPARENT);
  SetTextAlign(hdc, TA_CENTER|TA_TOP);
  y1=HeaderHeight;
  rct.top=HeaderHeight+1;
  rct.bottom=HeaderHeight+CellHeight-1;
  SetTextColor(hdc, RGB(255,255,255));
  HFONT hOldFont=(HFONT)SelectObject(hdc, hDataHiFont);
  for(i=s1, j=0; i<s2; i++,j++)
     {
       x1=ColZeroWidth+j*CellWidth;
       rct.left=x1+1;
       rct.right=x1+CellWidth-1;
       ExtTextOut(hdc, x1+(CellWidth>>1), y1, ETO_CLIPPED, &rct,
                  ds->SampleNames[i],
                  Min(strlen(ds->SampleNames[i]),11),0);
     }
  rct.top+=CellHeight;
  rct.bottom+=CellHeight;
  y1+=CellHeight;
  for(i=s1, j=0; i<s2; i++,j++)
     {
       x1=ColZeroWidth+j*CellWidth;
       rct.left=x1+1;
       rct.right=x1+CellWidth-1;
       p=ds->ClassNames[ds->SampleClasses[i]];
       ExtTextOut(hdc, x1+(CellWidth>>1), y1, ETO_CLIPPED, &rct,
                  p, Min(strlen(p), 11), 0);
     }
  for(i=f1, j=0; i<f2; i++,j++)
     {
       y1+=CellHeight;
       rct.top+=CellHeight;
       rct.bottom+=CellHeight;
       rct.left=1;
       rct.right=ColZeroWidth-1;
       p=ds->FactorNames[Index[i]];
       SetTextColor(hdc, RGB(255,255,255));
       SetTextAlign(hdc, TA_LEFT|TA_TOP);
       SelectObject(hdc, hDataHiFont);
       ExtTextOut(hdc, 2, y1, ETO_CLIPPED, &rct,
                  p, Min(strlen(p), 13), 0);
       SetTextAlign(hdc, TA_RIGHT|TA_TOP);
       sprintf_s(buf, "%5.3f", ds->HC-ds->CndH_1F[Index[i]]);
       TextOut(hdc, ColZeroWidth-2, y1, buf, strlen(buf));
       SetTextColor(hdc, i==indx?0xFFFFFF:0);
       SelectObject(hdc, hDataFont);
       for(k=s1, j=0; k<s2; k++,j++)
          {
            x1=ColZeroWidth+j*CellWidth;
            rct.left=x1+1;
            rct.right=x1+CellWidth-1;
            sprintf_s(buf,"%9.3f",ds->FactorData[Index[i]][k]);
            ExtTextOut(hdc, x1+CellWidth-2, y1, ETO_CLIPPED, &rct,
                       buf, Min(strlen(buf), 11), 0);
          }
     }
  SetTextColor(hdc, 0xFFFFFF);
  SetTextAlign(hdc, TA_LEFT|TA_TOP);
  SelectObject(hdc, hDataHiFont);
  rct.top=HeaderHeight+CellHeight;
  rct.bottom=rct.top+CellHeight;
  rct.left=0;
  rct.right=ColZeroWidth;
  FillRect(hdc, &rct, hDkDkGreyBrush);
  TextOut(hdc, 2, HeaderHeight+CellHeight,
             "Factors          MI", 19);
  SelectObject(hdc, hOldFont);
  SetTextColor(hdc, 0);
  y1=HeaderHeight-CellHeight+3;
  sprintf_s(buf,"%s (%s/%s)", ds->Title,
            ds->ClassNames[0],ds->ClassNames[1]);
  SetTextAlign(hdc, TA_LEFT|TA_TOP|TA_NOUPDATECP);
  i=strlen(buf);
  TextOut(hdc, 2, y1, buf, i);
  GetTextExtentPoint32A(hdc, buf, i, &Size);
  i=rct.right=cR;
  rct.top=0;
  rct.bottom=HeaderHeight-CellHeight;
  rct.left=2;
  rct.right=Size.cx+2;
  if(rct.right+403<i)
     rct.right=i-403;
  FillRect(hdc, &rct, hLtLtLtGreyBrush);
  DrawText(hdc, ds->Source, -1, &rct, DT_EDITCONTROL|DT_WORDBREAK);
  rct.left=rct.right+3;
  rct.right=i-1;
  if(rct.right-rct.left > 400)
     rct.left=rct.right - 400;
  if(rct.right-rct.left > 120)
    {
      rct.bottom=HeaderHeight-1;
      SelectObject(hdc, hBlackPen);
      MoveToEx(hdc, rct.left, rct.top, 0);
      LineTo(hdc, rct.right, rct.top);
      LineTo(hdc, rct.right, rct.bottom);
      LineTo(hdc, rct.left, rct.bottom);
      LineTo(hdc, rct.left, rct.top);
      rct.top+=4;
      rct.bottom-=5;
      rct.left+=4;
      rct.right-=5;
      FillRect(hdc, &rct, hPlotBkBrush);// hLtLtGreyBrush);
      int w=rct.right-rct.left,
          h=rct.bottom-rct.top;
      SelectObject(hdc, hDkDkGreyPen);
      MoveToEx(hdc, rct.left, rct.bottom, 0);
      LineTo(hdc, rct.right,  rct.bottom);
      //REAL fmin=ds->FactorMin[indx], fmax=ds->FactorMax[indx];
      //x2=rct.left;
      //if(fmin<=0 && fmax>=0)
      //  {
      //    x1=rct.left-w*fmin/(fmax-fmin);
      //    MoveToEx(hdc, x1, rct.top, 0);
      //    LineTo(hdc, x1, rct.bottom);
      //    x2=x1;
      //  }
      if(ds->CndH_1F!=NULL)
        {
          rct.top-=4;
          REAL W=rct.right-rct.left,
               H=(rct.bottom-rct.top)*0.95,
               Step=W/ds->FactorFreqSize,
               HalfStep=Step/2;
          int  xLinit=rct.left+W*(ds->FactorClusters[Index[indx]].MidPoint-ds->FactorMin[Index[indx]])/(ds->FactorMax[Index[indx]]-ds->FactorMin[Index[indx]]);
          RECT bar;
          int maxfreq=0, midY=(rct.top+rct.bottom)*.7;
          HBRUSH BarFill[]={hMagentaBrush,
                            hBlueBrush,
                            hRedBrush
                           };
          HBRUSH hcls[2]={hLtLtLtBlueBrush,hLtLtLtRedBrush};
          COLOR16 Cs[][3]={{0xffff,0x5000,0x5000},
                           {0x5000,0x5000,0xff00}};
          BOOL D=ds->FactorClusters[Index[indx]].Direction;
          TRIVERTEX Verticese[]=
             {{rct.left,rct.top+1,Cs[!D][0],Cs[!D][1],Cs[!D][2],0xff00},
              {xLinit,midY,0xffff,0xffff,0xffff,0xff00},
              {xLinit+1,rct.top+1,Cs[D][0],Cs[D][1],Cs[D][2],0xff00},
              {rct.right,midY,0xffff,0xffff,0xffff,0xff00}
             };
          GRADIENT_RECT Mesh[]={{0,1},{2,3}};
          GdiGradientFill(hdc,Verticese,CountOf(Verticese),
                          Mesh,CountOf(Mesh),
                          GRADIENT_FILL_RECT_V);
          for(i=0; i<ds->FactorFreqSize; i++)
           if(maxfreq<ds->FactorFreq[Index[indx]][i])
              maxfreq=ds->FactorFreq[Index[indx]][i];
          bar.bottom=rct.bottom;
          SelectObject(hdc, hWhitePen);
          for(j=0; j<=ds->nClasses; j++)
             {
               SelectObject(hdc, BarFill[j]);
               for(i=0; i<ds->FactorFreqSize; i++)
                  {
                    bar.left=rct.left+i*Step;
                    bar.right=bar.left+((j==1)?HalfStep-1:Step);
                    if(j==2)
                       bar.left+=HalfStep;
                    bar.top=bar.bottom-ds->FactorFreq[Index[indx]][i+j*ds->FactorFreqSize]*H/maxfreq;
                    Rectangle(hdc, bar.left, bar.top,bar.right, bar.bottom);
                  }
             }
        }
      SelectObject(hdc, hLtLtGreyBrush);//(HBRUSH) (COLOR_BTNFACE+1));
      SelectObject(hdc, hDkGreyPen);
      Rectangle(hdc, cL, cB-StatusBarHeight, cR, cB);
      MoveToEx(hdc, cL, cB-1, 0);
      LineTo(hdc, cR-1, cB-1);
      LineTo(hdc, cR-1, cB-StatusBarHeight-1);
      sprintf_s(buf, "%5d Factors  %5d Samples         "
                     "    Line #: %d",
                     ds->nFactors, ds->nSamples,
                     vPos+CurrentSelection+1);
      SetTextColor(hdc, cDkDkGreyColor);
      SetTextAlign(hdc, TA_LEFT|TA_TOP|TA_NOUPDATECP);
      SelectObject(hdc, hDataHiFont);
      TextOut(hdc, cL+5, cB-StatusBarHeight+1, buf, strlen(buf));
    }
  SelectObject(hdc, hGreyPen2);
  for(i=0; i<CountOf(xSortF); i++)
     {
       MoveToEx(hdc, xSortF[i], ySortF[i], 0);
       for(j=0; j<2; j++)
           LineTo(hdc, xSortF[i]+dx[i&1][j], ySortF[i]+dy[i&1][j]);
     }
  if(CurrentIndex>0)
    {
      SelectObject(hdc, hWhitePen2);
      i=CurrentIndex-1;
       MoveToEx(hdc, xSortF[i], ySortF[i], 0);
       for(j=0; j<2; j++)
           LineTo(hdc, xSortF[i]+dx[i&1][j], ySortF[i]+dy[i&1][j]);
    }
}

void NormalizeRect(RECT &rct)
{
  rct.right-=rct.left;
  rct.left=0;
  rct.bottom-=rct.top;
  rct.top=0;
}

void ScaleRect(RECT &rct, REAL scale)
{
  rct.left*=scale;
  rct.top*=scale;
  rct.right*=scale;
  rct.bottom*=scale;
}

/*
class BezierSpline
{
  /// <summary>
  /// Get open-ended Bezier Spline Control Points.
  /// </summary>
  /// <param name="knots">Input Knot Bezier spline points.</param>
  /// <param name="firstControlPoints">Output First Control points
  /// array of knots.Length - 1 length.</param>
  /// <param name="secondControlPoints">Output Second Control points
  /// array of knots.Length - 1 length.</param>
  /// <exception cref="ArgumentNullException"><paramref name="knots"/>
  /// parameter must be not null.</exception>
  /// <exception cref="ArgumentException"><paramref name="knots"/>
  /// array must contain at least two points.</exception>
  void GetCurveControlPoints(Point[] knots,
  	out Point[] firstControlPoints, 
        out Point[] secondControlPoints)
  {
     int n = knots.Length - 1;
     if (n == 1)
     { // Special case: Bezier curve should be a straight line.
     	firstControlPoints = new Point[1];
     	// 3P1 = 2P0 + P3
     	firstControlPoints[0].X = (2 * knots[0].X + knots[1].X) / 3;
     	firstControlPoints[0].Y = (2 * knots[0].Y + knots[1].Y) / 3;
     
     	secondControlPoints = new Point[1];
     	// P2 = 2P1 – P0
     	secondControlPoints[0].X = 2 *
     		firstControlPoints[0].X - knots[0].X;
     	secondControlPoints[0].Y = 2 *
     		firstControlPoints[0].Y - knots[0].Y;
     	return;
     }
     
     // Calculate first Bezier control points
     // Right hand side vector
     double[] rhs = new double[n];
     
     // Set right hand side X values
     for (int i = 1; i < n - 1; ++i)
     	rhs[i] = 4 * knots[i].X + 2 * knots[i + 1].X;
     rhs[0] = knots[0].X + 2 * knots[1].X;
     rhs[n - 1] = (8 * knots[n - 1].X + knots[n].X) / 2.0;
     // Get first control points X-values
     double[] x = GetFirstControlPoints(rhs);
     
     // Set right hand side Y values
     for (int i = 1; i < n - 1; ++i)
     	rhs[i] = 4 * knots[i].Y + 2 * knots[i + 1].Y;
     rhs[0] = knots[0].Y + 2 * knots[1].Y;
     rhs[n - 1] = (8 * knots[n - 1].Y + knots[n].Y) / 2.0;
     // Get first control points Y-values
     double[] y = GetFirstControlPoints(rhs);
     
     // Fill output arrays.
     firstControlPoints = new Point[n];
     secondControlPoints = new Point[n];
     for (int i = 0; i < n; ++i)
     {
       // First control point
       firstControlPoints[i] = new Point(x[i], y[i]);
       // Second control point
       if(i < n - 1)
        secondControlPoints[i]=new Point(2*knots[i+1].X-x[i+1],
                                         2*knots[i+1].Y-y[i+1]);
       else
       	secondControlPoints[i]=new Point((knots[n].X+x[n-1])/2,
                                         (knots[n].Y+y[n-1])/2);
     }
  }
  
  /// <summary>
  /// Solves a tridiagonal system for one of coordinates (x or y)
  /// of first Bezier control points.
  /// </summary>
  /// <param name="rhs">Right hand side vector.</param>
  /// <returns>Solution vector.</returns>
  double[] GetFirstControlPoints(double[] rhs)
  {
  	int n = rhs.Length;
  	double[] x = new double[n]; // Solution vector.
  	double[] tmp = new double[n]; // Temp workspace.
  
  	double b = 2.0;
  	x[0] = rhs[0] / b;
  	for (int i = 1; i < n; i++) // Decomposition and forward substitution.
  	{
  		tmp[i] = 1 / b;
  		b = (i < n - 1 ? 4.0 : 3.5) - tmp[i];
  		x[i] = (rhs[i] - x[i - 1]) / b;
  	}
  	for (int i = 1; i < n; i++)
  		x[n - i - 1] -= tmp[n - i] * x[n - i]; // Backsubstitution.
  
  	return x;
  }
}*/