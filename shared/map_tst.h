/* ******************************************************************** **
** @@ Ternary Search Tree
** @  Copyrt :
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

#ifndef _MAP_TST_HPP_
#define _MAP_TST_HPP_

/* ******************************************************************** **
** @@ internal defines
** ******************************************************************** */

struct TST_Node;

typedef void (*VISIT)(const TST_Node* const pNode);

/* ******************************************************************** **
** @@ internal prototypes
** ******************************************************************** */

/* ******************************************************************** **
** @@ external global variables
** ******************************************************************** */

/* ******************************************************************** **
** @@ static global variables
** ******************************************************************** */

/* ******************************************************************** **
** @@ Classes
** ******************************************************************** */

/* ******************************************************************** **
** @@ struct TST_Node
** @  Copyrt :
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

struct TST_Node
{       
   #ifdef _DEBUG
   DWORD          _dwID;
   DWORD          _dwParentID;
   #endif   
   TST_Node*      _pLo;
   TST_Node*      _pEq;
   TST_Node*      _pHi;
   char           _cSplitter;
};

/* ******************************************************************** **
** @@ class TernarySearchTree
** @  Copyrt :
** @  Author : 
** @  Modify :
** @  Update :
** @  Notes  :
** ******************************************************************** */

class TST
{  
   private:
      
      TST_Node*      _pRoot;
      #ifdef _DEBUG
      DWORD          _dwID;
      DWORD          _dwParentID;
      #endif   

   public:

      void*          _pData;      
      int            _iNodes;
      bool           _bInserted;

   public:
               
                TST();
      virtual  ~TST();
               
      bool     Insert(char* pStr);
      void*    Search(char* pStr);
      void     Dump(VISIT pVisit);

   private:
                  
      TST_Node*   Insert(TST_Node* pNode,char* pStr);
      void        Cleanup(TST_Node* pNode);
      bool        Search(TST_Node* p,char* pStr);
      void        PMSearch(TST_Node* p,char* s);
      void        NearSearch(TST_Node* p,char* s,int d);
      void        Visit(TST_Node* p,VISIT pVisit);
};

#endif

/* ******************************************************************** **
** End of File
** ******************************************************************** */
