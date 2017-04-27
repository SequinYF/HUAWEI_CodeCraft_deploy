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
            EDGE& fakeE = global->edge[global->hashAEdge[i]];
            EDGE& realE = global->edge[global->hashBEdge[i]];
            int fakeFlow = fakeE.pre_cap - fakeE.cap;
            int realFlow = realE.pre_cap - realE.cap;

            int flow = fakeFlow + realFlow;
            video_list[i] = flow;
            ans_cost += (realE.cost - fakeE.cost) * fakeFlow;

        }else{
            EDGE& fakeE = global->edge[global->hashAEdge[i]];

            int fakeFlow = fakeE.pre_cap - fakeE.cap;
            video_list[i] = fakeFlow;
            ans_cost -= fakeE.cost * fakeFlow;
        }
    }
//    printf("ee %d\n", global->get_time()-global->start_time);
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
        EDGE& realEdge = edge[global->hashBEdge[i]];
        EDGE& fakeEdge = edge[global->hashAEdge[i]];

//        printf("%d ", video_list[i]);

        if(video_list[i] != 0){
            int cap = std::min(global->capSum[i], video_list[i]);
            realEdge.pre_cap = realEdge.cap = cap;
            realEdge.cost = 0;

            fakeEdge.pre_cap = fakeEdge.cap = global->capSum[i]-cap;
            fakeEdge.cost = MAX_COST;
        }
        else{
            realEdge.pre_cap = realEdge.cap = 0;
            realEdge.cost = MAX_COST;

            fakeEdge.pre_cap = fakeEdge.cap = global->capSum[i];
            fakeEdge.cost = MAX_COST;
        }
    }
//    printf("\n");
}

void ZKW::t_work(int* video_list)
{
//    Global* global = Global::get();
//
//    memset(dis, 0, sizeof(dis));
//
//    for(int i=0; i<node_num-2; i++)
//    {
//        EDGE& fakeE = global->edge[global->hashAEdge[i]];
//        EDGE& realE = global->edge[global->hashBEdge[i]];
//        fakeE.cap = fakeE.pre_cap;
//        fakeE.cost = MAX_COST;
//
//        if(video_list[i] != p_ans[i]){
//            if(p_ans[i] == 0){
//                realE.pre_cap = realE.cap = global->videoCost[p_ans[i]-1].cap;
//                realE.cost = 0;
//            }
//            else if(video_list[i] == 0){
//                realE.pre_cap = realE.cap = 0;
//                realE.cost = MAX_COST;
//                ans_cost += (fakeE.pre_cap-fakeE.cap)*MAX_COST;
//            }
//            else{
//                int flow = std::max(fakeE.pre_cap-fakeE.cap, global->videoCost[video_list[i]-1].cap);
//                video_list[i] = global->hashCapForType[flow]+1;
//                realE.pre_cap = realE.cap = global->videoCost[video_list[i]-1].cap;
//                realE.cost = 0;
//            }
//        }
//    }
//
//    do{
//        do{
//            memset(vis, 0, sizeof(vis));
//        }while(t_dfs(src, INF, 0));
//    }while(t_change());
//
//
//    for(int i=0; i<node_num-2; i++){
//
//        if(video_list[i] != 0){
//            EDGE& fakeE = global->edge[global->hashAEdge[i]];
//            EDGE& realE = global->edge[global->hashBEdge[i]];
//            int fakeFlow = fakeE.pre_cap - fakeE.cap;
//            int realFlow = realE.pre_cap - realE.cap;
//
//            int flow = fakeFlow + realFlow;
//            if(flow != 0) {
//                int vt = global->hashCapForType[flow];
//                if(vt == -1)
//                {
//                    ans_cost = INF;
//                    return;
//                }
//                video_list[i] = vt+1;
//                ans_cost += (realE.cost - fakeE.cost) * fakeFlow;
//            }
//            else
//                video_list[i] = 0;
//        }else{
//            EDGE& fakeE = global->edge[global->hashAEdge[i]];
//
//            int fakeFlow = fakeE.pre_cap - fakeE.cap;
//            if(fakeFlow != 0){
//                int vt = global->hashCapForType[fakeFlow];
//                if(vt == -1)
//                {
//                    ans_cost = INF;
//                    return;
//                }
//                video_list[i] = vt+1;
//                ans_cost -= fakeE.cost * fakeFlow;
//            }
//        }
//    }
}

int ZKW::t_dfs(int now_node, int max_f, int now_cost)
{
    if(now_node == this->des || max_f == 0) {
        if(now_cost <= 0)
            return 0;
        ans_flow -= max_f;
        return max_f;
    }
    int used = 0;
    vis[now_node] = true;
    for(int i=head[now_node]; i != -1; i=edge[i].next){
        if(edge[i].cap && dis[edge[i].v]+edge[i].cost==dis[now_node] && !vis[edge[i].v]){

            int f = t_dfs(edge[i].v, std::min(edge[i].cap, max_f-used), now_cost+edge[i].cost);

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

bool ZKW::t_change()
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
//            printf("----\n");
        }while(dfs(src, INF));
//        printf("====\n");
    }while(change());
}