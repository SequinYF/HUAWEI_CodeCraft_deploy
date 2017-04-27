#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include "ZKW.h"
#include "GA.h"
#include "Dijkstra.h"
#include <queue>

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    srand(time(NULL));
    solve(topo, line_num);

//	 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(Global::get()->resultStr, filename);

}

bool tttt[NODE_MAXNUM];
int v_cost;

struct TNODE{
    int id;
    int flow;
    TNODE(int a, int b):id(a),flow(b){}
    bool operator < (const TNODE& h) const {
        return flow>h.flow;
    }
};

void tsolve()
{
    Global* global = Global::get();
    ZKW* zkw = ZKW::get();

    bool ans[NODE_MAXNUM];
    int ttans[NODE_MAXNUM];
    bool vis[NODE_MAXNUM];
    memset(vis, 0, sizeof(vis));

    memset(ans, 0, sizeof(ans));

    std::priority_queue<TNODE> q;

    for(int i=0; i<global->costNodeNum; i++) {
        q.push(TNODE(global->costEdge[i].node_id, global->costEdge[i].node_id));
        ans[global->costEdge[i].node_id] = true;
    }

    while(!q.empty()){
        TNODE now_node = q.top();
        q.pop();
        vis[now_node.id] = true;
        ans[now_node.id] = false;

        int p[29];
        int p_num = 0;
        for (int j=global->head[now_node.id]; j!=-1; j=global->edge[j].next){

            int v = global->edge[j].v;

            if(j&1 || global->edge[j].cost == 0 || vis[v])
                continue;

            if(ans[v])
                now_node.flow -= global->edge[j].cap;
            else
                p[p_num++] = j;
        }
        for(int i=0; i<p_num; i++)
            for(int j=i+1; j<p_num; j++){
                if(global->edge[p[i]].cost > global->edge[p[j]].cost)
                    std::swap(p[i], p[j]);
                else if(global->edge[p[i]].cost == global->edge[p[j]].cost){
                    if(global->edge[p[i]].cap < global->edge[p[j]].cap)
                        std::swap(p[i], p[j]);
                }
            }

        bool fff = false;

        for(int i=0; i<p_num; i++)
        {
            if(global->edge[p[i]].cap >= now_node.flow)
            {
                ans[global->edge[p[i]].v] = true;
                q.push(TNODE(global->edge[p[i]].v, now_node.flow));
                fff = true;
                break;
            }
        }
        if(fff)
            continue;
        for(int i=0; i<p_num; i++)
        {
            if(now_node.flow > global->edge[p[i]].cap){
                ans[global->edge[p[i]].v] = true;
                q.push(TNODE(global->edge[p[i]].v, global->edge[p[i]].cap));
                now_node.flow -= global->edge[p[i]].cap;
            }
            else{
                ans[global->edge[p[i]].v] = true;
                q.push(TNODE(global->edge[p[i]].v, now_node.flow));
                break;
            }
        }

        zkw->work(ans);
//        printf("%d %d\n", zkw->ans_flow, zkw->ans_cost);

        if (zkw->ans_flow == global->sumFlow) {
            if (zkw->ans_cost != 0) {

                if(zkw->ans_cost < v_cost) {
                    memcpy(tttt, ans, sizeof(tttt));
                    v_cost = zkw->ans_cost;
                }
                printf("-=-= %d -=-%d %d=\n", v_cost, zkw->ans_cost, zkw->ans_flow);

            }
        }
    }
}

void solve(char * topo[MAX_EDGE_NUM], int line_num)
{
    Global* global = Global::get();
    global->init();
    global->input_file(topo, line_num);

//    Dijkstra::get()->work();
//
//    return;

    ZKW *zkw = ZKW::get();
    zkw->init(global->edge, global->edgeNum, global->netNodeNum + 2, global->head, global->netNodeNum,
              global->netNodeNum + 1);

    tsolve();

    printf("初始化完毕\n");

    GA* ga = new GA(global->netNodeNum);
    printf("GA开始\n");

    ga->init(tttt, v_cost);
    printf("GA初始化\n");

    ga->work();

    if(ga->best_item.cost == INF || ga->best_item.cost >= global->videoCost*global->costNodeNum) {
        global->out_badResult();
        return;
    }

    bool v[NODE_MAXNUM];
    memset(v, 0, sizeof(v));
    for(int i=0; i<global->maxVideoNum; i++)
        v[ga->best_item.video_list[i]] = true;
    zkw->work(v);

    global->out_goodResult(v);

    if(zkw->ans_flow == global->sumFlow) {
        printf("V:");
    }else
        printf("X:");
    printf("cost=%d flow=%d sum_flow:%d now_flow:%d\n", zkw->ans_cost, zkw->ans_flow, global->sumFlow, global->now_flow);

}