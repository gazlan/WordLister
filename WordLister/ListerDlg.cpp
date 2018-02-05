/* ******************************************************************** **
** @@ Lister
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

/* ******************************************************************** **
**                uses pre-compiled headers
** ******************************************************************** */

#include "stdafx.h"

#include <afxrich.h>     
                 
#include "..\shared\mmf.h"
#include "..\shared\file.h"
#include "..\shared\map_tst.h"
#include "..\shared\map_bppt_jannink.h"
#include "..\shared\hash_murmur2_160.h"
#include "..\shared\hash_sha160.h"
#include "..\shared\hash_hsieh.h"
#include "..\shared\vector.h"

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

#define STOP_WORDS_CNT           (84)
#define MAX_WORD_SIZE            (255)

#define DEFAULT_VECTOR_SIZE      (256)
#define DEFAULT_VECTOR_DELTA     (32)

enum WL_METHOD
{
   WL_MTH_UNCONCERN,
   WL_MTH_STRONG,
   WL_MTH_RIGOROUSLY
};

enum WL_STOP
{
   WL_STP_NONE,
   WL_STP_INTERNAL,
   WL_STP_EXTERNAL
};

// Mersenne Twister Period parameters
#define N            (624)
#define M            (397)
#define MATRIX_A     (0x9908B0DF)    // constant vector a
#define UPPER_MASK   (0x80000000)    // most significant w-r bits
#define LOWER_MASK   (0x7FFFFFFF)    // least significant r bits

static DWORD      mt[N];               // the array for the state vector
static int        mti = N + 1;         // mti == N + 1 means mt[N] is not initialized

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

const char pXLAT_ENGLISH_LETTERS[256] =
{
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', '\x48', '\x49', '\x4A', '\x4B', '\x4C', '\x4D', '\x4E', '\x4F',
   '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57', '\x58', '\x59', '\x5A', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x61', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67', '\x68', '\x69', '\x6A', '\x6B', '\x6C', '\x6D', '\x6E', '\x6F',
   '\x70', '\x71', '\x72', '\x73', '\x74', '\x75', '\x76', '\x77', '\x78', '\x79', '\x7A', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00',
   '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
};

const char* STOP_WORDS[STOP_WORDS_CNT] =
{
   "accordance",
   "according",
   "all",
   "also",
   "and",
   "another",
   "are",
   "because",
   "been",
   "being",
   "claim",
   "comprises",
   "corresponding",
   "could",
   "described",
   "desired",
   "does",
   "each",
   "embodiment",
   "fig",
   "figs",
   "for",
   "from",
   "further",
   "generally",
   "had",
   "has",
   "have",
   "having",
   "herein",
   "however",
   "into",
   "invention",
   "its",
   "means",
   "not",
   "now",
   "onto",
   "other",
   "particularly",
   "preferably",
   "preferred",
   "present",
   "provide",
   "provided",
   "provides",
   "relatively",
   "respectively",
   "said",
   "should",
   "since",
   "some",
   "such",
   "suitable",
   "than",
   "that",
   "the",
   "their",
   "then",
   "there",
   "thereby",
   "therefore",
   "thereof",
   "thereto",
   "these",
   "they",
   "this",
   "those",
   "thus",
   "use",
   "various",
   "was",
   "were",
   "what",
   "when",
   "where",
   "whereby",
   "wherein",
   "which",
   "while",
   "who",
   "will",
   "with",
   "would"
};

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
** @@ InitTwister()
** @  Copyrt : 
** @  Author : 
** @  Update :
** @  Update :
** @  Notes  : Initializes mt[N] with a seed
** ******************************************************************** */

static void InitTwister(DWORD dwSeed)
{  
   mt[0] = dwSeed & 0xFFFFFFFF;

   for (mti = 1; mti < N; mti++) 
   {
      // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. 
      // In the previous versions, MSBs of the seed affect   
      // only MSBs of the array mt[].                        
      // 2002/01/09 modified by Makoto Matsumoto             
      mt[mti] = (1812433253 * (mt[mti - 1] ^ (mt[mti - 1] >> 30)) + mti); 
      
      // for > 32 bit machines
      mt[mti] &= 0xFFFFFFFF;
   }
}

/* ******************************************************************** **
** @@ Twist32()
** @  Copyrt :
** @  Author : 
** @  Modify :
** @  Modify : 
** @  Notes  : Generates a random number on [0,0xFFFFFFFF] interval
** ******************************************************************** */

static DWORD Twist32()
{
   DWORD    dwRand = 0;

   // mag01[x] = x * MATRIX_A  for x = 0,1
   static DWORD mag01[2] = { 0x0, MATRIX_A };

   if (mti >= N) 
   { 
      // generate N words at one time
      int   kk = 0;

      if (mti == N + 1)
      {
         // InitTwister() has not been called
         InitTwister(5489); // a default initial seed is used
      }

      for (kk=0;kk<N-M;kk++) 
      {
         dwRand = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);

         mt[kk] = mt[kk+M] ^ (dwRand >> 1) ^ mag01[dwRand & 0x1];
      }
      
      for ( ; kk < (N - 1); kk++) 
      {
         dwRand = (mt[kk]& UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);

         mt[kk] = mt[kk + (M-N)] ^ (dwRand >> 1) ^ mag01[dwRand & 0x1];
      }

      dwRand = (mt[N - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);

      mt[N - 1] = mt[M - 1] ^ (dwRand >> 1) ^ mag01[dwRand & 0x1];

      mti = 0;
   }

   dwRand = mt[mti++];

   // Tempering
   dwRand ^= (dwRand >> 11);
   dwRand ^= (dwRand <<  7) & 0x9d2c5680;
   dwRand ^= (dwRand << 15) & 0xefc60000;
   dwRand ^= (dwRand >> 18);

   return dwRand;
}

/* ******************************************************************** **
** @@ Shuffle()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

static void Shuffle(char** pWords,int iSize)
{
   InitTwister(time(NULL));

   for (int ii = iSize - 1; ii >= 0; --ii)
   {
      int      rr = Twist32() % iSize;

      char*    pTemp = pWords[ii];

      // Swap
      pWords[ii] = pWords[rr];
      pWords[rr] = pTemp;
   }
}

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
   m_Apostrophe  = TRUE;
   m_Hyphen      = TRUE;
   m_Underscore  = TRUE;
   m_Insensitive = TRUE;
   m_None      = 0;
   m_Unconcern = 0;
   m_Size  = FALSE;
   m_Count = FALSE;
   m_Src  = _T("");
   m_Out  = _T("");
   m_Stop = _T("");
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
   DDX_Check(pDX, IDC_CHK_APOSTROPHE, m_Apostrophe);
   DDX_Check(pDX, IDC_CHK_HYPHEH, m_Hyphen);
   DDX_Radio(pDX, IDC_RB_UNCONCERN, m_Unconcern);
   DDX_Check(pDX, IDC_CHK_SIZE, m_Size);
   DDX_Check(pDX, IDC_CHK_COUNT, m_Count);
   DDX_Text(pDX, IDC_EDT_SRC, m_Src);
   DDV_MaxChars(pDX, m_Src, 260);
   DDX_Text(pDX, IDC_EDT_OUT, m_Out);
   DDV_MaxChars(pDX, m_Out, 260);
   DDX_Text(pDX, IDC_EDT_STOP, m_Stop);
   DDV_MaxChars(pDX, m_Stop, 260);
   DDX_Check(pDX, IDC_CHK_INSENSITIVE, m_Insensitive);
   DDX_Check(pDX, IDC_CHK_UNDERSCORE, m_Underscore);
   DDX_Radio(pDX, IDC_RB_NONE, m_None);
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
   ON_BN_CLICKED(IDC_BTN_STOP, OnBtnStop)
   ON_BN_CLICKED(IDC_BTN_GO, OnBtnGo)
   ON_BN_CLICKED(IDC_RB_UNCONCERN, OnRbUnconcern)
   ON_BN_CLICKED(IDC_RB_RIGOROUSLY, OnRbRigorously)
   ON_BN_CLICKED(IDC_CHK_COUNT, OnChkCount)
   ON_BN_CLICKED(IDC_RB_INTERNAL, OnRbInternal)
   ON_BN_CLICKED(IDC_RB_EXTERNAL, OnRbExternal)
   ON_BN_CLICKED(IDC_RB_STRONG, OnRbStrong)
   ON_BN_CLICKED(IDC_RB_NONE, OnRbNone)
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

   m_SpMin.SetPos(3);
   m_SpMax.SetPos(32);

   GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
   GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);

   OnRbUnconcern();
   OnRbNone();

   _List.Resize(DEFAULT_VECTOR_SIZE);
   _List.Delta(DEFAULT_VECTOR_DELTA);

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

   int   iCnt = _List.Count();

   if (iCnt)
   {
      for (int ii = iCnt - 1; ii >= 0; --ii)
      {
         char*    pStr = (char*)_List.At(ii);

         if (pStr)
         {
            delete[] pStr;
            pStr = NULL; 
         }

         _List.RemoveAt(ii);
      }
   }
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

      SetDlgItemText(IDC_EDT_SRC,m_Src);
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

      SetDlgItemText(IDC_EDT_OUT,m_Out);
   }
}

/* ******************************************************************** **
** @@ CListerDlg::OnBtnStop()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnBtnStop()
{
   DWORD    dwFlags =   OFN_ENABLESIZING     |
                        OFN_FILEMUSTEXIST    |
                        OFN_PATHMUSTEXIST;
                                    
   char     pszFilter[MAX_PATH] = "TXT  (*.txt)|*.txt|"
                                  "LST  (*.lst)|*.lst|"
                                  "ALL  (*.*)|*.*||";

   CFileDialog    FileStop(TRUE,NULL,NULL,dwFlags,pszFilter);
   
   if (FileStop.DoModal() == IDOK)
   {  
      m_Stop = FileStop.GetPathName();

      SetDlgItemText(IDC_EDT_STOP,m_Stop);
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

   memcpy(_pXLAT,pXLAT_ENGLISH_LETTERS,256);

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

   if (!m_Apostrophe)
   {
      _pXLAT['\''] = '\'';
      _pXLAT[0x92] = '\x92';
   }

   if (!m_Hyphen)
   {
      _pXLAT['-'] = '-';
      _pXLAT[0x96] = '\x96';
      _pXLAT[0x97] = '\x97';
      _pXLAT[0xAD] = '\xAD';
   }

   if (!m_Underscore)
   {
      _pXLAT['_'] = '_';
   }
   
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

   char**   pStopList = NULL;
   int      iStopCnt  = 0;

   switch (m_None)
   {
      case WL_STP_NONE:
      {
         break;
      }
      case WL_STP_INTERNAL:
      {
         iStopCnt = STOP_WORDS_CNT;

         pStopList = new char*[iStopCnt];

         if (!pStopList)
         {
            // Error !
            break;
         }

         // Copy
         for (int ii = 0; ii < iStopCnt; ++ii)
         {
            pStopList[ii] = (char*)STOP_WORDS[ii];
         }

         break;
      }
      case WL_STP_EXTERNAL:
      {
         iStopCnt = LoadStopWords();

         pStopList = (char**)_List.GetData();

         break;
      }
   }

   if (iStopCnt && pStopList)
   {
      // Rearrange
      Shuffle(pStopList,iStopCnt);

      // Insert
      for (int ii = 0; ii < iStopCnt; ++ii)
      {
         _Dix._pData = pStopList[ii];
         _Dix.Insert(pStopList[ii]);
      }
   }

   switch (m_Unconcern)
   {
      case WL_MTH_UNCONCERN:
      {
         Unconcern(pOut);
         break;
      }
      case WL_MTH_STRONG:
      {
         Strong(pOut);
         break;
      }
      case WL_MTH_RIGOROUSLY:
      {
         Rigorously(hDB);
         break;
      }
   }

   _Src.Close();

   if ((m_None == WL_STP_INTERNAL) && pStopList)
   {
      delete[] pStopList;
      pStopList = NULL;
   }

   CloseHandle(hDB);
   hDB = INVALID_HANDLE_VALUE;

   _Index.Close();

   if (m_Unconcern == WL_MTH_RIGOROUSLY)
   {
      Report(pszTempDB,pOut);
   }

   fclose(pOut);
   pOut = NULL;

   DeleteFile(pszTempIdx);
   DeleteFile(pszTempDB);

   GetDlgItem(IDC_STT_READY)->ShowWindow(TRUE);
   GetDlgItem(IDC_PROGRESS)-> ShowWindow(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::OnRbUnconcern()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnRbUnconcern() 
{
   UpdateData();

   m_Count = 0;

   GetDlgItem(IDC_CHK_COUNT)->EnableWindow(FALSE);

   UpdateData(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::OnRbStrong()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnRbStrong() 
{
   UpdateData();

   m_Count = 0;

   GetDlgItem(IDC_CHK_COUNT)->EnableWindow(FALSE);

   UpdateData(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::OnRbRigorously()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnRbRigorously() 
{
   UpdateData();

   GetDlgItem(IDC_CHK_COUNT)->EnableWindow(TRUE);

   UpdateData(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::OnChkCount()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnChkCount() 
{
   UpdateData();

   if (!m_Unconcern)
   {
      m_Count = 0;
   }

   UpdateData(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::Unconcern()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::Unconcern(FILE* pOut)
{
   int      iLength = 0;

   DWORD    dwSeed = 0x9E3779B9;  // The Golden Ratio Hi
   DWORD    pMMHash[5];

   while (iLength = GetWord())
   {
      if ((iLength >= (int)m_Min) && (iLength <= (int)m_Max))
      {
         char     pszTemp[MAX_PATH + 1];

         strcpy(pszTemp,_pszWord);

         strlwr(pszTemp);

         // Check Stop words
         if ((m_None != WL_STP_NONE) && _Dix.Search(pszTemp))
         {
            // Skip this
            continue;
         }

         MurmurHash2_160(m_Insensitive  ?  pszTemp  :  _pszWord,(DWORD)iLength,dwSeed,pMMHash);

         if (_Index.Insert((char*)pMMHash,0))
         {
            if (m_Size)
            {
               fprintf(pOut,"%3d  %s\n",iLength,m_Insensitive  ?  pszTemp  :  _pszWord);
            }
            else
            {
               fprintf(pOut,"%s\n",m_Insensitive  ?  pszTemp  :  _pszWord);
            }
         }
      }
   }
}

/* ******************************************************************** **
** @@ CListerDlg::Strong()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::Strong(FILE* pOut)
{
   int      iLength = 0;

   SHA160   Hash;

   while (iLength = GetWord())
   {
      if ((iLength >= (int)m_Min) && (iLength <= (int)m_Max))
      {
         Hash.Reset();

         char     pszTemp[MAX_PATH + 1];

         strcpy(pszTemp,_pszWord);

         strlwr(pszTemp);

         // Check Stop words
         if ((m_None != WL_STP_NONE) && _Dix.Search(pszTemp))
         {
            // Skip this
            continue;
         }

         Hash.Update((BYTE*)(m_Insensitive  ?  pszTemp  :  _pszWord),iLength);
         Hash.Finalize();

         if (_Index.Insert((char*)Hash.Digest(),0))
         {
            if (m_Size)
            {
               fprintf(pOut,"%3d  %s\n",iLength,m_Insensitive  ?  pszTemp  :  _pszWord);
            }
            else
            {
               fprintf(pOut,"%s\n",m_Insensitive  ?  pszTemp  :  _pszWord);
            }
         }
      }
   }
}

/* ******************************************************************** **
** @@ CListerDlg::Rigorously()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::Rigorously(HANDLE hDB)
{
   int      iLength = 0;

   SHA160   Hash;

   while (iLength = GetWord())
   {
      if ((iLength >= (int)m_Min) && (iLength <= (int)m_Max))
      {
         Hash.Reset();

         char     pszTemp[MAX_PATH + 1];

         strcpy(pszTemp,_pszWord);

         strlwr(pszTemp);

         // Check Stop words
         if ((m_None != WL_STP_NONE) && _Dix.Search(pszTemp))
         {
            // Skip this
            continue;
         }

         Hash.Update((BYTE*)(m_Insensitive  ?  pszTemp  :  _pszWord),iLength);
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

            if (!WriteBuffer(hDB,m_Insensitive  ?  pszTemp  :  _pszWord,iLength + 1))
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
** @@ CListerDlg::OnRbNone()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnRbNone() 
{
   UpdateData();

   GetDlgItem(IDC_EDT_STOP)->EnableWindow(FALSE);
   GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);

   UpdateData(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::OnRbInternal()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnRbInternal() 
{
   UpdateData();

   GetDlgItem(IDC_EDT_STOP)->EnableWindow(FALSE);
   GetDlgItem(IDC_BTN_STOP)->EnableWindow(FALSE);

   UpdateData(FALSE);
}

/* ******************************************************************** **
** @@ CListerDlg::OnRbExternal()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

void CListerDlg::OnRbExternal() 
{
   UpdateData();

   GetDlgItem(IDC_EDT_STOP)->EnableWindow(TRUE);
   GetDlgItem(IDC_BTN_STOP)->EnableWindow(TRUE);

   UpdateData(FALSE);
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
   
      if (m_Size && m_Count)
      {
         fprintf(pOut,"%3d  %12lu  %s\n",pRecord->_bySize,pRecord->_dwCount,pRecord->_pszWord);
      }
      else if (m_Size && !m_Count)
      {
         fprintf(pOut,"%3d  %s\n",pRecord->_bySize,pRecord->_pszWord);
      }
      else if (!m_Size && m_Count)
      {
         fprintf(pOut,"%12lu  %s\n",pRecord->_dwCount,pRecord->_pszWord);
      }
      else
      {
         fprintf(pOut,"%s\n",pRecord->_pszWord);
      }
   }
   
   _Src.Close();
}

/* ******************************************************************** **
** @@ CListerDlg::LoadStopWords()
** @  Copyrt :
** @  Author :
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

int CListerDlg::LoadStopWords()
{
   FILE*    pIn = fopen((LPCTSTR)m_Stop,"rt");

   if (!pIn)
   {
      AfxMessageBox("Err: [LoadStopWords]: Can't load Stop words file.",MB_OK | MB_ICONSTOP);
      return 0;
   }

   char     pBuf[MAX_PATH + 1];

   while (fgets(pBuf,MAX_PATH,pIn))
   {
      pBuf[MAX_PATH] = 0; // ASCIIZ !

      DWORD    dwEOL = strcspn(pBuf,"\r\n");

      pBuf[dwEOL] = 0;  // Remove EOL chars

      int      iLen = strlen(pBuf);

      if (iLen)
      {
         char*    pStr = new char[iLen + 1];

         if (!pStr)
         {
            // Error !
            AfxMessageBox("Err: [LoadStopWords]: Not enough memory.",MB_OK | MB_ICONSTOP);
            return 0;
         }

         strcpy(pStr,pBuf);

         if (_List.Insert(pStr) == -1)
         {
            delete[] pStr;
            pStr = NULL;
         }
      }
   }

   fclose(pIn);
   pIn = NULL;

   return _List.Count();
}

/* ******************************************************************** **
** @@                   End of File
** ******************************************************************** */
