#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
using namespace std;

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

//----------------------------------------LSV Example----------------------------------------------//
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
//---------------------------------------------------------------------------------------------------------//

void Lsv_NtkPrintMSFC(Abc_Ntk_t* pNtk) {
  Abc_Obj_t* pObj;
  int i;
  vector<Abc_Obj_t*> v_coneFout;
  vector<vector<int> > v_msfc;
  Abc_NtkForEachPo(pNtk, pObj, i){
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      if(!Abc_ObjIsPi(pFanin)){
        if(find(v_coneFout.begin(),v_coneFout.end(),pFanin) == v_coneFout.end()){ // new msf of cone
          v_coneFout.push_back(pFanin);
          printf(" New MSF of Cone - %d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin), Abc_ObjName(pFanin));
        }
      }
    }
  }
  // for each msf, travorsal
  for(i=0;i<v_coneFout.size();++i){
    pObj = v_coneFout[i];
    vector<int> v_node;
    queue<Abc_Obj_t*> q;
    q.push(pObj);
    while(!q.empty()){ // BFS
      pObj = q.front();
      v_node.push_back(Abc_ObjId(pObj)); // put the node to this cone
      Abc_Obj_t* pFanin;
      int j;
      Abc_ObjForEachFanin(pObj, pFanin, j) {
        if(!Abc_ObjIsPi(pFanin)){ // not a PI
          if(pFanin->vFanouts.nSize > 1){ // new msf of cone
            if(find(v_coneFout.begin(),v_coneFout.end(),pFanin)==v_coneFout.end()){
              v_coneFout.push_back(pFanin);
              printf(" New MSF of Cone - %d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin), Abc_ObjName(pFanin));
            }
          }
          else{ // a node of cone
            if(find(v_node.begin(),v_node.end(),Abc_ObjId(pFanin))==v_node.end()){
              q.push(pFanin);
            }
          }
        }
      }
      q.pop();
    }
    v_msfc.push_back(v_node);
  }

  for(i=0;i<v_msfc.size();++i){
    cout << "MSFC " << i << ":";
    cout << " n" << v_msfc[i][0];
    for(int j=1;j<v_msfc[i].size();++j){
      cout << ",n" << v_msfc[i][j];
    }
    cout << "\n";
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
  Lsv_NtkPrintMSFC(pNtk);
  return 0;

usage:
  Abc_Print(-2, "usage: lsv_print_msfc [-h]\n");
  Abc_Print(-2, "\t        prints the MSFCs in the network\n");
  Abc_Print(-2, "\t-h    : print the command usage\n");
  return 1;
}