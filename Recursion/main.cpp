#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <time.h>
using namespace std;

struct Node
{
    int val;                // 当前顶点值
    bool isPlace;           // 是否为p类结点
    unordered_map<int, int> myKeywords;     // 当前结点关键词
    vector<Node *> children;                // 所有孩子结点
    short status;           // 0 -> not visited;  1 -> visiting;  2 -> visited
    bool circle_node;       // 当前结点是否为循环结点

    Node(int _val)
    {
        val = _val;
        isPlace = false;
        status = 0;         // 初始时为not visit
        circle_node = 0;    // 初始时不是循环结点
    }
};

vector<Node> vertexes;                      // 所有结点
unordered_map<int, int> v;                  // 保存node和index的对应关系（为了提高读取速率）
int keywordNum = 0;                         // 待查关键词数目
unordered_map<int, vector<int>> disMaps;    // 保存已经生成distanceMap的结点
unordered_map<int, vector<int>> minMaps;    // 存放查询到的所有skyline
vector<int> keywords_to_find;               // 待查关键词
int Enum = 0;
int pos = 0;
int count_v = 0;

void Initialize_Node_Keywords()
{
    ifstream in("../Yago_small/node_keywords.txt");
    string str;
    if(in.is_open())
    {
        vector<int> temp;
        while(!in.eof())
        {
            getline(in, str);
            istringstream is(str);
            int num;
            char ch;
            while(is >> num)
            {
                temp.push_back(num);
                is >> ch;
            }

            Node p(temp[0]);

            for (int i = 1; i < temp.size(); i++)
            {
                p.myKeywords.insert(unordered_map<int,int>::value_type(temp[i], -1));
            }
            vertexes.push_back(p);
            v.insert(pair<int, int>(p.val, pos++));

            temp.clear();
        }
    }
}

void Initialize_Edge()
{
    ifstream in("../Yago_small/edge.txt");
    string str;
    if(in.is_open())
    {
        vector<int> temp;
        while(!in.eof())
        {
            getline(in, str);
            istringstream is(str);
            int num;
            char ch;
            while(is >> num)
            {
                temp.push_back(num);
                is >> ch;
            }

            unordered_map<int, int>::iterator s = v.find(temp[0]);
            if(s != v.end())        // s(val, index)
            {
//                cout << s->first << endl;
                for (int j = 1; j < temp.size(); j++)
                {
                    unordered_map<int, int>::iterator e = v.find(temp[j]);
                    if(e != v.end())
                    {
                        Enum++;
                        vertexes[s->second].children.push_back(&vertexes[e->second]);
                    }
                }
            }
            temp.clear();
        }  //while
    } //if

//    for (auto & vertex : vertexes)
//    {
//        cout << vertex.val << ": ";
//        for (auto & j : vertex.children)
//        {
//            cout << j->val << " ";
//        }
//        cout << endl;
//    }
}

void Initialize_Place()
{
    ifstream inFile;
    inFile.open("../Yago_small/placeid2coordYagoVB.txt",ios::in);
    //判断文件是否打开
    if(!inFile.is_open())
    {
        cout<<"Error opening file" << endl;
    }
    while(!inFile.eof())        //所有地点都存在place里
    {
        int num;
        string sTmp;
        inFile >> num;
        getline(inFile, sTmp);//略过第一行

        unordered_map<int, int>::iterator s = v.find(num);
        if(s != v.end())        // s(val, index)
            vertexes[s->second].isPlace = true;
    }

    // 输出所有顶点isPlace信息
//    for(int i = 0; i < vertexes.size(); i++)
//        cout << vertexes[i].val << " " << vertexes[i].isPlace << " " << vertexes[i].visited_place << endl;
}

void init()
{
    cout << "********** Keywords **********" << endl;
    Initialize_Node_Keywords();     // 初始化顶点信息
    cout << "*********** Places ***********" << endl;
    Initialize_Place();             // 初始化地点信息
    cout << "************ Edge ************" << endl;
    Initialize_Edge();              // 初始化邻接表
}

// update index of each keyword in every node
void updateIndex() {
    for (int i = 0; i < vertexes.size(); ++i) {
        for (auto iter = vertexes[i].myKeywords.begin(); iter != vertexes[i].myKeywords.end(); iter++) {
            for (int j = 0; j < keywords_to_find.size(); j++) {
                if (iter->first == keywords_to_find[j])
                    iter->second = j;
            }
        }
    }

//    for (int i = 0; i < vertexes.size(); ++i) {
//        cout << vertexes[i].val << endl;
//        for (auto iter = vertexes[i].myKeywords.begin(); iter != vertexes[i].myKeywords.end(); iter++) {
//            cout << iter->first << ": " << iter->second << "    ";
//        }
//        cout << endl;
//    }
}

void clearMap(vector<int> &this_map)
{
    //将dismap中的值都置-1
    fill(this_map.begin(), this_map.end(), -1);
}

void combineDistanceMap(const vector<int>& inputMap, vector<int>& resultMap)
{
    for (int i = 0; i < inputMap.size(); i++) {
        if(inputMap[i] == -1)
            continue;
        else if(resultMap[i] == -1)
            resultMap[i] = inputMap[i];
        else if(inputMap[i] < resultMap[i])
            resultMap[i] = inputMap[i];
    }
}

void incrementDistance(vector<int>& distanceMap)
{
    for (int i = 0; i < distanceMap.size(); i++) {
        if(distanceMap.at(i) != -1)
            distanceMap.at(i) += 1;
    }
}

bool isMinMap(const vector<int>& inputMap)
{
    if(minMaps.empty())
        return true;
    else
    {
        for (auto iter = minMaps.begin(); iter != minMaps.end();)   //遍历每个candidate Map
        {
            bool smaller = true;           //比其中一个Map的所有Keywords都小
            bool larger = true;
            bool equal = true;
            for (int j = 0; j < keywordNum; j++)    //遍历每个keyword
            {
                if(inputMap[j] > iter->second[j])
                    smaller = false;
                if(inputMap[j] < iter->second[j])
                    larger = false;
                if(inputMap[j] != iter->second[j])
                    equal = false;
            }
            if(equal)
                return true;
            else if(smaller)            //到所有关键词的距离小于当前Map
                minMaps.erase(iter++);      //删除当前Map&Node，加入新结点到minNode
            else if(larger)
                return false;
            else
                iter++;

        } //for each minNode
        return true;
    }
}


void Display()
{
    cout << "----------------------- skyline -----------------------" << endl;
    cout << "skylineNum: " << minMaps.size() << endl;
    for (auto iter = minMaps.begin(); iter != minMaps.end(); iter++) {
        cout << "Root node(" << iter->first << ")" << endl;
        cout << "Distance to each keyword:" << endl;
        for (int i = 0; i < iter->second.size(); i++) {
            cout << keywords_to_find[i] << ": " << iter->second[i] << endl;
        }
        cout << endl;
    }
}

bool skylineSearch(Node *node, vector<int> &distanceMap)
{
//    cout << "node" << node->val << endl;
    //distanceMap is always {-1,-1,-1...} when passed in
    clearMap(distanceMap);

    if(node == NULL)
        return false;

    bool child_visiting = false;

    unordered_map<int, vector<int>>::iterator iter;
    iter = disMaps.find(node->val);

    if(iter != disMaps.end())       // 该node distanceMap已经生成
    {
        distanceMap = iter->second;
        incrementDistance(distanceMap);
        return false;
    }

    else {
        if (node->status == 2)       // 循环
        {
            node->circle_node = 1;      // 是循环结点，直接返回true
            return true;
        }
        node->status = 2;            // visited
//        count_v++;
        vector<int> tmpDistanceMap(keywordNum);

        //遍历所有children
        for(Node *p : node->children)
        {
            bool res = skylineSearch(p, tmpDistanceMap);
            if(res)
                child_visiting = res;
            combineDistanceMap(tmpDistanceMap, distanceMap);
        }

        // mykeyWord<(keyword)int, (keyword index)int>
        for (auto const& x : node->myKeywords)
        {
            if(x.second >= 0)
            {
//            cout << "index: " << x.second << endl;
                distanceMap[x.second] = 0;
            }
        }

        if(!child_visiting || node->circle_node)
            disMaps.insert(unordered_map<int, vector<int>>::value_type(node->val, distanceMap));

    }
//    cout << "--------" << node->val << "--------" << endl;
//    for (int i = 0; i < distanceMap.size(); ++i) {
//        cout << keywords_to_find[i] << ": " << distanceMap[i] << endl;
//    }

    if((find(distanceMap.begin(), distanceMap.end(), -1) == distanceMap.end())
       &&(node->isPlace))
    {
//        cout << "candidate node: " << node->val << endl;
        //found all key word, it is a candidate node
        if (isMinMap(distanceMap))
        {
            minMaps.insert(unordered_map<int, vector<int>>::value_type(node->val, distanceMap));
//            Display();
        }
    }

    // counting
    incrementDistance(distanceMap);

    if(child_visiting && !node->circle_node)
    {
        node->status = 1;       // 有一个children为visiting，就改为visiting（循环结点除外）
        return true;
    }

    return false;
}

int main() {
    cout << "-------------- Skyline based on Recursion --------------" << endl;
    init();
//    cout << "Vnum: " << pos << endl;
//    cout << "Enum: " << Enum << endl;

    clock_t startTime, endTime;

    int num;
    cout << "请输入待查找keywords：" << endl;
    while(cin >> num)
    {
        keywords_to_find.push_back(num);
        if(cin.get() == '\n')
            break;
    }

    updateIndex();

    keywordNum = keywords_to_find.size();

    startTime = clock();
    for(int i = 0; i < vertexes.size(); i++)
    {
        if(vertexes[i].status == 0)
        {
            vector<int> DistanceMap(keywordNum);
            skylineSearch(&vertexes[i], DistanceMap);
        }
    }
//    for(auto iter = disMaps.begin(); iter != disMaps.end(); iter++) {
//        cout << "ALREADY" << iter->first << endl;
//        for (int i = 0; i < iter->second.size(); i++) {
//            cout << iter->second[i] << " ";
//        }
//        cout << endl;
//    }

    for(int i = 0; i < vertexes.size(); i++)
    {
        if(vertexes[i].status == 1)     //所有visiting的结点
        {
            vector<int> DistanceMap(keywordNum);
            skylineSearch(&vertexes[i], DistanceMap);
        }
    }

    endTime = clock();

    Display();

//    cout << "conutV: " << count_v << endl;
//    cout << "conutMap: " << disMaps.size() << endl;

    cout << "Total time: " << (double)(endTime - startTime)/CLOCKS_PER_SEC << "s" << endl;

    return 0;
}


/*
 * 测试数据1：11544759 9429730
 * 测试数据2：10561570 8964035
 */