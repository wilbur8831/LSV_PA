#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

static void printNodeUnateInfo(Abc_Obj_t*, Vec_Ptr_t*, Vec_Ptr_t*, Vec_Ptr_t*);
static void printObjNameInVec(Vec_Ptr_t*);
static int Vec_PtrSortCompareObjId(void** pp1, void** pp2) {
  Abc_Obj_t* pObj1 = (Abc_Obj_t*)*pp1;
  Abc_Obj_t* pObj2 = (Abc_Obj_t*)*pp2;
  if (Abc_ObjId(pObj1) < Abc_ObjId(pObj2)) return -1;
  if (Abc_ObjId(pObj1) > Abc_ObjId(pObj2)) return 1;
  return 0;
}

void Lsv_NtkPrintSopUnate(Abc_Ntk_t* pNtk) {
  Vec_Ptr_t* vPosUnate = Vec_PtrAlloc(8);
  Vec_Ptr_t* vNegUnate = Vec_PtrAlloc(8);
  Vec_Ptr_t* vBinate = Vec_PtrAlloc(8);
  Abc_Obj_t* pNode;
  int i;
  Abc_NtkForEachNode(pNtk, pNode, i) {
    Vec_PtrClear(vPosUnate);
    Vec_PtrClear(vNegUnate);
    Vec_PtrClear(vBinate);
    char* pSop = (char*)pNode->pData;
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pNode, pFanin, j) {
      bool hasPosLit = false, hasNegLit = false;
      char* pCube;
      Abc_SopForEachCube(pSop, Abc_SopGetVarNum(pSop), pCube) {
        if (!hasPosLit && pCube[j] == '1') {
          hasPosLit = true;
        }
        if (!hasNegLit && pCube[j] == '0') {
          hasNegLit = true;
        }
      }
      if (!hasNegLit) {
        Vec_PtrPush(vPosUnate, pFanin);
      }
      if (!hasPosLit) {
        Vec_PtrPush(vNegUnate, pFanin);
      }
      if (hasPosLit && hasNegLit) {
        Vec_PtrPush(vBinate, pFanin);
      }
    }
    Vec_PtrSort(vPosUnate, (int (*)())Vec_PtrSortCompareObjId);
    Vec_PtrSort(vNegUnate, (int (*)())Vec_PtrSortCompareObjId);
    Vec_PtrSort(vBinate, (int (*)())Vec_PtrSortCompareObjId);
    if (Abc_SopGetPhase(pSop)) {
      printNodeUnateInfo(pNode, vPosUnate, vNegUnate, vBinate);
    } else {
      printNodeUnateInfo(pNode, vNegUnate, vPosUnate, vBinate);
    }
  }
  Vec_PtrFree(vPosUnate);
  Vec_PtrFree(vNegUnate);
  Vec_PtrFree(vBinate);
}

void printNodeUnateInfo(Abc_Obj_t* pNode, Vec_Ptr_t* pVecPosUnate,
                        Vec_Ptr_t* pVecNegUnate, Vec_Ptr_t* pVecBinate) {
  if (Vec_PtrSize(pVecPosUnate) || Vec_PtrSize(pVecNegUnate) ||
      Vec_PtrSize(pVecBinate)) {
    printf("node %s:\n", Abc_ObjName(pNode));
    if (Vec_PtrSize(pVecPosUnate)) {
      printf("+unate inputs: ");
      printObjNameInVec(pVecPosUnate);
    }
    if (Vec_PtrSize(pVecNegUnate)) {
      printf("-unate inputs: ");
      printObjNameInVec(pVecNegUnate);
    }
    if (Vec_PtrSize(pVecBinate)) {
      printf("binate inputs: ");
      printObjNameInVec(pVecBinate);
    }
  }
}

void printObjNameInVec(Vec_Ptr_t* pVec) {
  Abc_Obj_t* pFanin;
  int j;
  Vec_PtrForEachEntry(Abc_Obj_t*, pVec, pFanin, j) {
    printf("%s", Abc_ObjName(pFanin));
    if (j < Vec_PtrSize(pVec) - 1) {
      printf(",");
    }
  }
  printf("\n");
}