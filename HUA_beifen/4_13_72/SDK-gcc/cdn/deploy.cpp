#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include "Global.h"
#include "ZKW.h"
#include "TZKW.h"
#include "GA.h"

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    srand(time(NULL));
    solve(topo, line_num);

//	 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(Global::get()->resultStr, filename);

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

    for(int k=0; k<10; k++) {

        tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
                   global->netNodeNum + 1);

        for (int i = 0; i < global->netNodeNum; i++) {
            if (global->hashNode[i] != -1) {
                tzkw->add_edge(global->netNodeNum, i, INF, nodeCost[i]);
            }
        }

        tzkw->costflow();

        memset(video_list, 0, sizeof(video_list));

        for (int i = tzkw->head[tzkw->src]; i != -1; i = tzkw->edge[i].next) {

            EDGE &e = tzkw->edge[i];

            if (e.cap != e.pre_cap) {
                int vt = global->hashCapForType[e.pre_cap - e.cap];
                video_list[e.v] = vt+1;
                nodeCost[e.v] = (global->buildCost[e.v] + global->videoCost[vt].cost) * 1.0 / (e.pre_cap - e.cap);
            } else
                nodeCost[e.v] = MAX_COST;
        }
        printf("\nT%d %d\n", tzkw->ans_cost, tzkw->ans_flow);
        global->get_time();
        zkw->work(video_list);
        int cost = zkw->ans_cost;
        for(int i=0; i<global->netNodeNum; i++){
            if(video_list[i] != 0)
                cost += global->videoCost[video_list[i]-1].cost + global->buildCost[i];
        }
//        printf("V%d %d\n", cost, zkw->ans_flow);
        global->get_time();

        if(videoCost > cost)
        {
            videoCost = cost;
            memcpy(ans, video_list, sizeof(video_list));
        }
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

    GA* ga = new GA(global->netNodeNum);
    ga->init();
    ga->work();

    global->out_goodResult(ga->best_item.video_list);
}

