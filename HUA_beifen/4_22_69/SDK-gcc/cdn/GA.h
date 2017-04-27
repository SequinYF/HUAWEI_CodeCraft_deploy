//
// Created by shiyi on 2017/4/1.
//

#ifndef CDN_GA_H
#define CDN_GA_H

#include "Global.h"

struct GA_ITEM{

    int video_list[NODE_MAXNUM];
    int cost;

    bool operator<(const GA_ITEM& t) const
    {
        return this->cost < t.cost;
    }

    void init();

    bool check();

    void assess();

    void variation1(int f);

    void variation2(int f);

};

struct GA {
    int flag[1000000];
    GA_ITEM *group, *new_group;
    GA_ITEM agroup[1000];
    GA_ITEM bgroup[1000];
    GA_ITEM best_item;
    int best_iter;
    int GA_ITERATIONS;
    int GA_ITEM_NUM;
    int GA_VATRIATION;
    bool GA_VARITION_FLAG;

    int node_cost[NODE_MAXNUM];

    GA(int node_num)
    {
	    GA_ITERATIONS = 10000;
        GA_VATRIATION = 100;
        GA_VARITION_FLAG = true;

        if(node_num < 200)
        {
            GA_ITEM_NUM = 40;

        }
        else if(node_num < 500){
            GA_ITEM_NUM = 40;

        }
        else if(node_num < 1000){
            GA_ITEM_NUM = 10;
        }

        group = agroup;
        new_group = bgroup;
    }

    void init();

    void make_init1(GA_ITEM& item, double p);
    void make_init2(GA_ITEM& item, double p);

    void assess(){

        for(int i=0; i<GA_ITEM_NUM; i++)
            group[i].assess();
    }

    void variation();

    int getId1(int sum, int len);

    int getId2(int len);

    void cross_item(GA_ITEM& boy, GA_ITEM& girl, GA_ITEM& father, GA_ITEM& mother);

    void cross();

    void sort(){
        std::sort(group, group+GA_ITEM_NUM);
    }

    void work();
};

//void make_init(DE_item& item, double p) {
//    item.cost = INF;
//
//    int video_list[NODE_MAXNUM];
//    int ans[NODE_MAXNUM];
//
//    Global *global = Global::get();
//
//    int nodeCost[NODE_MAXNUM][MAX_TYPE];
//    for (int i = 0; i < global->netNodeNum; i++) {
//        for (int j = 0; j <= global->nodeMaxLevel[i]; j++) {
//            int cap = std::min(global->videoCost[j].cap, global->capSum[i]);
//            nodeCost[i][j] = (global->videoCost[j].cost + global->buildCost[i]) * p / cap;
//        }
//    }
//
//    TZKW *tzkw = TZKW::get();
//    ZKW *zkw = ZKW::get();
//
//    int videoCost = INF;
//    int preCost = INF;
//
//    for (int k = 0; k < 10; k++) {
//
//        tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
//                   global->netNodeNum + 1);
//
//        for (int i = 0; i < global->netNodeNum; i++) {
////            if (global->hashNode[i] != -1) {
//            for (int j = 0; j <= global->nodeMaxLevel[i]; j++) {
//                int cap = std::min(global->videoCost[j].cap, global->capSum[i]);
//                tzkw->add_edge(global->netNodeNum, i, cap, nodeCost[i][j]);
//            }
////            }
//        }
//
//        tzkw->costflow();
//
//        memset(video_list, 0, sizeof(video_list));
//
//        for (int i = tzkw->head[tzkw->src]; i != -1; i = tzkw->edge[i].next) {
//            if(i & 1)
//                continue;
//            EDGE &e = tzkw->edge[i];
//
//            if (e.cap != e.pre_cap) {
//                video_list[e.v] += e.pre_cap - e.cap;
//            }
//        }
//
//        for(int i=0; i<global->netNodeNum; i++){
//
//            if(video_list[i] > global->capSum[i]){
//                video_list[i] = global->capSum[i];
//            }
//        }
//
//        for (int i = 0; i < global->netNodeNum; i++) {
//            for (int j = 0; j <= global->nodeMaxLevel[i]; j++) {
//                nodeCost[i][j] = MAX_COST;
//            }
//            if (video_list[i] != 0) {
//                int vt = global->hashCapForType[video_list[i]];
//                nodeCost[i][vt] =
//                        (global->buildCost[i] + global->videoCost[vt].cost) * 1.0 / video_list[i];
//            }
//        }
//
//        zkw->work(video_list);
//        int cost = zkw->ans_cost;
//        for (int i = 0; i < global->netNodeNum; i++) {
//            if (video_list[i] != 0) {
//                int vt = global->hashCapForType[video_list[i]];
//                cost += global->buildCost[i] + global->videoCost[vt].cost;
//            }
//        }
//
//        if (item.cost > cost) {
//            item.cost = cost;
//            memcpy(item.video_list, video_list, sizeof(int)*global->netNodeNum);
//        } else if (cost == preCost) {
//            break;
//        }
//
//        printf("%d %d %d\n", tzkw->ans_cost, cost, item.cost);
//
//        preCost = cost;
//    }
//    printf("==   %d\n", item.cost);
//}

#endif //CDN_GA_H
