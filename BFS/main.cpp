#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <queue>
#include <time.h>
using namespace std;

struct Vertex       // 顶点
{
    int name;               // 顶点名
    vector<int> keywords;   // 顶点属性集合
};

struct Node         // 结点
{
    Vertex now;             // 当前顶点
    Node *next;             // 下一结点指针
    bool isPlace;   // 当前结点是否为p类结点
    unordered_map<int, int> distanceMap;        // 当前结点到各待查关键词的距离
    int index;              // 当前结点下标（读取数据时使用，可以提高读文件效率）
    bool find_one;
    bool find_all;

    Node(Vertex v, int i)
    {
        now = v;
        next = NULL;
        index = i;          // 第几个输入的结点，下标就为几
        find_one = false;   // 一个都没找到的情况
        find_all = false;   // 都找到的情况
        isPlace = false;
    }
    // 对当前结点进行BFS查询，得到distanceMap值
    void searchOnNode(unordered_map<int, bool> find);
};

vector<Node> vertexes;          // 所有结点集合
unordered_map<int, int> vMap;   // 用来记录结点和其索引值的对应关系
vector<Node> candidateNodes;    // 能找到所有关键词的结点
vector<Node> minNodes;          // skyline查询的根结点
int Enum = 0;                   // 总边数

int v_num = 0;

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

            Vertex v;
            v.name = temp[0];

            for (int i = 1; i < temp.size(); i++)
            {
                v.keywords.push_back(temp[i]);
            }
            temp.clear();

            Node n(v, v_num);
            vertexes.push_back(n);
            vMap.insert(pair<int, int>(v.name, v_num));
            v_num++;
        }
    }
//    for(auto iter = vMap.begin(); iter != vMap.end(); iter++)
//        cout << iter->first << "[" << iter->second << "]" << endl;
//
//    for (int i = 0; i < vertexes.size(); i++)
//    {
//        cout << vertexes[i].now.name << "[" << vertexes[i].index << "] :";
//        for (int j = 0; j < vertexes[i].now.keywords.size(); ++j) {
//            cout << vertexes[i].now.keywords[j] << " ";
//        }
//        cout << endl;
//    }
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

            unordered_map<int, int>::iterator s = vMap.find(temp[0]);
            if(s != vMap.end())        // s(val, index)
            {
//                cout << vertexes[s->second].now.name << endl;
                    for (int j = 1; j < temp.size(); j++)       // 遍历寻找顶点2: temp[j]
                    {
//                        cout << "temp[j] = " << temp[j] << endl;
                        Vertex v;
                        unordered_map<int, int>::iterator e = vMap.find(temp[j]);
                        if(e != vMap.end())
                        {
                            Enum++;
                            v = vertexes[e->second].now;            // 找到顶点2
                            int index = vertexes[e->second].index;
                            Node *p = new Node(v, index);

                            if (vertexes[s->second].next == NULL)        // 若顶点1没有与其他顶点相连
                                {
                                    vertexes[s->second].next = p;
                                    p = NULL;
                                }
                            else {          // 顶点1已经与其他顶点相连，向后遍历
                                Node *t = vertexes[s->second].next;
                                while (t->next != NULL)
                                    t = t->next;
                                t->next = p;
                                p = NULL;
                            }
                        }
                    }
            }
//            else{
//                Vertex v;
//                v.name = temp[0];
//                Node t(v, v_num);
//                vertexes.push_back(t);
//                vMap.insert(pair<int, int>(v.name, v_num));
//                v_num++;
//            }
            temp.clear();
        }  //while
    } //if


//    for (int i = 0; i < vertexes.size(); i++)
//    {
//         Node *t;
//         cout << vertexes[i].now.name;
//         t = vertexes[i].next;
//         while(t)
//         {
//             cout << "->" << t->now.name;
//             t = t->next;
//         }
//        cout << "->^" << endl;
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

        unordered_map<int, int>::iterator s = vMap.find(num);
        if(s != vMap.end())        // s(val, index)
            vertexes[s->second].isPlace = true;
//        else{
//            Vertex v;
//            v.name = num;
//            Node t(v, v_num);
//            t.isPlace = true;
//            vertexes.push_back(t);
//            vMap.insert(pair<int, int>(v.name, v_num));
//            v_num++;
//        }
    }

    //    //查看是否是place
//    for (int i = 0; i < vertexes.size(); i++) {
//        cout << vertexes[i].now.name << " " << vertexes[i].isPlace << endl;
//    }
}

// BFS初始化distanceMap
void Node::searchOnNode(unordered_map<int, bool> find)
{
    int Vnum = vertexes.size();
    int *visit = new int[Vnum];         //记录每个结点是否被遍历
    for (int i = 0; i < Vnum; i++) {
        visit[i] = 0;
    }
    queue<pair<Node*, int>> Q;

    int level = 0;      //层数

    visit[this->index] = 1;
    pair<Node*, int> temp(this, level);
    Q.push(temp);
//    cout << this->now.name << endl;

    while(!Q.empty())
    {
        pair<Node*, int> here = Q.front();
        Q.pop();

//        cout << here.first->now.name << endl;

        level = here.second;

        //在当前结点的属性中查找关键词
        for (auto & iter : find)
        {
            if (iter.second == false)
            {
                int j;
                for (j = 0; j < here.first->now.keywords.size(); j++)   //遍历该结点的所有keywords
                {
                    if (here.first->now.keywords[j] == iter.first) {
                        this->find_one = true;
                        this->distanceMap.insert(unordered_map<int, int>::value_type(iter.first, level));
                        iter.second = true;
                        break;  //继续查找下一个关键词
                    }
                }
            }
        }

        bool all = true;
        for (auto iter = find.begin(); iter != find.end(); iter++)
        {
            if(iter->second == false)
                all = false;
        }
        if(all)        //已经全部找到，退出循环
        {
            this->find_all = true;
            break;
        }

        Node* t;
        for (int i = 0; i < vertexes.size(); i++) {
            if(vertexes[i].now.name == here.first->now.name)
                t = vertexes[i].next;
        }
        while(t)
        {
            if(visit[t->index] == 0)
            {
                visit[t->index] = 1;
//                cout << t->now.name << " " << t->isPlace << " " << visit[t->index] << endl;
                pair<Node*, int> in(t, level + 1);
                Q.push(in);
            }
            t = t->next;
        }
    }
    delete []visit;
}

void skylineSearch(unordered_map<int, bool> find)
{
    for (int i = 0; i < candidateNodes.size(); i++) {
//        cout << "candidate node: " << candidateNodes[i].now.name << endl;
        if(minNodes.empty())
            minNodes.push_back(candidateNodes[i]);
        else{
            bool insert = true;     //是否插入minNodes
            for(auto iter = minNodes.begin(); iter != minNodes.end();)
            {
                bool smaller = true;        //小于其中一个minNode
                bool larger = true;         //大于其中一个minNode
                bool equal = true;          //等于其中一个minNode
                for (auto f = find.begin(); f != find.end(); f++)
                {   //查看是否出现反例
                    if(candidateNodes[i].distanceMap[f->first] > iter->distanceMap[f->first])
                        smaller = false;
                    if(candidateNodes[i].distanceMap[f->first] < iter->distanceMap[f->first])
                        larger = false;
                    if(candidateNodes[i].distanceMap[f->first] != iter->distanceMap[f->first])
                        equal = false;
                }
                if(equal)
                    break;
                else if(smaller)
                    minNodes.erase(iter);
                else if(larger)
                {
                    insert = false;
                    break;
                }
                else
                    iter++;
            }
            if(insert)
                minNodes.push_back(candidateNodes[i]);
        }
    }
}

void Display() {
    cout << "----------------------- skyline -----------------------" << endl;
    cout << "skylineNum: " << minNodes.size() << endl;
    for (int i = 0; i < minNodes.size(); i++)
    {
        cout << "Root node(" << minNodes[i].now.name << ")" << endl;
        cout << "Distance to each keyword:" << endl;
        for(auto iter = minNodes[i].distanceMap.begin(); iter != minNodes[i].distanceMap.end(); iter++)
            cout << iter->first << ": " << iter->second << endl;
        cout << endl;
    }
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

int main() {
    cout << "-------------- Skyline based on BFS --------------" << endl;
    init();
//    cout << "Vnum: " << vertexes.size() << endl;
//    cout << "Enum: " << Enum << endl;

    clock_t startTime, endTime;

    unordered_map<int, bool> find;       // 待查找的关键词集合
    int n;
    cout << "请输入待查找keywords：" << endl;
    while (cin >> n)
    {
        find.insert(unordered_map<int, bool>::value_type(n, false));
        if(cin.get() == '\n')
            break;
    }

    startTime = clock();
    // 初始化每个结点的distanceMap
    for (int i = 0; i < vertexes.size(); i++) {
        if(!vertexes[i].isPlace)    //不满足根结点要求
            continue;
        vertexes[i].searchOnNode(find);

        if(vertexes[i].find_all)     //满足查到所有关键词
        {
            candidateNodes.push_back(vertexes[i]);
            //查看每个candidate node的map结果
//            unordered_map<int, int>::reverse_iterator iter;
//            cout << vertexes[i].now.name << ": ";
//            for (iter = vertexes[i].distanceMap.rbegin(); iter != vertexes[i].distanceMap.rend(); iter++)
//                cout << iter->first << " " << iter->second << endl;
        }
    }

    // 遍历每个满足要求的点，skyline查找
    skylineSearch(find);
    endTime = clock();

    Display();
    cout << "Total time: " << (double)(endTime - startTime)/CLOCKS_PER_SEC << "s" << endl;

    return 0;
}


/*
 * 测试数据1：11544759 9429730
 * 测试数据2：10561570 8964035
 */