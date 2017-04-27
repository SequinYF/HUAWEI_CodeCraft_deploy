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

    for(int i=0; i<node_num-2; i++){

        if(video_list[i] != 0){
            EDGE& fakeE = global->edge[global->hashAEdge[i]];
            EDGE& realE = global->edge[global->hashBEdge[i]];
            int fakeFlow = fakeE.pre_cap - fakeE.cap;
            int realFlow = realE.pre_cap - realE.cap;

            int flow = fakeFlow + realFlow;
            if(flow != 0) {
                int vt = global->hashCapForType[flow];
                if(vt == -1)
                {
                    ans_cost = INF;
                    return;
                }
                video_list[i] = vt+1;
                ans_cost += (realE.cost - fakeE.cost) * fakeFlow;
            }
            else
                video_list[i] = 0;
        }else{
            EDGE& fakeE = global->edge[global->hashAEdge[i]];

            int fakeFlow = fakeE.pre_cap - fakeE.cap;
            if(fakeFlow != 0){
                int vt = global->hashCapForType[fakeFlow];
                if(vt == -1)
                {
                    ans_cost = INF;
                    return;
                }
                video_list[i] = vt+1;
                ans_cost -= fakeE.cost * fakeFlow;
            }
        }
    }
}

void ZKW::prepare(int* video_list)
{
    Global* global = Global::get();

    memset(dis, 0, sizeof(dis));
    ans_cost = ans_flow = 0;

    for(int i=0; i<edge_num; i+=2)
    {
        edge[i].cap = edge[i].pre_cap;
        edge[i^1].cap = 0;
    }

    for(int i=0; i<node_num-2; i++)
    {
        int eid = global->hashBEdge[i];

        if(video_list[i] != 0){
            edge[eid].pre_cap = edge[eid].cap = global->videoCost[video_list[i]-1].cap;
            edge[eid].cost = 0;
        }
        else{
            edge[eid].pre_cap = edge[eid].cap = 0;
            edge[eid].cost = MAX_COST;
        }
    }
}

void ZKW::clear()
{
    int rel = spfa();

    while(rel != -1)
    {
        memset(vis, 0, sizeof(vis));
//        vis[rel] = true;
//        puts("-=-");
        int i = rel;
        while(!vis[i]){
//            printf("%d ", i);
            vis[i] = true;
            i=pre_node[i];
        }
        rel = i;
//        printf("\n-=-\n");

//        printf("rel=%d\n", rel);

        int min_flow = INF;
        int sum_cost = 0;
        int u = rel;
        do{
            int e_id = pre_edge[u];
            sum_cost += edge[e_id].cost;
            int flow = edge[e_id].cap;
            min_flow = min(min_flow, flow);
//            printf("%d== %d %d       %d    == %d %d\n",u, min_flow, edge[e_id].cost, flow, edge[e_id].pre_cap,edge[e_id].cap);

            u = pre_node[u];
        }while(u != rel);
//        printf("%d %d\n", min_flow, sum_cost);
        ans_cost += min_flow*sum_cost;

        u = rel;
        do{
            int e_id = pre_edge[u];
            edge[e_id].cap -= min_flow;
            edge[e_id^1].cap += min_flow;

            u = pre_node[u];
        }while(u != rel);

        rel = spfa();
    }

}

int ZKW::spfa()
{
    queue<int> q;
    memset(in,0,sizeof(in));
    memset(vis,0,sizeof(vis));
    memset(pre_node,-1,sizeof(pre_node));
    memset(pre_edge,-1,sizeof(pre_edge));

    for(int i=0; i<node_num; i++)
        dis[i] = INF;
    dis[src] = 0;
    q.push(src);
    vis[src] = 1;
    while(!q.empty())
    {
        int u = q.front();
        q.pop();
        vis[u] = 0;

        for(int i=head[u]; i!=-1; i=edge[i].next)
        {
            int v = edge[i].v;
            if(edge[i].cap > 0 && dis[v] > dis[u]+edge[i].cost)
            {
                dis[v] = dis[u] + edge[i].cost;
                pre_node[v] = u;
                pre_edge[v] = i;
                if(!vis[v])
                {
                    vis[v] = 1;
                    q.push(v);
                    in[v]++;
                    if(in[v] > node_num)       //可改为度的大小
                        return v;
                }
            }
        }
    }
    return -1;
}


inline int ZKW::dfs(int now_node, int max_f)
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
    do{
        do{
            memset(vis, 0, sizeof(vis));
        }while(dfs(src, INF));
    }while(change());
}