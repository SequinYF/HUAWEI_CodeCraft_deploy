//
// Created by shiyi on 2017/4/2.
//

#ifndef CDN_DIJKSTRA_H
#define CDN_DIJKSTRA_H

#include "Global.h"
#include <vector>
#include <queue>

struct HeapNode{  //prority_queue 中的优先级
    int u,dist;   //dist: u点到起点的最短路 ,u: 有向边的终点
    HeapNode(int u,int d):u(u),dist(d){}
    bool operator < (const HeapNode& h) const {
        return dist>h.dist;
    }
};

struct Edge{
    int v, cost, cap, next;
    Edge(){}
    Edge(int v,int cap, int cost)
            :v(v), cap(cap), cost(cost)
    {}
};

struct Dijkstra{ //打包在Dijkstra中

    Edge edge[EDGE_MAXNUM];
    int edge_num;
    int node_num;
    int head[MAX_EDGE_NUM];
    bool vis[NODE_MAXNUM];
    int dist[COST_MAXNUM][NODE_MAXNUM];
    int pre_node[COST_MAXNUM][NODE_MAXNUM];
    int pre_edge[COST_MAXNUM][NODE_MAXNUM];
    int flow[COST_MAXNUM][NODE_MAXNUM];
    int m_flow[NODE_MAXNUM];
    int need_flow[NODE_MAXNUM];
    int sum_flow;
    bool flag[NODE_MAXNUM];

    Dijkstra(){}

    static Dijkstra* get(){
        static Dijkstra dijkstra;
        return &dijkstra;
    }

    void init(int n){
        memset(head, -1, sizeof(head));
        memset(m_flow, 0, sizeof(m_flow));

        sum_flow = 0;
        edge_num = 0;
        node_num = n;
    }
    void addEdge(int u, int v, int cap, int cost){
        edge[edge_num].v = v;
        edge[edge_num].cap = cap;
        edge[edge_num].cost = cost;
        edge[edge_num].next = head[u];
        head[u] = edge_num++;

        edge[edge_num].v = u;
        edge[edge_num].cap = cap;
        edge[edge_num].cost = cost;
        edge[edge_num].next = head[v];
        head[v] = edge_num++;
    }
    //每次只算没满足的消费节点的最大容量合路径
    void dijkstra(int x, int s)
    {
        std::priority_queue<HeapNode> Q;
        memset(vis, false, sizeof(vis));

        dist[x][s] = 0;
        Q.push(HeapNode(s,0));
        while(!Q.empty())
        {
            int u = Q.top().u;
            Q.pop();
            if(vis[u])
                continue;
            vis[u] = true;
            for(int i = head[u]; i != -1; i=edge[i].next)
            {
                Edge& e = edge[i];
                int v = e.v;
                int cost = e.cost;

                if(edge[i^1].cap > 0 && dist[x][v] > dist[x][u]+cost)
                {
                    dist[x][v] = dist[x][u] + cost;
                    flow[x][v] = std::min(flow[x][u], edge[i^1].cap);
                    Q.push(HeapNode(v, dist[x][v]));
//                    printf("-=%d=%d-\n", flow[x][v], dist[x][v]);
                }
            }
        }

    }

    void push_flow(int st)
    {
        Global* global = Global::get();

        memset(vis, 0, sizeof(vis));

        std::queue<int> q;
        q.push(st);
        vis[st] = true;

        while(!q.empty()) {
            int u = q.front();
            q.pop();

            int costNodeId = global->hashNode[u];
            if (costNodeId != -1)
            {
                int t = std::min(need_flow[costNodeId], m_flow[u]);
                need_flow[costNodeId] -= t;
                m_flow[u] -= t;
                sum_flow += t;
            }

            for(int i=head[u]; i!=-1; i=edge[i].next)
            {
                if(m_flow[u] <= 0)
                    break;
                Edge& e = edge[i];
                if(!vis[e.v] && e.cap > 0 && !flag[e.v])
                {
                    vis[e.v] = true;
                    int t = std::min(e.cap, m_flow[u]);
                    e.cap -= t;
                    m_flow[e.v] += t;
                    m_flow[u] -= t;
                    q.push(e.v);
                }
            }
        }


    }



    void work()
    {
        Global* global = Global::get();
        for(int i=0; i<global->costNodeNum; i++)
            need_flow[i] = global->costEdge[i].flow_need;

//        printf("sumFlow = %d\n", global->sumFlow);
        memset(flag, 0, sizeof(flag));

        while(sum_flow < global->sumFlow) {

//            memset(dist, 0x3f, sizeof(dist));
//            memset(flow, 0x3f, sizeof(flow));
//
//            for (int i = 0; i < global->costNodeNum; i++) {
//                dijkstra(i, global->costEdge[i].node_id);
//                for(int j=0; j<global->netNodeNum; j++)
//                    if(flow[i][j] == 0x3f3f3f3f)
//                        flow[i][j] = 0;
//            }
//
//            int mx = 0;
//            int mi = 0;
//            for (int i = 0; i < global->netNodeNum; i++) {
//                flow[global->costNodeNum][i] = 0;
//                for (int j = 0; j < global->costNodeNum; j++)
//                    if(need_flow[j] != 0)
//                        flow[global->costNodeNum][i] += flow[j][i];
//                if(!flag[i] && mx < flow[global->costNodeNum][i]) {
//                    mx = flow[global->costNodeNum][i];
//                    mi = i;
//                }
//            }

            int mi = 0;
            do{
                mi = rand()%global->netNodeNum;
            }while(flag[mi]);
            m_flow[mi] = INF;
//            if(!flag[mi])
                push_flow(mi);
            flag[mi] = true;
            printf("sumFlow=%d\n", sum_flow);
            printf("mx=%d mi=%d\n",mi, mi);
            getchar();
        }

        int num = 0;
        for(int i=0; i < global->netNodeNum; i++)
            if(flag[i])
                num++;
        printf("preSumFlow=%d nowSumFlow=%d videoNum=%d\n", global->sumFlow, sum_flow, num);

    }


//    vector<int> getLoad(int s, int t)
//    {
//        vector<int> ans;
//
//        if(this->dist[t] == 0x3f3f3f3f)
//            return ans;
//
//        for(int i=t;i!=s;i=this->edges[this->p[i]].from)
//            ans.push_back(i);
//
//        return ans;
//    }
};


#endif //CDN_DIJKSTRA_H
