#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <vector>
#include <algorithm>

static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandPrintMsfc(Abc_Frame_t* pAbc, int argc, char** argv);

void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_msfc", Lsv_CommandPrintMsfc, 0);
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
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
             Abc_ObjName(pFanin));
    }
    if (Abc_NtkHasSop(pNtk)) {
      printf("The SOP of this node:\n%s", (char*)pObj->pData);
    }
  }
}

void msfc_traversal(Abc_Obj_t *pObj ,std::vector<Abc_Obj_t*>& vec, std::vector<Abc_Obj_t*>& Ids){
  int j, k = 0;
  Abc_Obj_t *pFanin;
  Abc_ObjForEachFanin(pObj, pFanin, j) {
    if(Abc_ObjIsNode(pFanin) && Abc_ObjFanoutNum(pFanin) == 1){
      vec.push_back(pFanin);
      std::vector<Abc_Obj_t*>::iterator iter = std::find(Ids.begin(), Ids.end(), pFanin);
      Ids.erase(iter);
      msfc_traversal(pFanin, vec, Ids);
    }
  }
  return;
}

void Lsv_NtkPrintMsfc(Abc_Ntk_t* pNtk) {
  Abc_Obj_t *pObj, *tmp_Obj;
  int i = 0, j;
  Abc_NtkIncrementTravId(pNtk);
  std::vector<Abc_Obj_t*> Ids, vec;
  std::vector<std::vector<Abc_Obj_t*>> msfc;
  std::vector<Abc_Obj_t*>::iterator iter;
  Abc_NtkForEachNode(pNtk, pObj, i){
    Ids.push_back(pObj);
  }
  while(Ids.size() > 0) {
    tmp_Obj = Ids.back();
    printf("Id: %d\n", Abc_ObjId(tmp_Obj));
    Ids.pop_back();
    vec.push_back(tmp_Obj);
    msfc_traversal(tmp_Obj, vec, Ids);
    msfc.push_back(vec);
    vec.clear();
  }
  for(int k=0; k<msfc.size(); ++k){
    printf("MSFC %d: ", k);
    for(int l=0; l<msfc[k].size(); ++l){
      if(l == 0) printf("%s", Abc_ObjName(msfc[k][l]));
      else printf(",%s", Abc_ObjName(msfc[k][l]));
    }
    printf("\n");
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

int Lsv_CommandPrintMsfc(Abc_Frame_t* pAbc, int argc, char** argv) {
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
  Lsv_NtkPrintMsfc(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_msfc [-h]\n");
  Abc_Print(-2, "\t        prints all msfc in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}