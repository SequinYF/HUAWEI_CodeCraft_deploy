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

void ZKW::work(bool* video_list)
{
    prepare(video_list);
    costflow();

    for(int i=head[src]; i!=-1; i=edge[i].next) {
        int v = edge[i].v;

        if (video_list[v]) {
            ans_cost += Global::get()->videoCost;
        } else {
            int all_cap = edge[i].pre_cap;
            int now_cap = edge[i].cap;
            if (all_cap != now_cap && !video_list[v]) {
                ans_cost -= (all_cap - now_cap) * MAX_COST;
                ans_flow -= all_cap-now_cap;
            }
        }
    }
}

void ZKW::prepare(bool* video_list)
{
    memset(dis, 0, sizeof(dis));
    ans_cost = ans_flow = 0;

    for(int i=0; i<edge_num; i+=2)
    {
        edge[i].cap = edge[i].pre_cap;
        edge[i^1].cap = 0;
    }

    for(int i=head[src]; i!=-1; i=edge[i].next)
    {
        int all_cap = edge[i].pre_cap;
        int now_cap = edge[i].cap;
        int v = edge[i].v;


        if(edge[i].cost == 0 && !video_list[v])      //之前存在现在不存在
        {
            edge[i].cost = MAX_COST;
            edge[i^1].cost = MAX_COST;
        }
        else if(edge[i].cost == MAX_COST && video_list[v])  //之前不存在现在存在
        {
            edge[i].cost = 0;
            edge[i^1].cost = 0;
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
    do{
        do{
            memset(vis, 0, sizeof(vis));
        }while(dfs(src, INF));
    }while(change());
}