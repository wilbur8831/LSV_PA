#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <vector>
#include <algorithm>
#include <cassert>
#define DEBUG 0

static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandPrintMSFC(Abc_Frame_t* pAbc, int argc, char** argv);

void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_msfc", Lsv_CommandPrintMSFC, 0);
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

bool mycompare(Abc_Obj_t*& a, Abc_Obj_t*& b){
  return Abc_ObjId(a) < Abc_ObjId(b);
} 

bool mycompare2(std::vector<Abc_Obj_t*>& a, std::vector<Abc_Obj_t*>& b){
  assert(!a.empty());
  assert(!b.empty());
  return Abc_ObjId(a[0]) < Abc_ObjId(b[0]);
}

void Lsv_NtkPrintMSFC(Abc_Ntk_t* pNtk) {
  // get reverse dfs
  Vec_Ptr_t* DfsList = Abc_NtkDfsReverse(pNtk);

  // get the number of internal nodes
  int numNodes = DfsList->nSize;
  
  // count the number of nodes
  if(DEBUG)
    printf("#nodes: %d\n",numNodes);

  // print the reverse dfs result and their #fanouts
  if(DEBUG){
    printf("-----dfs----\n");
    for(int i = 0; i < numNodes; i++){
      Abc_Obj_t* pNode = (Abc_Obj_t *) DfsList->pArray[i];
      printf("Node: %d  #Fanouts: %d\n", Abc_ObjId(pNode), Abc_ObjFanoutNum(pNode));
    }
    printf("------------\n");
  }

  // change all the internal nodes' iTemp(visited or not) to 0
  for(int i = 0; i < numNodes; i++){
    Abc_Obj_t* pNode = (Abc_Obj_t *) DfsList->pArray[i];
    pNode->iTemp = 0;
  }

  Abc_Obj_t* const1Node = Abc_AigConst1(pNtk);
  const1Node->iTemp = 0;

  // change all the pi nodes' iTemp to 1
  for(int i = 0; i < Abc_NtkPiNum(pNtk); i++){
    Abc_Obj_t* pNode = Abc_NtkPi(pNtk, i);
    pNode->iTemp = 1;
  }


  // use dfs to find msfc
  std::vector<Abc_Obj_t*> S;
  std::vector<std::vector<Abc_Obj_t*> > res;
  for(int i = 0; i < numNodes; i++){
  
    Abc_Obj_t* pNode = (Abc_Obj_t *) DfsList->pArray[i];
    if(pNode->iTemp == 1) continue;

    std::vector<Abc_Obj_t*> group;
    S.push_back(pNode);
    while(!S.empty()){
      Abc_Obj_t* curNode = S.back();
      S.pop_back();
      if(curNode == pNode){
        group.push_back(curNode);
        Abc_Obj_t* child0Node = Abc_ObjFanin0(curNode);
        Abc_Obj_t* child1Node = Abc_ObjFanin1(curNode);
        if(child0Node->iTemp == 0) S.push_back(child0Node);
        if(child1Node->iTemp == 0) S.push_back(child1Node);
        curNode->iTemp = 1;
      }
      else if(Abc_ObjFanoutNum(curNode) == 1){
        group.push_back(curNode);
        if(!Abc_AigNodeIsConst(curNode)){
          Abc_Obj_t* child0Node = Abc_ObjFanin0(curNode);
          Abc_Obj_t* child1Node = Abc_ObjFanin1(curNode);
          if(child0Node->iTemp == 0) S.push_back(child0Node);
          if(child1Node->iTemp == 0) S.push_back(child1Node);
        }
        curNode->iTemp = 1;
      }
    }
    res.push_back(group);
  }

  // process const 1 node
  if(const1Node->iTemp == 0 && Abc_ObjFanoutNum(const1Node) > 0){
    std::vector<Abc_Obj_t*> group;
    group.push_back(const1Node);
    res.push_back(group);
  }

  // sort the id
  for(int i = 0; i < res.size(); ++i){
    sort(res[i].begin(), res[i].end(), mycompare);
  }

  sort(res.begin(), res.end(), mycompare2);

  // print the msfc result
  for(int i = 0; i < res.size(); ++i){
    printf("MSFC %d: ", i);
    for(int j = 0; j < res[i].size(); ++j){
      printf("%s", Abc_ObjName(res[i][j]));
      if(j != res[i].size()-1) printf(",");
    }
    printf("\n");
  }
}

int Lsv_CommandPrintMSFC(Abc_Frame_t* pAbc, int argc, char** argv) {
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
  if(!Abc_NtkIsStrash(pNtk)){
    Abc_Print(-1, "Current network is not an AIG.\n");
    return 1;
  }
  Lsv_NtkPrintMSFC(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_msfc [-h]\n");
  Abc_Print(-2, "\t        prints the MSFC in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
  
}
