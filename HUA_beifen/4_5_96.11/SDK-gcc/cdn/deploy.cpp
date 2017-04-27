#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "Global.h"
#include "ZKW.h"
#include "GA.h"
#include "ZKW.h"
#include "TZKW.h"

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    srand(time(NULL));
    solve(topo, line_num);

//	 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(Global::get()->resultStr, filename);

}

double node_cost[NODE_MAXNUM];
double tnode_cost[NODE_MAXNUM];
bool v[NODE_MAXNUM];

void t_solve()
{
    Global* global = Global::get();
    for (int i = 0; i < global->netNodeNum; i++)
        node_cost[i] = global->videoCost*1.5/global->capSum[i]+0.5;

    int cnt = 3;
    int pre = 0;
    while(cnt) {

        TZKW *tzkw = new TZKW;
        ZKW *zkw = ZKW::get();

        tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
                   global->netNodeNum + 1);

        for (int i = 0; i < global->netNodeNum; i++) {
            if (global->hashNode[i] != -1) {
                tzkw->add_edge(global->netNodeNum, i, INF, node_cost[i]);
//            printf("%d %lf\n", (int)(global->videoCost*1.0/global->capSum[i]+0.5), global->videoCost*1.0/global->capSum[i]);
            }
        }
        tzkw->costflow();

        memset(v, 0, sizeof(v));

        for (int i = tzkw->head[tzkw->src]; i != -1; i = tzkw->edge[i].next) {
            if (i & 1)
                continue;
            if (tzkw->edge[i].cap != tzkw->edge[i].pre_cap) {
                v[tzkw->edge[i].v] = true;
            }
        }
        zkw->work(v);

        if(zkw->ans_flow != global->sumFlow)
            continue;

//        for(int i=0; i<global->netNodeNum; i++)
//            node_cost[i] = INF;

        for(int i=tzkw->head[tzkw->src]; i!=-1; i=tzkw->edge[i].next) {
            if (i & 1)
                continue;
            EDGE& e = tzkw->edge[i];
            int flow = e.pre_cap - e.cap;
            if(flow != 0)
                node_cost[e.v] = global->videoCost * 1.0/flow ;
            else
                node_cost[e.v] = INF;
        }

        printf("cost=%d flow=%d\n", zkw->ans_cost, zkw->ans_flow);
        if(pre == zkw->ans_cost)
            cnt--;

        pre = zkw->ans_cost;
    }

}

void test_solve()
{
    Global* global = Global::get();
    TZKW *tzkw = new TZKW;
    ZKW *zkw = ZKW::get();

    tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
               global->netNodeNum + 1);
    bool v[NODE_MAXNUM];
    memset(v, 0, sizeof(v));
    int num = 0;
    for (int i = 0; i < global->netNodeNum; i++) {
        v[i] = rand() % 2;
        if (v[i]) {
            tzkw->add_edge(global->netNodeNum, i, INF, 0);
            num++;
        }
    }
    tzkw->costflow();
    zkw->work(v);

    printf("tzkw: cost=%d flow=%d\n", tzkw->ans_cost+num*global->videoCost, tzkw->ans_flow);
    printf(" zkw: cost=%d flow=%d\n", zkw->ans_cost, zkw->ans_flow);



    tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
               global->netNodeNum + 1);
    memset(v, 0, sizeof(v));
    num = 0;
    for (int i = 0; i < global->netNodeNum; i++) {
        v[i] = rand() % 2;
        if (v[i]) {
            tzkw->add_edge(global->netNodeNum, i, INF, 0);
            num++;
        }
    }
    tzkw->costflow();
    zkw->change_work(v);

    printf("tzkw: cost=%d flow=%d\n", tzkw->ans_cost+num*global->videoCost, tzkw->ans_flow);
    printf(" zkw: cost=%d flow=%d\n", zkw->ans_cost, zkw->ans_flow);

}

void solve(char * topo[MAX_EDGE_NUM], int line_num)
{
    Global* global = Global::get();
    global->init();
    global->input_file(topo, line_num);

    ZKW *zkw = ZKW::get();
    zkw->init(global->edge, global->edgeNum, global->netNodeNum + 2, global->head, global->netNodeNum,
              global->netNodeNum + 1);

//    t_solve();
//    return;

    GA* ga = new GA(global->netNodeNum);
    ga->init();
    ga->work();

    if(ga->best_item.cost == INF || ga->best_item.cost >= global->videoCost*global->costNodeNum) {
        global->out_badResult();
        return;
    }

    zkw->work(ga->best_item.video_list);

    global->out_goodResult(ga->best_item.video_list);

//    if(zkw->ans_flow == global->sumFlow) {
//        printf("V:");
//    }else
//        printf("X:");
//    printf("cost=%d flow=%d sum_flow:%d now_flow:%d\n", zkw->ans_cost, zkw->ans_flow, global->sumFlow, global->now_flow);


}
