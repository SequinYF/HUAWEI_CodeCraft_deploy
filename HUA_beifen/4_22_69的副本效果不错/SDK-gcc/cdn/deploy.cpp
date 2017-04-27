#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include "Global.h"
#include "ZKW.h"
#include "TZKW.h"
#include "GA.h"
#include "ACO.h"
#include "PSO.h"
#include "DE.h"

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    srand(time(NULL));
    solve(topo, line_num);

//	 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(Global::get()->resultStr, filename);

}

void ttt(){
    Global* global = Global::get();

    int d[NODE_MAXNUM];
    int q[NODE_MAXNUM];
    memcpy(q, global->buildCost, sizeof(q));
    for(int i=0; i<global->netNodeNum; i++)
        d[i] = i;
    for(int i=0; i<global->netNodeNum; i++){
        for(int j=i+1; j<global->netNodeNum; j++){
            if(q[d[i]] > q[d[j]]){
                std::swap(q[d[i]], q[d[j]]);
                std::swap(d[i], d[j]);
            }else if(q[d[i]] == q[d[j]]){
                if(global->capSum[d[i]] < global->capSum[d[j]])
                {
                    std::swap(d[i], d[j]);
                }
            }
        }
    }

    ZKW* zkw = ZKW::get();

    int v[NODE_MAXNUM];
    memset(v, 0, sizeof(v));
    int flow = 0;
    for(int i=0; i<global->netNodeNum; i++){
        flow += global->capSum[d[i]];
        v[d[i]] = global->capSum[d[i]];
        if(flow >= global->sumFlow){
            zkw->work(v);
            if(zkw->ans_flow == global->sumFlow){
                int cost = zkw->ans_cost;
                for(int j=0; j<global->netNodeNum; j++) {
                    if (v[j] != 0) {
                        int vt = global->hashCapForType[v[j]];
                        cost += global->buildCost[j] + global->videoCost[vt].cost;
                    }
                }
                printf("%d\n", cost);
//                return;
            }
        }
    }
}


void make_ans(){
    int video_list[NODE_MAXNUM];
    int ans[NODE_MAXNUM];

    Global* global = Global::get();

    int t = 0;
    for(int i=0; i<global->costNodeNum; i++){
        t += global->buildCost[global->costEdge[i].node_id];
    }
    printf("%d===%d\n", global->sumFlow, t);

    int nodeCost[NODE_MAXNUM];
    for(int i=0; i<global->netNodeNum; i++) {
        VideoType& vt = global->videoCost[global->capSum[i]];
        nodeCost[i] = (vt.cost+global->buildCost[i]) * 1.0 / global->capSum[i];
    }

    TZKW* tzkw = TZKW::get();
    ZKW* zkw = ZKW::get();

    int videoCost = INF;
    int preCost = INF;

    for(int k=0; ; k++) {

        tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
                   global->netNodeNum + 1);

        for (int i = 0; i < global->netNodeNum; i++) {
//            if (global->hashNode[i] != -1) {
                tzkw->add_edge(global->netNodeNum, i, global->capSum[i], nodeCost[i]);
//            }
        }

        tzkw->costflow();

        memset(video_list, 0, sizeof(video_list));

        for (int i = tzkw->head[tzkw->src]; i != -1; i = tzkw->edge[i].next) {

            EDGE &e = tzkw->edge[i];

            if (e.cap != e.pre_cap) {
                int vt = global->hashCapForType[e.pre_cap - e.cap];
                video_list[e.v] = e.pre_cap - e.cap;
                nodeCost[e.v] = (global->buildCost[e.v] + global->videoCost[vt].cost) * 1.0 / (e.pre_cap - e.cap);
            } else
                nodeCost[e.v] = MAX_COST;
        }
        printf("\nT%d %d\n", tzkw->ans_cost, tzkw->ans_flow);

        zkw->work(video_list);

        int cost = zkw->ans_cost;
        for(int i=0; i<global->netNodeNum; i++) {
            if (video_list[i] != 0) {
                int vt = global->hashCapForType[video_list[i]];
                cost += global->buildCost[i] + global->videoCost[vt].cost;
            }
        }
        printf("V%d %d %d\n", cost, zkw->ans_flow, videoCost);

        if(videoCost > cost)
        {
            videoCost = cost;
            memcpy(ans, video_list, sizeof(video_list));
        }
        else if(cost == preCost){
            break;
        }

        preCost = cost;
    }

    printf("%d\n", videoCost);
    global->out_goodResult(ans);
}

void solve(char * topo[MAX_EDGE_NUM], int line_num)
{
    Global* global = Global::get();
    global->init();
    global->input_file(topo, line_num);

    ZKW* zkw = ZKW::get();
    zkw->init(global->edge, global->edgeNum, global->netNodeNum + 2, global->head, global->netNodeNum,
              global->netNodeNum + 1);

//    ttt();
//    make_ans();
//    return;

//    GA* ga = new GA(global->netNodeNum);
//    ga->work();

    DE* de = new DE;
    de->work();

    global->out_goodResult(de->best_item.video_list);
//    PSO* pso = new PSO;
//    pso->init();
//    pso->work();
//
//    global->out_goodResult(pso->Gvideo_list);

//    ACO* aco = new ACO;
//    aco->init();
//    aco->work();

//    GA* ga = new GA(global->netNodeNum);
//    ga->init();
//    ga->work();
//
//    global->out_goodResult(ga->best_item.video_list);
}

