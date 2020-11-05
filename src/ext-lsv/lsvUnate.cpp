#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include "sat/cnf/cnf.h"

extern "C" {
Aig_Man_t* Abc_NtkToDar(Abc_Ntk_t*, int, int);
}

static void printPoUnateInfo(Abc_Ntk_t*, Abc_Obj_t*, int, Vec_Ptr_t*,
                             Vec_Ptr_t*, Vec_Ptr_t*, int);
static void printNodeUnateInfoLsvFormat(Abc_Obj_t*, Vec_Ptr_t*, Vec_Ptr_t*,
                                        Vec_Ptr_t*);
static void printNodeUnateInfoAbcFormat(Abc_Ntk_t*, int, Vec_Ptr_t*, Vec_Ptr_t*,
                                        Vec_Ptr_t*);
static void printObjNameInVec(Vec_Ptr_t*);
static int Vec_PtrSortCompareObjId(void** pp1, void** pp2) {
  Abc_Obj_t* pObj1 = (Abc_Obj_t*)*pp1;
  Abc_Obj_t* pObj2 = (Abc_Obj_t*)*pp2;
  if (Abc_ObjId(pObj1) < Abc_ObjId(pObj2)) return -1;
  if (Abc_ObjId(pObj1) > Abc_ObjId(pObj2)) return 1;
  return 0;
}

static void writeCnfIntoSolver(Cnf_Dat_t*, sat_solver*);
static bool isUnate(sat_solver*, Cnf_Dat_t*, Cnf_Dat_t*, Vec_Int_t*, Aig_Man_t*,
                    int, bool);
static bool isPositiveUnate(sat_solver* pSat, Cnf_Dat_t* pCnfPos,
                            Cnf_Dat_t* pCnfNeg, Vec_Int_t* pVecEnVars,
                            Aig_Man_t* pMan, int pIndexCi) {
  return isUnate(pSat, pCnfPos, pCnfNeg, pVecEnVars, pMan, pIndexCi, true);
}
static bool isNegativeUnate(sat_solver* pSat, Cnf_Dat_t* pCnfPos,
                            Cnf_Dat_t* pCnfNeg, Vec_Int_t* pVecEnVars,
                            Aig_Man_t* pMan, int pIndexCi) {
  return isUnate(pSat, pCnfPos, pCnfNeg, pVecEnVars, pMan, pIndexCi, false);
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
      printNodeUnateInfoLsvFormat(pNode, vPosUnate, vNegUnate, vBinate);
    } else {
      printNodeUnateInfoLsvFormat(pNode, vNegUnate, vPosUnate, vBinate);
    }
  }
  Vec_PtrFree(vPosUnate);
  Vec_PtrFree(vNegUnate);
  Vec_PtrFree(vBinate);
}

void printPoUnateInfo(Abc_Ntk_t* pNtk, Abc_Obj_t* pPo, int indexPo,
                      Vec_Ptr_t* pVecPosUnate, Vec_Ptr_t* pVecNegUnate,
                      Vec_Ptr_t* pVecBinate, int fLsvOutputFormat) {
  if (fLsvOutputFormat) {
    printNodeUnateInfoLsvFormat(pPo, pVecPosUnate, pVecNegUnate, pVecBinate);
  } else {
    printNodeUnateInfoAbcFormat(pNtk, indexPo, pVecPosUnate, pVecNegUnate,
                                pVecBinate);
  }
}

void printNodeUnateInfoLsvFormat(Abc_Obj_t* pNode, Vec_Ptr_t* pVecPosUnate,
                                 Vec_Ptr_t* pVecNegUnate,
                                 Vec_Ptr_t* pVecBinate) {
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

void printNodeUnateInfoAbcFormat(Abc_Ntk_t* pNtk, int indexPo,
                                 Vec_Ptr_t* pVecPosUnate,
                                 Vec_Ptr_t* pVecNegUnate,
                                 Vec_Ptr_t* pVecBinate) {
  if (Vec_PtrSize(pVecPosUnate) || Vec_PtrSize(pVecNegUnate) ||
      Vec_PtrSize(pVecBinate)) {
    printf("Out%4d : ", indexPo);
    Abc_Obj_t* pPi;
    int j;
    Abc_NtkForEachPi(pNtk, pPi, j) {
      bool isPosUnate = false;
      bool isNegUnate = false;
      if (Vec_PtrFind(pVecPosUnate, pPi) != -1) {
        isPosUnate = true;
      }
      if (Vec_PtrFind(pVecNegUnate, pPi) != -1) {
        isNegUnate = true;
      }
      if (isPosUnate && !isNegUnate) printf("p");
      if (!isPosUnate && isNegUnate) printf("n");
      if (!isPosUnate && !isNegUnate) printf(".");
      if (isPosUnate && isNegUnate) printf(" ");
    }
    printf("\n");
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

void Lsv_NtkPrintPoUnate(Abc_Ntk_t* pNtk, int fLsvOutputFormat) {
  Vec_Ptr_t* vPosUnate = Vec_PtrAlloc(Abc_NtkPiNum(pNtk));
  Vec_Ptr_t* vNegUnate = Vec_PtrAlloc(Abc_NtkPiNum(pNtk));
  Vec_Ptr_t* vBinate = Vec_PtrAlloc(Abc_NtkPiNum(pNtk));
  Abc_Obj_t* pPo;
  int i;
  Abc_NtkForEachPo(pNtk, pPo, i) {
    Vec_PtrClear(vPosUnate);
    Vec_PtrClear(vNegUnate);
    Vec_PtrClear(vBinate);
    Abc_Ntk_t* pNtkCone =
        Abc_NtkCreateCone(pNtk, Abc_ObjFanin0(pPo), Abc_ObjName(pPo), 0);
    if (Abc_ObjFaninC0(pPo)) {
      Abc_ObjSetFaninC(Abc_NtkPo(pNtkCone, 0), 0);
    }
    Abc_Obj_t* pPi;
    int j;
    Abc_NtkForEachPi(pNtk, pPi, j) {
      if (!Abc_NtkFindCi(pNtkCone, Abc_ObjName(pPi))) {
        Vec_PtrPush(vPosUnate, pPi);
        Vec_PtrPush(vNegUnate, pPi);
      }
    }
    Vec_Ptr_t* vPiMapping = Vec_PtrStart(Abc_NtkPiNum(pNtkCone));
    Abc_NtkForEachPi(pNtkCone, pPi, j) {
      Vec_PtrWriteEntry(vPiMapping, j, Abc_NtkFindCi(pNtk, Abc_ObjName(pPi)));
    }
    Aig_Man_t* pMan = Abc_NtkToDar(pNtkCone, 0, 0);
    Cnf_Dat_t* pCnfPositiveCofactor = Cnf_Derive(pMan, Aig_ManCoNum(pMan));
    Cnf_Dat_t* pCnfNegativeCofactor = Cnf_DataDup(pCnfPositiveCofactor);
    Cnf_DataLift(pCnfNegativeCofactor, pCnfPositiveCofactor->nVars);
    sat_solver* pSatSolver = sat_solver_new();
    sat_solver_setnvars(
        pSatSolver, pCnfPositiveCofactor->nVars + pCnfNegativeCofactor->nVars);
    writeCnfIntoSolver(pCnfPositiveCofactor, pSatSolver);
    writeCnfIntoSolver(pCnfNegativeCofactor, pSatSolver);
    Vec_Int_t* vEnableVars = Vec_IntAlloc(Aig_ManCiNum(pMan));
    Aig_Obj_t* pCi;
    Aig_ManForEachCi(pMan, pCi, j) {
      Vec_IntPush(vEnableVars, sat_solver_addvar(pSatSolver));
      sat_solver_add_buffer_enable(
          pSatSolver, pCnfPositiveCofactor->pVarNums[Aig_ObjId(pCi)],
          pCnfNegativeCofactor->pVarNums[Aig_ObjId(pCi)],
          Vec_IntEntryLast(vEnableVars), 0);
    }
    Aig_ManForEachCi(pMan, pCi, j) {
      bool isPosUnate = false;
      bool isNegUnate = false;
      if (isPositiveUnate(pSatSolver, pCnfPositiveCofactor,
                          pCnfNegativeCofactor, vEnableVars, pMan, j)) {
        Vec_PtrPush(vPosUnate, (Abc_Obj_t*)Vec_PtrEntry(vPiMapping, j));
        isPosUnate = true;
      }
      if (isNegativeUnate(pSatSolver, pCnfPositiveCofactor,
                          pCnfNegativeCofactor, vEnableVars, pMan, j)) {
        Vec_PtrPush(vNegUnate, (Abc_Obj_t*)Vec_PtrEntry(vPiMapping, j));
        isNegUnate = true;
      }
      if (!isPosUnate && !isNegUnate) {
        Vec_PtrPush(vBinate, (Abc_Obj_t*)Vec_PtrEntry(vPiMapping, j));
      }
    }
    Vec_PtrSort(vPosUnate, (int (*)())Vec_PtrSortCompareObjId);
    Vec_PtrSort(vNegUnate, (int (*)())Vec_PtrSortCompareObjId);
    Vec_PtrSort(vBinate, (int (*)())Vec_PtrSortCompareObjId);
    printPoUnateInfo(pNtk, pPo, i, vPosUnate, vNegUnate, vBinate,
                     fLsvOutputFormat);
    Aig_ManStop(pMan);
    Cnf_DataFree(pCnfPositiveCofactor);
    Cnf_DataFree(pCnfNegativeCofactor);
    sat_solver_delete(pSatSolver);
    Vec_IntFree(vEnableVars);
    Abc_NtkDelete(pNtkCone);
    Vec_PtrFree(vPiMapping);
  }
  Vec_PtrFree(vPosUnate);
  Vec_PtrFree(vNegUnate);
  Vec_PtrFree(vBinate);
}

void writeCnfIntoSolver(Cnf_Dat_t* pCnf, sat_solver* pSat) {
  for (int i = 0; i < pCnf->nClauses; i++) {
    sat_solver_addclause(pSat, pCnf->pClauses[i], pCnf->pClauses[i + 1]);
  }
}

bool isUnate(sat_solver* pSat, Cnf_Dat_t* pCnfPos, Cnf_Dat_t* pCnfNeg,
             Vec_Int_t* pVecEnVars, Aig_Man_t* pMan, int pIndexCi,
             bool pPhase) {
  lit assumptions[Aig_ManCiNum(pMan) + 2 + 2];
  int Entry, i;
  Vec_IntForEachEntry(pVecEnVars, Entry, i) {
    assumptions[i] =
        (i == pIndexCi) ? toLitCond(Entry, 1) : toLitCond(Entry, 0);
  }
  assumptions[Aig_ManCiNum(pMan)] =
      toLitCond(pCnfPos->pVarNums[Aig_ObjId(Aig_ManCi(pMan, pIndexCi))], 0);
  assumptions[Aig_ManCiNum(pMan) + 1] =
      toLitCond(pCnfNeg->pVarNums[Aig_ObjId(Aig_ManCi(pMan, pIndexCi))], 1);
  assumptions[Aig_ManCiNum(pMan) + 2] =
      toLitCond(pCnfPos->pVarNums[Aig_ObjId(Aig_ManCo(pMan, 0))], pPhase);
  assumptions[Aig_ManCiNum(pMan) + 3] =
      toLitCond(pCnfNeg->pVarNums[Aig_ObjId(Aig_ManCo(pMan, 0))], !pPhase);
  if (sat_solver_solve(pSat, assumptions, assumptions + Aig_ManCiNum(pMan) + 4,
                       0, 0, 0, 0) == l_False) {
    return true;
  }
  return false;
}