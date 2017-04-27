//
// Created by shiyi on 2017/4/3.
//

#include "TZKW.h"

void TZKW::init(EDGE* edge, int edge_num, int node_num, int* head, int src, int des){

    this->ans_cost = this->ans_flow = 0;

    memcpy(this->edge, edge, sizeof(this->edge));
    memcpy(this->head, head, sizeof(this->head));
    memset(dis, 0, sizeof(dis));
    this->edge_num = edge_num;
    this->node_num = node_num;
    this->src = src;
    this->des = des;
}

void TZKW::work(int* video_list){
    Global* global = Global::get();

    init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
               global->netNodeNum + 1);

    for(int i=0; i<global->netNodeNum; i++){
        if(video_list[i] != 0){
            add_edge(global->netNodeNum, i, video_list[i], 0);
        }
    }

    costflow();

    if(ans_flow != global->sumFlow)
        ans_cost = INF;

    for(int i=head[src]; i!=-1; i=edge[i].next){
        if(video_list[i] != 0){
            video_list[i] = edge[i].pre_cap-edge[i].cap;
        }
    }
}

int TZKW::dfs(int now_node, int max_f)
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

void TZKW::add_edge(int u, int v, int cap, int cost)
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

bool TZKW::change()
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

void TZKW::costflow()
{
    do{
        do{
            memset(vis, 0, sizeof(vis));
        }while(dfs(src, INF));
    }while(change());
}
