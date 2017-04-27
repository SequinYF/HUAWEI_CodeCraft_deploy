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

    static ZKW zkw;
};

//个体
struct ITEM{

    bool operator<(const ITEM& t) const
    {
        return this->cost < t.cost;
    }

    void init() {
        do {
        for (int i = 0; i < ITEM::node_num; i++) {
            this->video_list[i] = rand() % 100 > 50 ? 1 : 0;
        }
        }while(!check());
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

        if(this->flow != ITEM::sum_flow)
            this->cost = INF;
    }

    void variation(int f) {
        int num = rand() % 100;
        if (num > f)
            return;

        num = rand() % ITEM::node_num;
        this->video_list[num] ^= 1;

        int pre = this->cost;
        this->assess();
        if(pre < this->cost)
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
    static CostEdge* costEdge;
    static int costNodeNum;
};

//遗传算法
struct GA
{
    GA(){
        GA_ITERATIONS = 10000;   //迭代次数
        GA_VATRIATION = 100;         //变异概率???

        if(ITEM::node_num > 500) {
            GA_ITEM_NUM = 40;
        }
        else if(ITEM::node_num > 200){
            GA_ITEM_NUM = 40;
        } else{
            GA_ITEM_NUM = 40;
        }
    }

    //种群初始化
    void init(bool *t, int value){
        group = agroup;
        new_group = bgroup;

        if(ITEM::node_num > 500) {
            memcpy(group[0].video_list, t, sizeof(bool)*NODE_MAXNUM);
        }
        else if(ITEM::node_num > 200){
            memcpy(group[0].video_list, t, sizeof(bool)*NODE_MAXNUM);
        } else{
//            group[0].init();
            memcpy(group[0].video_list, t, sizeof(bool)*NODE_MAXNUM);
        }

        group[0].cost = value;

        for(int i=1; i<GA_ITEM_NUM; i++){
            do{
                group[i].init();
                group[i].assess();
            }while(group[i].cost == INF);
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
        boy.assess();
        girl.assess();
        if(boy.cost > father.cost)
            boy = father;
        if(girl.cost > mother.cost)
            girl = mother;
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
        int len = GA_ITEM_NUM;

        int sum = 0;
        for(int i=0; i<len; i++){
            sum += group[len-1].cost+1-group[i].cost;
        }

        new_group[0] = group[0];
        for(int i=1; i<len; i+=2){
            int fa = getId(sum, len);
            int mo;
            do{
                mo = getId(sum, len);
            }while(mo == fa);

            cross_item(new_group[i], new_group[i+1], group[fa], group[mo]);
        }
    }

    //变异
    void variation(){
        for (int i = 1; i < GA_ITEM_NUM; ++i) {
            group[i].variation(10);
        }
    }

    void sort(){
        std::sort(group, group+GA_ITEM_NUM);
    }

    //执行进化
    void work() {

        best_item.cost = INF;
        best_iter = 0;

        for (int i = 0; i < GA_ITERATIONS; ++i) {

//            assess();
            sort();

            if (best_item.cost > group[0].cost) {
                best_item = group[0];
                best_iter = i;
            }

            ++flag[best_item.cost];
            if(flag[best_item.cost] % 30 == 0){
//                GA_ITEM_NUM += 40;
//                printf("个体数：%d\n", GA_ITEM_NUM);
            }
//            if(flag[best_item.cost]%10 == 0){
//                for(int k=GA_ITEM_NUM/4; k<GA_ITEM_NUM; k++)
//                    group[k].init();
//            }

            if(ITEM::node_num < 200 && best_iter+150 < i)
                break;

            if (get_time() - GA::start_time > 85)
                break;


            printf("第%d代： cost = %d group[0]: %d [%d] flow = %d time = %d\n", i + 1, best_item.cost, group[0].cost, flag[best_item.cost], best_item.flow, get_time() - GA::start_time);

            cross();
            swap(group, new_group);
            variation();


        }

//        printf("第1： cost = %d  flow = %d\n", group[0].cost, group[0].flow);
//        printf("第2： cost = %d  flow = %d\n", group[1].cost, group[1].flow);
//        printf("第3： cost = %d  flow = %d\n", group[2].cost, group[2].flow);
    }

    int flag[1000000];
    ITEM *group, *new_group;
    ITEM agroup[1000];
    ITEM bgroup[1000];
    ITEM best_item;
    int best_iter;
    int GA_ITERATIONS;
    int GA_ITEM_NUM;
    int GA_VATRIATION;
    static int start_time;
};

struct particle {//定义一个粒子
    double x[NODE_MAXNUM];//当前位置矢量
    double bestx[NODE_MAXNUM];//历史最优位置
    double f;//当前适应度
    double bestf;//历史最优适应度
    double v[NODE_MAXNUM];
    double bestv[NODE_MAXNUM];
    void assess(){
        bool v[NODE_MAXNUM];
        for(int i=0; i<ITEM::node_num; i++)
        {
            v[i] = x[i]>=0?1:0;
            double t = x[i];
            if(t < 0)
                t = t+0.99999;
            v[i] = ((int)t)%2;
        }

        ZKW::zkw.init(ITEM::edge, ITEM::edge_num, ITEM::node_num+2, ITEM::head, ITEM::node_num, ITEM::node_num+1, v);
        ZKW::zkw.costflow();

        int num = 0;
        for(int i=0; i<ITEM::node_num; i++)
            if(v[i])
                ++num;
        this->f = ZKW::zkw.ans_cost + num*ITEM::video_cost;

        if(ZKW::zkw.ans_flow != ITEM::sum_flow)
            this->f = INF;
    }
};

#define randf ((rand()%10000+rand()%10000*10000)/100000000.0) //产生0-1随机浮点数

struct PSO{
    const int NUM=80;//粒子数
    const double c1=9;//参数
    const double c2=9;//参数

    double xmin=-10.0;//位置下限
    double xmax=10.0;//位置上限
    double vmin = -4.0;
    double vmax = 4.0;
    double gbestx[NODE_MAXNUM];//全局最优位置
    double gbestv[NODE_MAXNUM];
    double gbestf;//全局最优适应度
    double w = 0.8;
    particle group[80];//定义粒子群

    void init(bool* pre, double f){
        for(int i=0; i<ITEM::node_num; i++) {
            gbestx[i] = randf * (xmax - xmin) + xmin;
            gbestv[i] = randf * (vmax - vmin) + vmin;;
        }
        gbestf = f;

        for(int i=0; i<NUM; i++){
            particle* p1=&group[i];
            for(int j=0; j<ITEM::node_num; j++) {
                p1->x[j] = randf * (xmax - xmin) + xmin;
                p1->v[j] = randf * (vmax - vmin) + vmin;
            }
            p1->assess();
            p1->bestf=100000000000000.0;
        }
    }

    void work()
    {
        for(int t=0; t<5000; t++) {
            for(int i=0; i<NUM; i++) {
                particle* p1=&group[i];
                for(int j=0; j<ITEM::node_num; j++)//进化方程
                {
                    p1->v[j] = (0.729*p1->v[j] + c1 * randf * (p1->bestx[j] - p1->x[j])
                                + c2 * randf * (gbestx[j] - p1->x[j]));
                    p1->x[j] += p1->v[j];
                    p1->x[j] = max(p1->x[j], xmin);
                    p1->x[j] = min(p1->x[j], xmax);
                }
                p1->assess();
                if(p1->f<p1->bestf) {//改变历史最优
                    for(int j=0;j<ITEM::node_num;j++) {
                        p1->bestx[j] = p1->x[j];
                        p1->bestv[j] = p1->v[j];
                    }
                    p1->bestf=p1->f;
                }
                if(p1->f<gbestf) {//改变全局最优
                    for (int j = 0; j < ITEM::node_num; j++) {
                        gbestx[j] = p1->x[j];
                        gbestv[j] = p1->v[j];
                    }
                    gbestf = p1->f;
                    for (int j = 0; j < ITEM::node_num; j++)//把当前全局最优的粒子随机放到另一位置
                    {
                        p1->x[j] = randf * (xmax - xmin) + xmin;
                        p1->v[j] = randf * (vmax - vmin) + vmin;
                    }
                }
            }
            printf("%d=== %lf    time:%d\n", t, gbestf, get_time()-GA::start_time);
        }
    }
};


#endif
