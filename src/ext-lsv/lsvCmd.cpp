#include "base/abc/abc.h"
#include "base/main/main.h"
#include "base/main/mainInt.h"
#include <set>
#include <vector>
#include <list>
#include <queue>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <map>
#include <typeinfo>

using namespace std;
static int Lsv_CommandPrintNodes(Abc_Frame_t* pAbc, int argc, char** argv);
static int Lsv_CommandMFFC(Abc_Frame_t* pAbc, int argc, char** argv);

void init(Abc_Frame_t* pAbc) {
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_nodes", Lsv_CommandPrintNodes, 0);
  Cmd_CommandAdd(pAbc, "LSV", "lsv_print_msfc", Lsv_CommandMFFC, 0);
}

void destroy(Abc_Frame_t* pAbc) {}

Abc_FrameInitializer_t frame_initializer = {init, destroy};

struct PackageRegistrationManager {
  PackageRegistrationManager() { Abc_FrameAddInitializer(&frame_initializer); }
} lsvPackageRegistrationManager;


class Graph{
private:
    int num_vertex;
    std::vector< std::list<int> > AdjList;
    int *color,             // 0:white, 1:gray, 2:black
        *predecessor,
        *distance,          // for BFS()
        *discover,          // for DFS()
        *finish;
public:
    Graph():num_vertex(0){};
    Graph(int N):num_vertex(N){
        // initialize Adj List
        AdjList.resize(num_vertex);
    };
    void AddEdgeList(int from, int to);
        
    void DFS(int Start);
    void DFSVisit(int vertex, int &time);

    void CCDFS(int vertex,int num11,std::map < int,string > nodemap);                // 利用DFS 
                // 利用BFS, 兩者邏輯完全相同
    void SetCollapsing(int vertex);
                   // 印出predecessor, 供驗証用, 非必要
};
void Graph::AddEdgeList(int from, int to){
    AdjList[from].push_back(to);
}
void Graph::DFS(int Start){
    color = new int[num_vertex];           // 配置記憶體位置
    discover = new int[num_vertex];
    finish = new int[num_vertex];
    predecessor = new int[num_vertex];

    int time = 0;                          // 初始化, 如圖三(b)
    for (int i = 0; i < num_vertex; i++) { 
        color[i] = 0;
        discover[i] = 0;
        finish[i] = 0;
        predecessor[i] = -1;
    }

    int i = Start;
    for (int j = 0; j < num_vertex; j++) { // 檢查所有Graph中的vertex都要被搜尋到
        if (color[i] == 0) {               // 若vertex不是白色, 則進行以該vertex作為起點之搜尋
            DFSVisit(i, time);
        }
        i = j;                             // j會把AdjList完整走過一遍, 確保所有vertex都被搜尋過
    }
}

void Graph::DFSVisit(int vertex, int &time){   // 一旦有vertex被發現而且是白色, 便進入DFSVisit()
    color[vertex] = 1;                         // 把vertex塗成灰色
    discover[vertex] = ++time;                 // 更新vertex的discover時間
    for (std::list<int>::iterator itr = AdjList[vertex].begin();   // for loop參數太長
         itr != AdjList[vertex].end(); itr++) {                    // 分成兩段
        if (color[*itr] == 0) {                // 若搜尋到白色的vertex
            predecessor[*itr] = vertex;        // 更新其predecessor
            DFSVisit(*itr, time);              // 立刻以其作為新的搜尋起點, 進入新的DFSVisit()
        }
    }
    color[vertex] = 2;                         // 當vertex已經搜尋過所有與之相連的vertex後, 將其塗黑
    finish[vertex] = ++time;                   // 並更新finish時間
}
void Graph::SetCollapsing(int current){
    int root;  // root
    for (root = current; predecessor[root] >= 0; root = predecessor[root]);

    while (current != root) {
        int parent = predecessor[current];
        predecessor[current] = root;
        current = parent;
    }
}

void Graph::CCDFS(int vertex = 0,int num11 = 0, std::map <int,string> nodemap = {}){

    DFS(vertex);      // 
    
    for (int i = 0; i< num_vertex; i++){
        SetCollapsing(i);
    }
    

    int num_cc = 0;
    for (int i = 0; i < num_vertex; i++) {
        if (predecessor[i] < 0 && i >= num11) {
            std::cout << "MSFC " << num_cc++ << ": " << nodemap[i] ;
            for (int j = 0; j < num_vertex; j++) {
                if (predecessor[j] == i) {
                    std::cout << ","<<        nodemap[j] ;
                }
            }
            std::cout << std::endl;
        }
    }
}


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
bool cmp1(std::pair<int,int>a,std::pair<int,int>b)
{
    return a.first < b.first;
}
int Lsv_CommandMFFC(Abc_Frame_t* pAbc, int argc, char** argv) {
  Abc_Ntk_t* pNtk = Abc_FrameReadNtk(pAbc);
  int c;
  Extra_UtilGetoptReset();
  while ((c = Extra_UtilGetopt(argc, argv, "h")) != EOF) {
    
  }
  if (!pNtk) {
    Abc_Print(-1, "Empty network.\n");
    return 1;
  }
  //main
  Abc_Obj_t* pObj;
  int i;
  //static int 14 = Abc_NtkNodeNum(pNtk);
  std::vector <int> if_two_fanout;
  std::vector <std::pair<int,int> > big_set;
  std::vector <int> boom;
  std::map<int, std::string> nodemap;
  int num11 = Abc_NtkObjNum(pNtk) - Abc_NtkNodeNum(pNtk) ;
  //std::cout<<num11;
  //Abc_NtkIncrementTravId(pNtk);
  int node_num = Abc_NtkObjNum(pNtk);
  Abc_NtkForEachNode(pNtk, pObj, i) {
    //printf("Object Id = %d, name = %s\n", Abc_ObjId(pObj), Abc_ObjName(pObj));
    nodemap.insert(std::pair<int, std::string>(Abc_ObjId(pObj), Abc_ObjName(pObj)));
    std::pair<int,int> aig_set;
    aig_set.first=(Abc_ObjId(pObj));
    Abc_Obj_t* pFanin;
    int j;
    Abc_ObjForEachFanin(pObj, pFanin, j) {
      //printf("  Fanin-%d: Id = %d, name = %s\n", j, Abc_ObjId(pFanin),
      //       Abc_ObjName(pFanin));
      if (!Abc_ObjIsPi(pFanin)){

        {
          
          if (std::find(if_two_fanout.begin(), if_two_fanout.end(),Abc_ObjId(pFanin))!=if_two_fanout.end())
          {
            //std::cout<<Abc_ObjId(pFanin)<<std::endl;
            //boom.push_back(Abc_ObjId(pFanin));
            for (int i  = 0; i< big_set.size();i++){
              if(big_set[i].second == Abc_ObjId(pFanin)) big_set.erase(big_set.begin()+i);
            }
          }
          
          else
          {
            aig_set.second = (Abc_ObjId(pFanin));
            big_set.push_back(aig_set);
            if_two_fanout.push_back(Abc_ObjId(pFanin));
          }
          
          
          
        }
      }
    }
    //std::vector<std::set<int> >::iterator it_i_2;
    //std::set <int>::iterator it;
    //for (it = aig_set.begin(); it != aig_set.end(); ++it) {
    //  for(it_i_2=big_set.begin(); it_i_2!=big_set.end(); ++it_i_2)
    //  {
    //    if(*it_i.count(*it) ){
    //      std::set <int> concat_set;
    //      std::set_union(*it_i.begin(), *it_i.end(),
    //            *it.begin(), *it.end(),
    //            std::inserter(concat_set, concat_set.begin()));
    //      
    //    }
    //  }
    //  
    //}
    
    
  }
  //graph start
  
  Graph g3(node_num);


  
//   for(int lp = 0; lp<big_set.size(); lp++){
//     set<int>::iterator it1;
//     for (it1 = big_set[lp].begin(); it1 != big_set[lp].end(); it1 ++)
// {
//     cout << *it1 <<" ";
// }
// std::cout<<std::endl;
//   }
    
  
  //boom.clear();
  if_two_fanout.clear();
  std::vector<std::pair<int, int> > vec;
  for( int op = 0; op<big_set.size(); op++){
    //if (big_set[op].size() >1) {
      //std::cout<<*(big_set[op].begin())<<" "<<*(++(big_set[op].begin()))<<std::endl;
      vec.push_back(std::make_pair(big_set[op].first,big_set[op].second));
      vec.push_back(std::make_pair(big_set[op].second,big_set[op].first));
      
    //}
    //if (big_set[op].size() == 3){
    //  g3.AddEdgeList(*(big_set[op].begin())-num11, *(++(big_set[op].begin()))-num11);
    //  g3.AddEdgeList(*(++(big_set[op].begin()))-num11, *(--(big_set[op].end()))-num11);
    //  //std::cout<<*(big_set[op].begin())<<" "<<*(++(big_set[op].begin()))<<std::endl;
    //  //std::cout<<*(++(big_set[op].begin()))<<" "<<*(--(big_set[op].end()))<<std::endl;
    //} 
    //std::cout<<"Next set"<<std::endl;
        
       // if(itr!=(*it_i).end() && p < (*it_i).size()){    
       //   //g3.AddEdgeList(*itr++, *itr); 
       //   itr--;
       // }
  }
  
  sort(vec.begin(), vec.end(), cmp1);
  for (int re = 0; re<vec.size();re++){
    g3.AddEdgeList(vec[re].first,vec[re].second);
    //std::cout<<vec[re].first<< " "<<vec[re].second<<std::endl;
  }
  big_set.clear();
  //big_set.shrink_to_fit();   
  
  //std::cout<<node_num;
  g3.CCDFS(0,num11,nodemap);
  //std::cout<<node_num<<"  "<<num11<<" ";
  return 0;


}