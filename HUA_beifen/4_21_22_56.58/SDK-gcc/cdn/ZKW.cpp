//
// Created by shiyi on 2017/3/31.
//

#include "ZKW.h"
#include <algorithm>
#include <queue>

using namespace std;

void ZKW::init(EDGE* edge, int edge_num, int node_num, int* head, int src, int des){

    this->edge = edge;
    this->head = head;
    this->edge_num = edge_num;
    this->node_num = node_num;
    this->src = src;
    this->des = des;
}

void ZKW::work(int* video_list)
{
    Global* global = Global::get();
    prepare(video_list);
    costflow();

    video_num = 0;

    if(ans_flow != global->sumFlow) {
        ans_cost = INF;
        return;
    }

    for(int i=0; i<node_num-2; i++){

        if(video_list[i] != 0){
//            EDGE& fakeE = global->edge[global->hashAEdge[i]];
            EDGE& realE = global->edge[global->hashBEdge[i]];
//            int fakeFlow = fakeE.pre_cap - fakeE.cap;
            int realFlow = realE.pre_cap - realE.cap;

            int flow = realFlow;
            video_list[i] = global->hashCapForType[flow]+1;
//            ans_cost += (realE.cost - fakeE.cost) * fakeFlow;

        }else {
            EDGE &realE = global->edge[global->hashBEdge[i]];

            int fakeFlow = realE.pre_cap - realE.cap;
            if (fakeFlow != 0) {
                video_list[i] = global->hashCapForType[fakeFlow] + 1;
                ans_cost -= realE.cost * fakeFlow;
            }
        }
    }
//    printf("ee %d\n", global->get_time()-global->start_time);
}

void ZKW::prepare(int* video_list)
{
    Global* global = Global::get();

    memset(dis, 0, sizeof(dis));
    ans_cost = ans_flow = 0;

    for(int i=0; i<edge_num; i++)
    {
        edge[i].cap = edge[i].pre_cap;
    }

    for(int i=0; i<node_num-2; i++)
    {
        EDGE& realEdge = edge[global->hashBEdge[i]];
//        EDGE& fakeEdge = edge[global->hashAEdge[i]];

//        printf("%d ", video_list[i]);

        if(video_list[i] != 0){
            int cap = global->videoCost[video_list[i]-1].cap;
            realEdge.pre_cap = realEdge.cap = cap;
            realEdge.cost = 0;

//            fakeEdge.pre_cap = fakeEdge.cap = global->capSum[i]-cap;
//            fakeEdge.cost = MAX_COST;
        }
        else{
            realEdge.pre_cap = realEdge.cap = 0;
            realEdge.cost = MAX_COST;

//            fakeEdge.pre_cap = fakeEdge.cap = global->capSum[i];
//            fakeEdge.cost = MAX_COST;
        }
    }
//    printf("\n");
}

int ZKW::dfs(int now_node, int max_f)
{
    if(now_node == this->des || max_f == 0) {
        ans_flow += max_f;
        return max_f;
    }
    int used = 0;
    vis[now_node] = true;
    for(int i=head[now_node]; i != -1; i=edge[i].next){
        if(edge[i].cap && dis[edge[i].v]+edge[i].cost==dis[now_node] && !vis[edge[i].v]){

            int f = dfs(edge[i].v, std::min(edge[i].cap, max_f-used));

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

bool ZKW::change()
{
    int delta = INF;
    for(int i=0; i<node_num; i++){
        if(vis[i])
            for(int j=head[i]; j!=-1; j=edge[j].next){
                if(!vis[edge[j].v] && edge[j].cap)
                    delta = std::min(delta, dis[edge[j].v]+edge[j].cost-dis[i]);
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

void ZKW::costflow()
{
//    int b = Global::get()->get_time()-Global::get()->start_time;
//    printf("b %d\n", b);
    do{
        do{
//            printf("dfs %d %d %d\n", ans_cost, ans_flow, Global::get()->get_time()-Global::get()->start_time);
            memset(vis, 0, sizeof(vis));
        }while(dfs(src, INF));
//        printf("====\n");
//        printf("change %d\n", Global::get()->get_time()-Global::get()->start_time);
    }while(change());
//    int e = Global::get()->get_time()-Global::get()->start_time;
//    printf("e %d\n", e);
//    if(e - b > 1) {
//        printf("===\n");
//        getchar();
//    }
}