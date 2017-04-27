#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include "ZKW.h"
#include "GA.h"

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

void tsolve()
{
    Global* global = Global::get();
    ZKW* zkw = ZKW::get();

    int ans[NODE_MAXNUM];
    int ttans[NODE_MAXNUM];

    memset(ans, 0, sizeof(ans));
    for(int i=0; i<global->costNodeNum; i++)
        ans[global->costEdge[i].node_id] = global->costEdge[i].flow_need;

    v_cost = global->netNodeNum * global->videoCost;

    int ooo = 0;

    while(1) {

        memset(ttans, 0, sizeof(ttans));

        for(int i=0; i<global->netNodeNum; i++) {
            if (ans[i]) {
                int to = -1;
                int cost = INF;
                for (int k = global->head[i]; k != -1; k = global->edge[k].next) {
                    if (k % 2 != 0 || global->edge[k].cap < ans[i])
                        continue;
                    int v = global->edge[k].v;
                    if (global->edge[k].cost != 0 && cost > global->edge[k].cost) {
                        cost = global->edge[k].cost;
                        to = v;
                    }
                }
                if (to != -1) {
                    ttans[to] += ans[i];
                }
                else
                    ttans[i]+= ans[i];
            }
        }

        for(int i=0; i<global->netNodeNum; i++) {
            ans[i] = ttans[i];
        }

        bool v[NODE_MAXNUM];
        memset(v, 0, sizeof(v));

        for(int i=0; i<global->netNodeNum; i++){
            v[i] = ans[i]>0?1:0;
        }

        zkw->work(v);
//        printf("%d %d\n", zkw->ans_flow, zkw->ans_cost);

        if (zkw->ans_flow == global->sumFlow) {
            if (zkw->ans_cost != 0) {

                if(zkw->ans_cost < v_cost) {
                    memcpy(tttt, v, sizeof(tttt));
                    v_cost = zkw->ans_cost;
                    ooo = 0;
                }
//                printf("-=-= %d -=-=\n", v_cost);

            }
//        } else {
//            ooo++;
//            for(int i=0; i<ooo; i++) {
//                int a;
//                do
//                    a = rand() % netNodeNum;
//                while (!ans[a]);
//                int b;
//                do
//                    b = rand() % netNodeNum;
//                while (ans[b]);
//                swap(ans[a], ans[b]);
//            }
//            if(ooo > 5)
//                ooo = 0;
        }
        if(ooo++ > 50)
            break;

    }
}

bool init_video_list[NODE_MAXNUM];
int init_cost;

void init_solve()
{
    Global* global = Global::get();
    TZKW* tzkw = new TZKW;
    tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
              global->netNodeNum + 1);

    for(int i=0; i<global->netNodeNum; i++) {
        if (global->hashNode[i] != -1) {
            tzkw->add_edge(global->netNodeNum, i, INF, global->videoCost*6.0 / global->capSum[i]);
//            printf("%d %lf\n", (int)(global->videoCost*1.0/global->capSum[i]), global->videoCost*1.0/global->capSum[i]);
        }
    }

    tzkw->costflow();

    init_cost = tzkw->ans_cost;
//    printf("%d %d\n", tzkw->ans_flow, init_cost);

    memset(init_video_list, 0, sizeof(init_video_list));

    for(int i=tzkw->head[tzkw->src]; i!=-1; i=tzkw->edge[i].next) {
        if(i&1)
            continue;
        if(tzkw->edge[i].cap != tzkw->edge[i].pre_cap) {
            init_video_list[tzkw->edge[i].v] = true;
        }
    }

    ZKW* zkw = ZKW::get();
    zkw->init(global->edge, global->edgeNum, global->netNodeNum + 2, global->head, global->netNodeNum,
              global->netNodeNum + 1);
    zkw->work(init_video_list);
    init_cost = zkw->ans_cost;
//    printf("%d %d\n", zkw->ans_flow, init_cost);
}

void solve(char * topo[MAX_EDGE_NUM], int line_num)
{
    Global* global = Global::get();
    global->init();
    global->input_file(topo, line_num);

    init_solve();

    ZKW *zkw = ZKW::get();
    zkw->init(global->edge, global->edgeNum, global->netNodeNum + 2, global->head, global->netNodeNum,
              global->netNodeNum + 1);

//    tsolve();

    GA* ga = new GA(global->netNodeNum);
    ga->init(init_video_list, init_cost);
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
