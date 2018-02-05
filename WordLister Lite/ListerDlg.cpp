/* ******************************************************************** **
** @@ Lister
** @  Copyrt : (c) gazlan@yandex.ru
** @  Author : gazlan@yandex.ru
** @  Modify :
** @  Update : 2011-06-25
** @  Notes  :
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include <afxrich.h>     
                 
#include "..\shared\mmf.h"
#include "..\shared\file.h"
#include "..\shared\map_bppt_jannink.h"
#include "..\shared\hash_sha160.h"

#include "Lister.h"
#include "ListerDlg.h"

/* ******************************************************************** **
** @@                   internal defines
** ******************************************************************** */

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ASCII_SIZE               (256)
#define MAX_WORD_SIZE            (255)

/* ******************************************************************** **
** @@                   internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@                   external global variables
** ******************************************************************** */

extern DWORD   dwKeepError = 0;

/* ******************************************************************** **
** @@                   static global variables
** ******************************************************************** */

static char    _pszWord[MAX_PATH];
static char    _pXLAT  [MAX_PATH];

static DWORD   _dwPos   = 0;
static DWORD   _dwKeep  = 0;
static BYTE*   _pText   = NULL;
static DWORD   _dwSize  = 0;
static DWORD   _dwDelta = 0;

/* ******************************************************************** **
** @@                   real code
** ******************************************************************** */

/* ******************************************************************** **
** @@ CMP_Hash()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Hash compare function
** ******************************************************************** */

inline int CMP_Hash(const void* const pKey1,const void* const pKey2)
{
   DWORD**  p1 = (DWORD**)pKey1;
   DWORD**  p2 = (DWORD**)pKey2;

   if (p1[0] > p2[0])
   {
      return 1;
   }
   else if (p1[0] < p2[0])
   {
      return -1;
   }
   else
   {
      if (p1[1] > p2[1])
      {
         return 1;
      }
      else if (p1[1] < p2[1])
      {
         return -1;
      }
      else
      {
         if (p1[2] > p2[2])
         {
            return 1;
         }
         else if (p1[2] < p2[2])
         {
            return -1;
         }
         else
         {
            if (p1[3] > p2[3])
            {
               return 1;
            }
            else if (p1[3] < p2[3])
            {
               return -1;
            }
            else
            {
               if (p1[4] > p2[4])
               {
                  return 1;
               }
               else if (p1[4] < p2[4])
               {
                  return -1;
               }
               else
               {
                  return 0;
               }
            }
         }
      }
   }
}

/* ******************************************************************** **
** @@ CListerDlg::CListerDlg()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  : Constructor
** ******************************************************************** */

CListerDlg::CListerDlg(CWnd* pParent /*=NULL*/)
   : CDialog(CListerDlg::IDD, pParent)
{
   //{{AFX_DATA_INIT(CListerDlg)
   m_Min = 1;
   m_Max = 255;
   m_Src  = _T("");
	m_Map  = _T("");
   m_Out  = _T("");
	//}}AFX_DATA_INIT

   // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/* ******************************************************************** **
** @@ CListerDlg::DoDataExchange()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::DoDataExchange(CDataExchange* pDX)
{
   CDialog::DoDataExchange(pDX);
   //{{AFX_DATA_MAP(CListerDlg)
   DDX_Control(pDX, IDC_PROGRESS, m_Progress);
   DDX_Control(pDX, IDC_SPN_MAX, m_SpMax);
   DDX_Control(pDX, IDC_SPN_MIN, m_SpMin);
   DDX_Text(pDX, IDC_EDT_SIZE_MIN, m_Min);
   DDV_MinMaxUInt(pDX, m_Min, 1, 255);
   DDX_Text(pDX, IDC_EDT_SIZE_MAX, m_Max);
   DDV_MinMaxUInt(pDX, m_Max, 1, 255);
   DDX_Text(pDX, IDC_EDT_SRC, m_Src);
   DDV_MaxChars(pDX, m_Src, 260);
   DDX_Text(pDX, IDC_EDT_OUT, m_Out);
   DDV_MaxChars(pDX, m_Out, 260);
	DDX_Text(pDX, IDC_EDT_MAP, m_Map);
	DDV_MaxChars(pDX, m_Map, 255);
	//}}AFX_DATA_MAP
}

/* ******************************************************************** **
** @@ CListerDlg::MESSAGE_MAP()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

BEGIN_MESSAGE_MAP(CListerDlg, CDialog)
   //{{AFX_MSG_MAP(CListerDlg)
   ON_WM_PAINT()
   ON_WM_QUERYDRAGICON()
   ON_BN_CLICKED(IDC_BTN_SRC, OnBtnSrc)
   ON_BN_CLICKED(IDC_BTN_OUT, OnBtnOut)
	ON_BN_CLICKED(IDC_BTN_MAP, OnBtnMap)
   ON_BN_CLICKED(IDC_BTN_GO, OnBtnGo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/* ******************************************************************** **
** @@ CListerDlg::OnInitDialog()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

BOOL CListerDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

   // Set the icon for this dialog.  The framework does this automatically
   //  when the application's main window is not a dialog
   SetIcon(m_hIcon,TRUE);     // Set big icon
   SetIcon(m_hIcon,FALSE);    // Set small icon

   m_SpMin.SetRange(1,255);
   m_SpMax.SetRange(1,255);

   m_SpMin.SetPos(1);
   m_SpMax.SetPos(255);

   GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
   GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);

   return TRUE;  // return TRUE  unless you set the focus to a control
}

/* ******************************************************************** **
** @@ CListerDlg::OnPaint()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

// If you add a minimize button to your dialog, you will need the code below
// to draw the icon.  For MFC applications using the document/view model,
// this is automatically done for you by the framework.

void CListerDlg::OnPaint()
{
   if (IsIconic())
   {
      CPaintDC dc(this); // device context for painting

      SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

      // Center icon in client rectangle
      int cxIcon = GetSystemMetrics(SM_CXICON);
      int cyIcon = GetSystemMetrics(SM_CYICON);
      CRect rect;
      GetClientRect(&rect);
      int x = (rect.Width() - cxIcon + 1) / 2;
      int y = (rect.Height() - cyIcon + 1) / 2;

      // Draw the icon
      dc.DrawIcon(x, y, m_hIcon);
   }
   else
   {
      CDialog::OnPaint();
   }
}

/* ******************************************************************** **
** @@ CListerDlg::OnQueryDragIcon()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

// The system calls this to obtain the cursor to display while the user drags
// the minimized window.

HCURSOR CListerDlg::OnQueryDragIcon()
{
   return (HCURSOR) m_hIcon;
}

/* ******************************************************************** **
** @@ CListerDlg::GetWord()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int CListerDlg::GetWord()
{
   memset(_pszWord,0,sizeof(_pszWord));

   if (!_Src.IsOpen())
   {
      return 0;
   }

   if (_dwPos >= (_dwSize - 1))
   {
      return 0;
   }

   // Skip
   while (!_pXLAT[_pText[_dwPos]])
   {
      ++_dwPos;

      if ((_dwPos - _dwKeep) > _dwDelta)
      {
         m_Progress.SetPos(_dwPos);
         _dwKeep = _dwPos;
      }

      if (_dwPos >= (_dwSize - 1))
      {
         return 0;
      }
   }

   int      iSize = 0;

   DWORD    dwStart = _dwPos;

   // Collect
   while (_pXLAT[_pText[_dwPos]])
   {
      ++_dwPos;
      ++iSize;

      if ((_dwPos - _dwKeep) > _dwDelta)
      {
         m_Progress.SetPos(_dwPos);
         _dwKeep = _dwPos;
      }

      if ((_dwPos >= (_dwSize - 1)) || (iSize > MAX_WORD_SIZE))
      {
         break;
      }
   }

   memcpy(_pszWord,&_pText[dwStart],iSize);

   return iSize;
}

/* ******************************************************************** **
** @@ CListerDlg::OnCancel()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnCancel()
{
   Cleanup();

   CDialog::OnCancel();
}

/* ******************************************************************** **
** @@ CListerDlg::Cleanup()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::Cleanup()
{
   _Src.Close();
}

/* ******************************************************************** **
** @@ CListerDlg::OnBtnSrc()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnBtnSrc()
{
   DWORD    dwFlags =   OFN_ENABLESIZING     |
                        OFN_FILEMUSTEXIST    |
                        OFN_PATHMUSTEXIST;
                                    
   char     pszFilter[MAX_PATH] = "TXT  (*.txt)|*.txt|"
                                  "DLL  (*.dll)|*.dll|"
                                  "EML  (*.eml)|*.eml|"
                                  "EXE  (*.exe)|*.exe|"
                                  "FB2  (*.fb2)|*.fb2|"
                                  "HTML  (*.htm*)|*.htm*|"
                                  "LIB  (*.lib)|*.lib|"
                                  "ALL  (*.*)|*.*||";

   CFileDialog    FileSrc(TRUE,NULL,NULL,dwFlags,pszFilter);
   
   if (FileSrc.DoModal() == IDOK)
   {  
      m_Src = FileSrc.GetPathName();

      SetDlgItemText(IDC_EDT_SRC,(LPCTSTR)m_Src);
   }
}

/* ******************************************************************** **
** @@ CListerDlg::OnBtnOut()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnBtnOut()
{
   DWORD    dwFlags = OFN_EXPLORER | OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

   CFileDialog       FileOut(FALSE,"TXT","output.txt",dwFlags);

   if (FileOut.DoModal() == IDOK)
   {
      m_Out = FileOut.GetPathName(); 

      SetDlgItemText(IDC_EDT_OUT,(LPCTSTR)m_Out);
   }
}

/* ******************************************************************** **
** @@ CListerDlg::OnBtnMap()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnBtnMap()
{
   DWORD    dwFlags =   OFN_ENABLESIZING     |
                        OFN_FILEMUSTEXIST    |
                        OFN_PATHMUSTEXIST;
                                    
   char     pszFilter[MAX_PATH] = "XLT  (*.xlt)|*.xlt|"
                                  "ALL  (*.*)|*.*||";

   CFileDialog    FileXlt(TRUE,NULL,NULL,dwFlags,pszFilter);
   
   if (FileXlt.DoModal() == IDOK)
   {  
      m_Map = FileXlt.GetPathName();

      SetDlgItemText(IDC_EDT_MAP,(LPCTSTR)m_Map);
   }
}

/* ******************************************************************** **
** @@ CListerDlg::OnBtnGo()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnBtnGo()
{
   CWaitCursor    Waiter;

   GetDlgItem(IDC_STT_READY)->ShowWindow(FALSE);
   GetDlgItem(IDC_PROGRESS)-> ShowWindow(TRUE);

   LoadMap();

   UpdateData();

   if (_Src.IsOpen())
   {
      _Src.Close();
   }

   if (!_Src.OpenReadOnly((LPCTSTR)m_Src))
   {
      AfxMessageBox("Err: [OnBtnSrc]: Can't open Src file.",MB_OK | MB_ICONSTOP);
      GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
      GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);
      Waiter.Restore();
      return;
   }

   // Reset ALL
   _pText   = _Src.Buffer();
   _dwSize  = _Src.Size();
   _dwPos   = 0;
   _dwKeep  = 0;
   _dwDelta = (_dwSize >> 7) + 1;

   m_Progress.SetRange32(0,_dwSize);

   if (!m_Out.GetLength())
   {
      AfxMessageBox("Err: [OnBtnGo]: Gimme Out file name.",MB_OK | MB_ICONSTOP);
      GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
      GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);
      Waiter.Restore();
      return;
   }

   FILE*    pOut = fopen((LPCTSTR)m_Out,"wt");

   if (!pOut)
   {
      // Error !
      GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
      GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);
      Waiter.Restore();
      return;
   }

   char     pszTempPath[MAX_PATH + 1];

   memset(pszTempPath,0,sizeof(pszTempPath));

   // Windows NT/2000 or later: 
   // The GetTempPath function does not verify that the directory 
   // specified by the TMP or TEMP environment variables exists. 
   if (GetTempPath(MAX_PATH,pszTempPath))
   {
      EnsureDirectory(pszTempPath);
   }

   char     pszTempIdx[MAX_PATH + 1];
   char     pszTempDB [MAX_PATH + 1];

   if (!GetTempFileName(pszTempPath,"@IX",0,pszTempIdx))
   {
      // Error !
   }

   if (!GetTempFileName(pszTempPath,"@DB",0,pszTempDB))
   {
      // Error !
   }

   BPPT_INDEX_INFO      Info;

   memset(&Info,0,sizeof(Info));

   strcpy(Info._pszIndexName,pszTempIdx);

   Info._pCompare = CMP_Hash;

   Info._bDuplicate   = false;
   Info._bInitialized = false;  
                                    
   Info._iKeySize    = 20;          // 160 bits Hash 
   Info._iSectorSize = (1 << 16);   // 64 K

   if (!_Index.Open(Info))
   {
      // Error !
   }

   HANDLE   hDB = CreateFile(pszTempDB);

   if (hDB == INVALID_HANDLE_VALUE)
   {
      // Error !
      DeleteFile(pszTempIdx);
      GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
      GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);
      Waiter.Restore();
      return;
   }
   
   Calc(hDB);

   _Src.Close();

   CloseHandle(hDB);
   hDB = INVALID_HANDLE_VALUE;

   _Index.Close();

   Report(pszTempDB,pOut);

   fclose(pOut);
   pOut = NULL;

   DeleteFile(pszTempIdx);
   DeleteFile(pszTempDB);

   GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
   GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::Calc()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::Calc(HANDLE hDB)
{
   int      iLength = 0;

   SHA160   Hash;

   while (iLength = GetWord())
   {
      if ((iLength >= (int)m_Min) && (iLength <= (int)m_Max))
      {
         Hash.Reset();

         Hash.Update((BYTE*)_pszWord,iLength);
         Hash.Finalize();

         DWORD    dwOfs = GetFileSizeLo(hDB);

         if (_Index.Insert((char*)Hash.Digest(),dwOfs))
         {
            DWORD    dwCount = 1;

            if (!WriteBuffer(hDB,&dwCount,sizeof(DWORD)))
            {
               // Error !
               return;
            }

            if (!WriteBuffer(hDB,&iLength,sizeof(BYTE)))
            {
               // Error !
               return;
            }

            if (!WriteBuffer(hDB,_pszWord,iLength + 1))
            {
               // Error !
               return;
            }
         }
         else
         {
            DWORD    dwValue = 0;
            DWORD    dwCount = 0;

            char     pszStr[MAX_PATH + 1];

            memset(pszStr,0,sizeof(pszStr));

            if (!_Index.Find((char*)Hash.Digest(),&dwValue))
            {
               // Error !
               return;
            }

            // Set Offset 
            // Record start with Counter field
            if (SetFilePointerCUR(hDB,dwValue,FILE_BEGIN) != dwValue)
            {
               // Error !
               return;
            }

            if (!ReadBuffer(hDB,&dwCount,sizeof(DWORD)))
            {
               // Error !
               return;
            }

            ++dwCount;

            // Set Offset Again after Read
            if (SetFilePointerCUR(hDB,dwValue,FILE_BEGIN) != dwValue)
            {
               // Error !
               return;
            }

            // Update Counter
            if (!WriteBuffer(hDB,&dwCount,sizeof(DWORD)))
            {
               // Error !
               return;
            }

            // Goto EOF
            if (SetFilePointerEOF(hDB) != dwOfs)
            {
               // Error !
               return;
            }
         }
      }
   }
}

/* ******************************************************************** **
** @@ CListerDlg::Report()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::Report(const char* const pszIn,FILE* pOut)
{
   if (!_Src.OpenReadOnly(pszIn))
   {
      AfxMessageBox("Err: [Report]: Can't open Src file.",MB_OK | MB_ICONSTOP);
      return;
   }

   BYTE*    pBuf   = _Src.Buffer();
   DWORD    dwSize = _Src.Size();
   DWORD    dwPos  = 0;

   while (dwPos < dwSize)
   {
      DB_RECORD*     pRecord = (DB_RECORD*)(pBuf + dwPos);

      dwPos += sizeof(DWORD) + 1 + pRecord->_bySize + 1;
   
      fprintf(pOut,"%3d  %12lu  %s\n",pRecord->_bySize,pRecord->_dwCount,pRecord->_pszWord);
   }
      
   _Src.Close();
}

/* ******************************************************************** **
** @@ CListerDlg::LoadMap()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::LoadMap()
{
   if (!_Src.OpenReadOnly((LPCTSTR)m_Map) || (_Src.Size() < ASCII_SIZE))
   {
      return;
   }

   memcpy(_pXLAT,_Src.Buffer(),ASCII_SIZE);

   _Src.Close();
}

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */
