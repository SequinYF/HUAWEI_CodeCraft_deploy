#include "deploy.h"
#include <stdio.h>
#include <stdlib.h>
#include "Global.h"
#include "ZKW.h"
#include "GA.h"
#include "ZKW.h"

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    srand(time(NULL));
    solve(topo, line_num);

//	 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(Global::get()->resultStr, filename);

}

void solve(char * topo[MAX_EDGE_NUM], int line_num)
{
    Global* global = Global::get();
    global->init();
    global->input_file(topo, line_num);

    ZKW *zkw = ZKW::get();
    zkw->init(global->edge, global->edgeNum, global->netNodeNum + 2, global->head, global->netNodeNum,
              global->netNodeNum + 1);

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
