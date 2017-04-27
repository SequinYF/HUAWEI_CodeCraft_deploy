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

void ttt() {

    Global *global = Global::get();
    ZKW *tttzkw = ZKW::get();
    TZKW* tzkw = new TZKW;

        printf("sumFlow %d\n", global->sumFlow);

    int v[NODE_MAXNUM];

    for (int j = 0; j < global->netNodeNum; j++) {
        if (rand() % 2 != 0) {
            v[j] = global->nodeMaxLevel[j] + 1;
        } else
            v[j] = 0;
    }
//    for (int j = 0; j < global->costNodeNum; j++) {
//        int cap = global->costEdge[j].flow_need;
//        int id = global->costEdge[j].node_id;
//        v[id] = global->nodeMaxLevel[id]+1;
//    }


    tzkw->work(v);
    printf("tzkw : %d %d\n", tzkw->ans_cost, tzkw->ans_flow);
    global->get_time();

    tttzkw->work(v);
    printf("===tttzkw : %d %d\n", tttzkw->ans_cost, tttzkw->ans_flow);
    global->get_time();



    int num;
//    num = rand()%global->netNodeNum;
//    v[num] = rand()%(global->nodeMaxLevel[num]+1);
    tzkw->work(v);
    printf("tzkw : %d %d\n", tzkw->ans_cost, tzkw->ans_flow);

    global->get_time();
    tttzkw->work(v);
    printf("===tttzkw : %d %d\n", tttzkw->ans_cost, tttzkw->ans_flow);
    global->get_time();

    tzkw->work(v);
    printf("tzkw : %d %d\n", tzkw->ans_cost, tzkw->ans_flow);

    global->get_time();
    tttzkw->work(v);
    printf("===tttzkw : %d %d\n", tttzkw->ans_cost, tttzkw->ans_flow);
    global->get_time();


    tzkw->work(v);
    printf("tzkw : %d %d\n", tzkw->ans_cost, tzkw->ans_flow);

    global->get_time();
    tttzkw->work(v);
    printf("===tttzkw : %d %d\n", tttzkw->ans_cost, tttzkw->ans_flow);
    global->get_time();

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
//    return;

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

