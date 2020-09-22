#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"

extern void Lsv_NtkPrintSopUnate(Abc_Ntk_t*);

static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandPrintSopUnate(Abc_Frame_t* pAbc, int argc, char** argv);

void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_sopunate", Lsv_CommandPrintSopUnate,
                 0);
}

void destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {init, destroy};

struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;

void Lsv_NtkPrintNodes(Abc_Ntk_t* pNtk) {
  Abc_Obj_t* pObj;
  int i;
  Abc_NtkForEachNode(pNtk, pObj, i) {
    printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    if (Abc_NtkHasSop(pNtk)) {
      char* pSop = (char*)pObj->pData;
      printf("The SOP of this node:\n%s", pSop);
      printf("The phase of the SOP: %d\n", Abc_SopGetPhase(pSop));
    }
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
             Abc_ObjName(pFanin));
      if (Abc_NtkHasSop(pNtk)) {
        printf("  Phase of this fanin:");
        char* pSop = (char*)pObj->pData;
        char* pCube;
        Abc_SopForEachCube(pSop, Abc_SopGetVarNum(pSop), pCube) {
          printf(" %c", pCube[j]);
        }
        printf("\n");
      }
    }
  }
}

int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  Lsv_NtkPrintNodes(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_nodes [-h]\n");
  Abc_Print(-2, "\t        prints the nodes in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}

int Lsv_CommandPrintSopUnate(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    switch (c) {
      case 'h':
        goto usage;
      default:
        goto usage;
    }
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  if (!Abc_NtkHasSop(pNtk)) {
    Abc_Print(-1, "The network does not have SOP.\n");
    return 1;
  }
  Lsv_NtkPrintSopUnate(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_sopunate [-h]\n");
  Abc_Print(-2,
            "\t        prints the unate information of the SOP nodes in the "
            "network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}