//
// Created by shiyi on 2017/4/17.
//

#ifndef CDN_DE_H
#define CDN_DE_H

#include "Global.h"
#include "ZKW.h"
#include "TZKW.h"

#define frand(a) (rand()%100001*a*1.0/100000.0)

struct DE_item{
    int video_list[NODE_MAXNUM];
    int cost;
};

struct DE {
    Global* global;
    int item_num;
    int iterations;
    DE_item group[1000];
    DE_item best_item;
    ZKW* zkw;

    DE(){
        iterations = 100000;

        global = Global::get();
        zkw = ZKW::get();

        int node_num = global->netNodeNum;

        if(node_num < 200)
        {
            item_num = 20;
        }
        else if(node_num < 500){
            item_num = 10;
        }
        else if(node_num < 1000){
            item_num = 10;
        }
        else{
            item_num = 10;
        }
    }

    void make_init(DE_item& best_item, double p)
    {
        best_item.cost = INF;

        int video_list[NODE_MAXNUM];
        int ans[NODE_MAXNUM];

        Global* global = Global::get();

        int nodeCost[NODE_MAXNUM];
        for(int i=0; i<global->netNodeNum; i++) {
            VideoType& vt = global->videoCost[global->capSum[i]];
            nodeCost[i] = (vt.cost+global->buildCost[i]) * 1.0 / global->capSum[i];
        }

        TZKW* tzkw = TZKW::get();
        ZKW* zkw = ZKW::get();

        int videoCost = INF;
        int preCost = INF;

        for(int k=0; k<10; k++) {

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

            zkw->work(video_list);

            int cost = zkw->ans_cost;
            for(int i=0; i<global->netNodeNum; i++) {
                if (video_list[i] != 0) {
                    int vt = global->hashCapForType[video_list[i]];
                    cost += global->buildCost[i] + global->videoCost[vt].cost;
                }
            }

            if(best_item.cost > cost)
            {
                best_item.cost = cost;
                memcpy(best_item.video_list, video_list, sizeof(video_list));
            }
            else if(cost == preCost){
                break;
            }

            preCost = cost;
        }
    }

    void init(){

        make_init(best_item, 1);

        for(int i=0; i<item_num; i++){
            DE_item& item = group[i];

            make_init(group[i], i+1);
//            for(int j=0; j<global->netNodeNum; j++){
//                if(rand()%2){
//                    item.video_list[j] = rand()%(global->capSum[j]+1);
//                }
//            }



        }
    }

    void assess_item(DE_item& item) {
        zkw->work(item.video_list);
        item.cost = zkw->ans_cost;

        for (int j = 0; j < global->netNodeNum; j++) {
            if (item.video_list[j] != 0) {
                int vt = global->hashCapForType[item.video_list[j]];
                item.cost += global->buildCost[j] + global->videoCost[vt].cost;
            }
        }

        if (item.cost < best_item.cost) {
            best_item = item;
        }
    }

    void assess(){

        for(int i=0; i<item_num; i++) {
            assess_item(group[i]);
        }
    }

    void update_item(DE_item& item){
        DE_item temp;

        int a, b, c, d;
        do{
            a = rand()%item_num;
            b = rand()%item_num;
            c = rand()%item_num;
            d = rand()%item_num;
        }while(a!=b && a!=c && a!=d
                && b!=c && b!=d && c!=d);

//        do{
//            a = rand()%item_num;
//            b = rand()%item_num;
//            c = rand()%item_num;
//        }while(a!=b && a!=c
//                && b!=c);

        for(int i=0; i<global->netNodeNum; i++){
            int r = rand()%11;
            if(r > 8) {
//                temp.video_list[i] = group[a].video_list[i]+
//                                     frand(1)*(group[b].video_list[i]-group[c].video_list[i])+0.5;
                temp.video_list[i] = best_item.video_list[i]
                                     + frand(1) * (group[a].video_list[i] + group[b].video_list[i]
                                                   - group[c].video_list[i] - group[d].video_list[i]) + 0.5;
            }
            else{
                temp.video_list[i] = item.video_list[i];
            }
        }

        assess_item(temp);

        if(temp.cost < item.cost){
            item = temp;
        }
        else{
//            double r = frand(1);
//            if(r > item.cost*1.0/temp.cost)
//                item = temp;
//            printf("%lf %lf %d %d\n", r, item.cost*1.0/temp.cost, item.cost, temp.cost);
        }
    }

    void update(){
        for(int i=0; i<item_num; i++)
            update_item(group[i]);
    }

    void work(){
        init();
        assess();

        for(int k=0; k<iterations; k++){

            int time = global->get_time()-global->start_time;

//            for(int i=0; i<item_num; i++)
//                printf("%d ", group[i].cost);
//            printf("\n");
//
//            printf("第%d代, cost=%d time=%d\n", k, best_item.cost, time);

            if (global->get_time() - global->start_time > 86)
                break;

            update();
        }
    }
};


#endif //CDN_DE_H
