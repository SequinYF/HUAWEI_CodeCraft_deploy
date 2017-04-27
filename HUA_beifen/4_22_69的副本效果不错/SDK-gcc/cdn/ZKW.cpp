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

void ZKW::dfs_spfa(int now_node)
{
    if(dfs_flag != -1)
        return;
    vis[now_node] = true;

    for(int i=head[now_node]; i!=-1; i=edge[i].next){
        if(dfs_flag != -1)
            return;
        int v = edge[i].v;

        if(dis[now_node]+edge[i].cost < dis[v]){
            dis[v] = dis[now_node]+edge[i].cost;
            pre_node[v] = now_node;
            pre_edge[v] = i;
            if(vis[v])
            {
                dfs_flag = v;
                return;
            } else{
                dfs_spfa(v);
            }
        }
    }

    vis[now_node] = false;
}

void ZKW::clear_circle(int u)
{
    memset(dis, 0, sizeof(dis));
    memset(vis, 0, sizeof(vis));
    dfs_flag = -1;
    dfs_spfa(u);

    while(dfs_flag != -1){
        memset(vis, 0, sizeof(vis));

        int i = dfs_flag;
        while(!vis[i]){
            vis[i] = true;
            i = pre_node[i];
        }
        int rel = i;

        int min_flow = INF;
        int sum_cost = 0;
        int u = rel;
        do{
            int e_id = pre_edge[u];
            sum_cost += edge[e_id].cost;
            int flow = edge[e_id].cap;
            min_flow = min(min_flow, flow);

            u = pre_node[u];
        }while(u != rel);
        ans_cost += min_flow*sum_cost;

        u = rel;
        do{
            int e_id = pre_edge[u];
            edge[e_id].cap -= min_flow;
            edge[e_id^1].cap += min_flow;

            u = pre_node[u];
        }while(u != rel);


        memset(dis, 0, sizeof(dis));
        memset(vis, 0, sizeof(vis));
        dfs_flag = -1;
        dfs_spfa(u);
    }
}

void ZKW::t_work(int *video_list)
{
    Global* global = Global::get();

    for(int i=0; i<node_num-2; i++) {
        EDGE &realE = edge[global->hashBEdge[i]];
        EDGE &fakeE = edge[global->hashAEdge[i]];

        int fakeFlow = fakeE.pre_cap - fakeE.cap;
        int realFlow = realE.pre_cap - realE.cap;

        int cap = 0;
        if(video_list[i] != 0)
            global->videoCost[video_list[i] - 1].cap;
        realE.pre_cap = cap;
        fakeE.pre_cap = global->capSum[i] - cap;

        int p = std::min(fakeFlow, realE.pre_cap - realFlow);
        fakeE.cap += p;
        realE.cap -= p;

        ans_cost -= p*MAX_COST;
    }


    for(int i=0; i<node_num; i++){
        clear_circle(i);
    }


    for(int i=0; i<node_num-2; i++){

        if(video_list[i] != 0){
            EDGE& fakeE = global->edge[global->hashAEdge[i]];
            EDGE& realE = global->edge[global->hashBEdge[i]];
            int fakeFlow = fakeE.pre_cap - fakeE.cap;
            int realFlow = realE.pre_cap - realE.cap;

            int flow = realFlow;
            video_list[i] = global->hashCapForType[flow]+1;
            ans_cost += (realE.cost - fakeE.cost) * fakeFlow;

        }else {
            EDGE &realE = global->edge[global->hashBEdge[i]];

            int fakeFlow = realE.pre_cap - realE.cap;
            if (fakeFlow != 0) {
                video_list[i] = global->hashCapForType[fakeFlow] + 1;
                ans_cost -= realE.cost * fakeFlow;
            }
        }
    }
}

void ZKW::work(int* video_list)
{
    Global* global = Global::get();
    prepare(video_list);
    costflow();

    video_num = 0;

    if(ans_flow != global->sumFlow) {
        ans_cost = INF;
//        printf("-=-=-=\n");
//        getchar();
        return;
    }

    for(int i=0; i<node_num-2; i++){

        if(video_list[i] != 0){
            EDGE& fakeE = global->edge[global->hashAEdge[i]];
            EDGE& realE = global->edge[global->hashBEdge[i]];
            int fakeFlow = fakeE.pre_cap - fakeE.cap;
            int realFlow = realE.pre_cap - realE.cap;

            int flow = realFlow;
            video_list[i] = global->hashCapForType[flow]+1;
            ans_cost += (realE.cost - fakeE.cost) * fakeFlow;

        }else {
            EDGE &realE = global->edge[global->hashAEdge[i]];

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
        EDGE& fakeEdge = edge[global->hashAEdge[i]];

        int cap = 0;
        if(video_list[i] != 0)
            cap = global->videoCost[video_list[i]-1].cap;

        realEdge.pre_cap = realEdge.cap = cap;
        realEdge.cost = 0;

        fakeEdge.pre_cap = fakeEdge.cap = global->capSum[i]-cap;
        fakeEdge.cost = MAX_COST;
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