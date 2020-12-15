// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <Winspool.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include<commdlg.h>
#include<shlwapi.h>

// TODO: reference additional headers your program requires here
#include "Dataset.h"
#include "Bezier.h"
#include "Distortion.h"
#include "Bitmap.h"
#include "Tools.h"
#include<Commctrl.h>
#include<windowsx.h>

#define MAX_LOADSTRING 100
#define MAXLIST 100
#define MAXHEADER 4096

// Defined Types:
typedef unsigned int(WINAPI *ThreadAPI)(void*);
struct DatasetRec
{char Name[MAX_PATH+1], Header[4096]; BOOL ModelExists;};

// Global Variables:
extern HINSTANCE hInst;
extern HWND hMainWnd, CurrentDlg;
extern TCHAR szTitle[MAX_LOADSTRING];		// The title bar text
extern TCHAR szWindowClass[MAX_LOADSTRING];	// the main window class name
extern Dataset *ds;
extern char DataFolder[MAX_PATH+1],
            DatasetName[MAX_PATH+1],
            DatasetPath[MAX_PATH+1];
extern DatasetRec **DatasetList;
extern int DataFolderLen, DatasetPathLen, nDatasets;
extern int **Predictors_X, **Predictors_Y,
           nPredictorsDist, nPredictorsNoDist,
           SizePredictors;
extern BOOL changed, Loaded, tested, discovered, permtstd;
extern HRGN hRgnTop, hRgnROC, hRgnROC_Only;
extern HRGN hRgnNR, hRgnIC, hRgnDI, hRgnNR_Only;
extern HRGN hRgnSig;
extern Dataset::_FocusOn FocusOn;
extern int nKnots, nPoints;
extern HCURSOR crsr, hWaitCursor, hArrowCursor, hCurrentCursor;
extern COLORREF LegendBkColor, LowLightColor, HighLightColor,
                cDlgBkColor, cListBkColor, cBlackColor,
                cDkDkGreyColor, cDkGreyColor, cGreyColor,
                cLtGreyColor, cLtLtGreyColor, cLtLtLtGreyColor,
                cLtRedColor, cRedColor, cDkRedColor,
                cLtGreenColor, cGreenColor, cDkGreenColor,
                cLtBlueColor, cBlueColor, cDkBlueColor,
                cWhiteColor, cMagentaColor;

extern HPEN hGreyPlotPen0;
extern HPEN hBluePlotPen0;
extern HPEN hRedPlotPen0;
extern HPEN hGreenPlotPen0;

extern HPEN hGreyPlotPen1;
extern HPEN hBluePlotPen1;
extern HPEN hRedPlotPen1;
extern HPEN hGreenPlotPen1;

extern HPEN hGreyPlotPen2;
extern HPEN hBluePlotPen2;
extern HPEN hRedPlotPen2;
extern HPEN hGreenPlotPen2;

extern HPEN hGreyPlotPen3;
extern HPEN hBluePlotPen3;
extern HPEN hRedPlotPen3;
extern HPEN hGreenPlotPen3;

extern HPEN hGreyPlotPen4;
extern HPEN hBluePlotPen4;
extern HPEN hRedPlotPen4;
extern HPEN hGreenPlotPen4;

extern HPEN hGreyPlotPen5;
extern HPEN hBluePlotPen5;
extern HPEN hRedPlotPen5;
extern HPEN hGreenPlotPen5;

extern HPEN hGreyPlotPen6;
extern HPEN hBluePlotPen6;
extern HPEN hRedPlotPen6;
extern HPEN hGreenPlotPen6;

extern HPEN hGreyPlotPens[];
extern HPEN hBluePlotPens[];
extern HPEN hRedPlotPens[];
extern HPEN hGreenPlotPens[];

extern HPEN hGreyPen;
extern HPEN hDkGreyPen;
extern HPEN hDkDkGreyPen;
extern HPEN hLtGreyPen;
extern HPEN hLtLtGreyPen;
extern HPEN hLtLtLtGreyPen;
extern HPEN hDkBluePen;
extern HPEN hLtBluePen;
extern HPEN hDkRedPen;
extern HPEN hLtRedPen;
extern HPEN hLtLtRedPen;
extern HPEN hLtLtLtRedPen;
extern HPEN hBlackPen;
extern HPEN hBlackPen2;
extern HPEN hWhitePen;
extern HPEN hWhitePen2;
extern HPEN hGreyPen2;
extern HPEN hLtLtGreyPenDot;

extern HBRUSH hWhiteBrush;
extern HBRUSH hBkBrush;
extern HBRUSH hGreyBrush;
extern HBRUSH hDkGreyBrush;
extern HBRUSH hDkDkGreyBrush;
extern HBRUSH hLtGreyBrush;
extern HBRUSH hLtLtGreyBrush;
extern HBRUSH hLtLtLtGreyBrush;
extern HBRUSH hDkBlueBrush;
extern HBRUSH hBlueBrush;
extern HBRUSH hLtBlueBrush;
extern HBRUSH hLtLtBlueBrush;
extern HBRUSH hLtLtLtBlueBrush;
extern HBRUSH hDkRedBrush;
extern HBRUSH hRedBrush;
extern HBRUSH hLtRedBrush;
extern HBRUSH hLtLtRedBrush;
extern HBRUSH hLtLtLtRedBrush;
extern HBRUSH hMagentaBrush;
extern HBRUSH hDlgBkBrush;
extern HBRUSH hListBkBrush;
extern HBRUSH hLegendBkBrush;
extern HBRUSH hPlotBkBrush;
extern HBRUSH hHeatMapBrushs[3][256];
extern HPEN hGridMinorPen;
extern HPEN hGridMajorPen;
extern HPEN hGridPen;
extern HPEN hLegendPen;
extern HFONT hTextFont;

extern HFONT hDataFont;
extern HFONT hSmallFont;
extern HFONT hDataHiFont;
extern HFONT hHighFont;
extern HFONT hHeavyFont;
extern HFONT hVFont;
extern HFONT hTitleFont;
extern BOOL Aborted;
extern int ProgressSize, ProgressCounter;
extern time_t StartRunTime, ElapsedTime, RemainingTime;
extern char BigBuffer[8192], FindBuf[128];
extern HDC hdc_Dump;
extern HBITMAP hbmp_Dump;
extern int CellWidth, CellHeight, ColZeroWidth, HeaderHeight,
           StatusBarHeight, CurrentSelection,
           hPos, hPage, hMin, hMax, vPos, vPage, vMin, vMax;
extern REAL Scale;
extern int xSortF[], ySortF[], dx[2][2], dy[2][2];
extern int *Index, CurrentIndex;
extern WNDPROC EditProc;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SimulationProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK FindProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ScaleProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ReadOnlyEditControl(HWND, UINT, WPARAM, LPARAM);
void GetDataFolder();
void OpenDataset(BOOL Navigate=TRUE);
void CloseDataset();
void SetMainWindowTitle();
void GetDatasetNames(HWND);
void ActiveDataset(int);
void LowLight(HWND, RECT**, int);
void PaintMainWnd(HDC);
void PrintROC();
HDC BeginPrint(HWND);
void EndPrint(HDC);
void DrawROC(HDC, REAL);
void DrawSig(HDC, REAL);
void CalcSig();
void CalcROC();
BOOL CheckFileExists(char *);
void LoadModel();
void SaveModel();
BOOL ModelChanged();
void Simulate();
void Simulation();
void Progress();
void InitProgress();
void GetDumps(RECT**, int);
void PutDumps(HDC,RECT**,int);
void SetIndex();
void NormalizeRect(RECT&);
void ScaleRect(RECT&, REAL);
void SaveRectToFile(RECT&,char*,void(*)(HDC,REAL), REAL scale=Scale);
void InitHeatMaps();
void DeleteHeatMaps();
void Status(char*);
void PermutationTesting();

// Begin PredictionTesting forwards
INT_PTR CALLBACK PredictionTestingProc(HWND, UINT, WPARAM, LPARAM);
void PredictionTesting();
// End PredictionTesting forwards

// Begin DistortionDiscovering_Single forwards
INT_PTR CALLBACK DistortionDiscovering_SingleProc(HWND, UINT, WPARAM, LPARAM);
void DistortionDiscovering_Single();
// End DistortionDiscovering forwards

// Begin DistortionOverlap forwards
int DistortionOverlap(int argc, char* argv[]);
// End DistortionOverlap forwards
