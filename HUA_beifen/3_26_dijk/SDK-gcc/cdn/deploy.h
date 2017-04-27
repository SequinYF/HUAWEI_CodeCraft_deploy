#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <queue>
#include <sys/timeb.h>

#define INF                 999999
#define DATA_INF            100009
#define NODE_MAXNUM         1009            //网络节点数量最大值
#define EDGE_MAXNUM         50009           //最大链路数量
#define COST_MAXNUM         509             //消费节点数量最大值

using namespace std;

struct CostEdge
{
    int cost_id;        //消费节点id
    int node_id;        //供应节点id
    int flow_need;      //流量需求
};

struct EDGE
{
    int cost, cap, v;
    int next;
    int pre_cap;
};

void deploy_server(char * graph[MAX_EDGE_NUM], int edge_num, char * filename);

void parse_file(char* topo[MAX_EDGE_NUM], int line_num);

void add_edge(int u, int v, int cap, int cost);

int get_time();

void out_badResult();

void out_goodResult(bool* video_list);

void init_ITEM();

void solve();

struct HeapNode{  //prority_queue 中的优先级
    int u,dist;   //dist: u点到起点的最短路 ,u: 有向边的终点
    HeapNode(int u,int d):u(u),dist(d){}
    bool operator < (const HeapNode& h) const {
        return dist>h.dist;
    }
};
struct Dijkstra{ //打包在Dijkstra中
    struct Edge{
        int from,to,weight;
        Edge(int from,int to,int weight):from(from),to(to),weight(weight){}
    };
    struct HeapNode{  //prority_queue 中的优先级
        int u,dist;   //dist: u点到起点的最短路 ,u: 有向边的终点
        HeapNode(int u,int d):u(u),dist(d){}
        bool operator < (const HeapNode& h) const {
            return dist>h.dist;
        }
    };

    int n,m;
    vector<Edge> edges;
    vector<int> G[NODE_MAXNUM];
    bool done[NODE_MAXNUM];
    int dist[NODE_MAXNUM];
    int p[NODE_MAXNUM];
    void init(int n){
        this->n = n;
        for(int i=0;i<n;i++) G[i].clear();
        edges.clear();
    }
    void AddEdge(int from,int to,int weight){
        edges.push_back(Edge(from,to,weight));
        m=edges.size();
        G[from].push_back(m-1);  //保存from出发的边
    }
    void dijkstra(int s)
    {
        priority_queue<HeapNode> Q;
        memset(dist,0x3f,sizeof(dist));
        memset(done,false,sizeof(done));

        dist[s]=0;
        Q.push(HeapNode(s,0));
        while(!Q.empty())
        {
            int u=Q.top().u; Q.pop();
            if(done[u]) continue;
            done[u]=true;
            for(int i=0;i<G[u].size();i++)
            {
                Edge& e=edges[G[u][i]];
                int v=e.to ,w=e.weight;
                if(dist[v]>dist[u]+w)
                {
                    dist[v]=dist[u]+w;
                    p[v]=G[u][i];          //记录到各点的最短路径
                    Q.push(HeapNode(v,dist[v]));
                }
            }
        }
    }

    vector<int> getLoad(int s, int t)
    {
        vector<int> ans;

        if(this->dist[t] == 0x3f3f3f3f)
            return ans;

        for(int i=t;i!=s;i=this->edges[this->p[i]].from)
            ans.push_back(i);

        return ans;
    }

    static Dijkstra dijk;
};

struct ZKW{
    void init(EDGE* edge, int edge_num, int node_num, int* head, int src, int des, bool* video_list){

        this->ans_cost = this->ans_flow = 0;

        memcpy(ZKW::edge, edge, sizeof(ZKW::edge));
        memcpy(ZKW::head, head, sizeof(ZKW::head));
        memset(dis, 0, sizeof(dis));
        this->edge_num = edge_num;
        this->node_num = node_num;
        this->src = src;
        this->des = des;

        //设置超级源
        for(int i=0; i<ZKW::node_num-2; ++i)
        {
            if(video_list[i]) {
                add_edge(ZKW::src, i, INF, 0);
            }
        }
    }

    int dfs(int now_node, int max_f)
    {
        if(now_node == ZKW::des || max_f == 0) {
            ans_flow += max_f;
            return max_f;
        }
        int used = 0;
        vis[now_node] = true;
        for(int i=head[now_node]; i != -1; i=edge[i].next){
            if(edge[i].cap && dis[edge[i].v]+edge[i].cost==dis[now_node] && !vis[edge[i].v]){

                int f = dfs(edge[i].v, min(edge[i].cap, max_f-used));

                ans_cost += f*edge[i].cost;
                edge[i].cap -= f;
                edge[i^1].cap += f;
                used += f;
                if(used == max_f)
                    break;
            }
        }
        return used;
    }

    void add_edge(int u, int v, int cap, int cost)
    {
        edge[edge_num].v = v;
        edge[edge_num].cap = cap;
        edge[edge_num].pre_cap = cap;
        edge[edge_num].cost = cost;
        edge[edge_num].next = head[u];
        head[u] = edge_num++;

        edge[edge_num].v = u;
        edge[edge_num].cap = 0;
        edge[edge_num].cost = -cost;
        edge[edge_num].next = head[v];
        head[v] = edge_num++;
    }

    bool change()
    {
        int delta = INF;
        for(int i=0; i<node_num; i++){
            if(vis[i])
                for(int j=head[i]; j!=-1; j=edge[j].next){
                    if(!vis[edge[j].v] && edge[j].cap)
                        delta = min(delta, dis[edge[j].v]+edge[j].cost-dis[i]);
                }
        }

        if(delta == INF)
            return false;
        for(int i=0; i<node_num; ++i) {
            if(vis[i])
                dis[i] += delta;
        }
        return true;
    }

    void costflow()
    {
        do{
            do{
                memset(vis, 0, sizeof(vis));
            }while(dfs(src, INF));
        }while(change());
    }

    void init_p(int num, int costNum)
    {

        memset(f, 0, sizeof(f));
        for(int i=0; i<costNum; i++)
        {
            int st = costEdge[i].node_id;
            printf("st=%d-----------\n", st);
            Dijkstra::dijk.dijkstra(st);

            for(int i=0; i<num; i++)
            {
                if(Dijkstra::dijk.dist[i] != 0x3f3f3f3f) {
                    printf("to %d  sum = %d\n", i, Dijkstra::dijk.dist[i]);
                    printf("%d ", st);
                    vector<int> ans = Dijkstra::dijk.getLoad(st, i);
                    for (int i = ans.size() - 1; i >= 0; i--) {
                        printf("%d ", ans[i]);
                        f[ans[i]]++;
                    }
                    printf("\n");
                }
            }
        }

        for(int i=0; i<num; i++) {
            p[i] = i;
        }
        for(int i=0; i<num; i++) {
            for (int j = i + 1; j <num; j++) {
                if (f[p[i]] < f[p[j]]) {
                    swap(p[i], p[j]);
                }
            }
        }
        fmx = 1;
        for(int i=0; i<num; i++) {
            printf("[%d:%d] ", p[i], f[p[i]]);
            fmx = max(fmx, f[p[i]]);
        }
    }

    int ans_cost;
    int ans_flow;
    int edge_num;
    int node_num;
    int src;
    int des;
    EDGE edge[EDGE_MAXNUM];
    int head[NODE_MAXNUM];   //别忘记初始化
    int dis[NODE_MAXNUM];
    bool vis[NODE_MAXNUM];

    int f[NODE_MAXNUM];
    int p[NODE_MAXNUM];
    static ZKW zkw;
    CostEdge* costEdge;
    int fmx;
    int costNodeNum;
};

//个体
struct ITEM{

    bool operator<(const ITEM& t) const
    {
        return this->cost < t.cost;
    }

    void init() {
//        do {
        int mx = ZKW::zkw.fmx*1.2;
            for (int i = 0; i < ITEM::node_num; i++) {
                double r = ZKW::zkw.f[i]*1.0/mx;

                this->video_list[i] = (rand() % 1000) >  500+r*500? 0 : 1;
            }
//        }while(!check());
        this->cost = INF;
    }

    bool check()
    {
        int flow = 0;
        for(int i=0; i<ITEM::node_num; i++)
        {
            flow += ITEM::capSum[i];
        }
        if(flow >= ITEM::sum_flow)
        {
//            printf("-=-=-sddsg\n");
//            getchar();
        }
        return flow >= ITEM::sum_flow;
    }

    void assess(){

//        if(this->cost != INF)
//            return;

        ZKW::zkw.init(ITEM::edge, ITEM::edge_num, ITEM::node_num+2, ITEM::head, ITEM::node_num, ITEM::node_num+1, this->video_list);
        ZKW::zkw.costflow();

        int num = 0;
        for(int i=0; i<node_num; i++)
            if(video_list[i])
                ++num;
        this->cost = ZKW::zkw.ans_cost + num*video_cost;
        this->flow = ZKW::zkw.ans_flow;

//        if(this->flow != ITEM::sum_flow || ZKW::zkw.ans_cost == 0)

        if(this->flow != ITEM::sum_flow)
            this->cost = INF;
    }

    void variation(int f) {
        int num = rand() % 100;
        if (num > f)
            return;

        num = rand() % ITEM::node_num;
        this->video_list[num] ^= 1;
    }

    bool video_list[NODE_MAXNUM];
    int cost;
    int flow;

    static EDGE* edge;
    static int* head;
    static int node_num;
    static int edge_num;
    static int video_cost;
    static int sum_flow;
    static int* capSum;
};

//遗传算法
struct GA
{
    GA(){
        GA_ITERATIONS = 3000;   //迭代次数
        GA_ITEM_NUM = 200;       //最大个体数
        GA_VATRIATION = 100;         //变异概率???
    }

    //种群初始化
    void init(){
        for(int i=0; i<GA_ITEM_NUM; i++){
            group[i].init();
        }
    }

    //评估
    void assess()
    {
        for(int i=0; i<GA_ITEM_NUM; i++) {
            group[i].assess();
//            ++flag[group[i].cost];
        }
    }

    void cross_item(ITEM& boy, ITEM& girl, ITEM& father, ITEM& mother){
        int left = rand()%ITEM::node_num;
        int right = rand()%ITEM::node_num;
        if(left > right)
            swap(left, right);

        for(int i=0; i<left; i++){

            boy.video_list[i] = father.video_list[i];
            girl.video_list[i] = mother.video_list[i];
        }
        for(int i=left; i<=right; i++){
            boy.video_list[i] = mother.video_list[i];
            girl.video_list[i] = father.video_list[i];
        }
        for(int i=right+1; i<ITEM::node_num; i++){
            boy.video_list[i] = father.video_list[i];
            girl.video_list[i] = mother.video_list[i];
        }
        //可以优化  提前判定
        boy.cost = girl.cost = INF;
    }

    int getId(int sum, int len){
        int num = rand()%sum;

        for(int i=0; i<GA_ITEM_NUM; i++){
            num -= group[len-1].cost+1-group[i].cost;
            if(num <= 0)
                return i;
        }
        return 0;
    }

    //交叉
    void cross(){
        int len = GA_ITEM_NUM/4;

        while(group[len].cost == INF && len > 1)
            len--;

        if(len == 1)
        {
            init();
            return;
        }

        int sum = 0;
        for(int i=0; i<len; i++){
            sum += group[len-1].cost+1-group[i].cost;
        }

        for(int i=len; i<GA_ITEM_NUM; i+=2){
            int fa = getId(sum, len);
            int mo;
            do{
                mo = getId(sum, len);
            }while(mo == fa);

            cross_item(group[i], group[i+1], group[fa], group[mo]);
        }
    }

    //变异
    void variation(){
        for (int i = 0; i < GA_ITEM_NUM; ++i) {
            group[i].variation(100);
        }
    }

    void sort(){
        std::sort(group, group+GA_ITEM_NUM);
    }

    //执行进化
    void work() {

        init();
        memset(group[0].video_list, 0, sizeof(group[0].video_list));
        for(int i=0; i<ZKW::zkw.costNodeNum; i++)
            group[0].video_list[ZKW::zkw.costEdge[i].node_id] = true;group[0].assess();
        printf("%d\n", group[0].cost);
        best_item.cost = INF;
        best_iter = 0;

        for (int i = 0; i < GA_ITERATIONS; ++i) {

            assess();
            sort();

            if (best_item.cost > group[0].cost) {
                best_item = group[0];
                best_iter = i;
            }

//            if (i > best_iter + 150) {
////                printf("提前停止\n");
//                break;
//            }

//            printf("cost %d num %d     cost %d num %d\n", group[0].cost, flag[group[0].cost], group[1].cost, flag[group[1].cost]);
//            printf("bcost %d bnum %d\n", best_item.cost, flag[best_item.cost]);

            if(group[0].cost == best_item.cost)
                flag[group[0].cost]++;

            if (flag[best_item.cost] > 20 || i>best_iter+50 || get_time() - GA::start_time > 80)
                break;

//            if(i > best_iter+100) {
//                GA_VATRIATION = 200;
//                GA_VATRIATION += 100;
//                GA_VATRIATION = min(GA_VATRIATION, 200);
//                printf("增加变异概率\n");
//                break;
//            }
            printf("第%d代： cost = %d group[0]: %d [%d] flow = %d\n", i + 1, best_item.cost, group[0].cost, flag[best_item.cost], best_item.flow);

            cross();
            variation();


        }

//        printf("第1： cost = %d  flow = %d\n", group[0].cost, group[0].flow);
//        printf("第2： cost = %d  flow = %d\n", group[1].cost, group[1].flow);
//        printf("第3： cost = %d  flow = %d\n", group[2].cost, group[2].flow);
    }

    int flag[1000000];
    ITEM group[1000];
    ITEM best_item;
    int best_iter;
    int GA_ITERATIONS;
    int GA_ITEM_NUM;
    int GA_VATRIATION;
    static int start_time;
};



#endif
