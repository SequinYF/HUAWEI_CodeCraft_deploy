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

    bool operator<(const DE_item& t) const
    {
        return this->cost < t.cost;
    }
};

struct DE {
    Global* global;
    int item_num;
    int iterations;
    DE_item group[1000];
    DE_item best_item;
    ZKW* zkw;
    double cr;
    double f;
    int bianyi_num;

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
            cr = 5;
            bianyi_num = 5;
        }
        else{
            item_num = 5;
            cr = 5;
            bianyi_num = 10;
        }
    }

    void make_init(DE_item& item, double p) {
        item.cost = INF;

        int video_list[NODE_MAXNUM];
        int ans[NODE_MAXNUM];

        Global* global = Global::get();

        int nodeCost[NODE_MAXNUM];
        for(int i=0; i<global->netNodeNum; i++) {
            VideoType& vt = global->videoCost[global->capSum[i]];
            nodeCost[i] = (vt.cost+global->buildCost[i]) * p / global->capSum[i];
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
                    video_list[e.v] = vt+1;
                    nodeCost[e.v] = frand(1)*(global->buildCost[e.v] + global->videoCost[vt].cost) * 1.0 / (e.pre_cap - e.cap);
                } else
                    nodeCost[e.v] = MAX_COST;
            }

            zkw->work(video_list);

            int cost = zkw->ans_cost;
            for(int i=0; i<global->netNodeNum; i++) {
                if (video_list[i] != 0) {
                    int vt = video_list[i]-1;
                    cost += global->buildCost[i] + global->videoCost[vt].cost;
                }
            }

//            printf("%d %d\n", tzkw->ans_cost, cost);

            if(item.cost > cost)
            {
                item.cost = cost;
                memcpy(item.video_list, video_list, sizeof(video_list));
            }
            else if(cost == preCost){
                break;
            }

            preCost = cost;
        }
    }

    void make_init2(DE_item& item) {
        memset(item.video_list, 0, sizeof(int) * global->netNodeNum);
        for (int j = 0; j < global->costNodeNum; j++) {
            int cap = global->costEdge[j].flow_need;
            int id = global->costEdge[j].node_id;
            item.video_list[id] = global->nodeMaxLevel[id]+1;
        }
    }

    void init(){
        make_init(best_item, 1);
//        make_init2(best_item);
//        assess_item(best_item);
        for(int i=0; i<item_num; i++) {
            DE_item &item = group[i];

            make_init(group[i], frand(20));
//            for(int j=0; j<global->netNodeNum; j++){
//                if(rand()%2){
//                    item.video_list[j] = rand()%(global->capSum[j]+1);
//                }
//            }
//            make_init2(group[i]);
//            assess_item(item);

            if (item.cost < best_item.cost) {
                std::swap(item, best_item);
            }
        }
    }

    void assess_item(DE_item& item) {

        zkw->work(item.video_list);
        item.cost = zkw->ans_cost;

        for (int j = 0; j < global->netNodeNum; j++) {
            if (item.video_list[j] != 0) {
                int vt = item.video_list[j]-1;
                item.cost += global->buildCost[j] + global->videoCost[vt].cost;
            }
        }
    }

    void assess(){

        for(int i=0; i<item_num; i++) {
            assess_item(group[i]);
        }
    }

    bool flag;

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

        for(int i=0; i<global->netNodeNum; i++){

            int r = rand()%10;
            if(r > cr) {
//                if(flag)
//                    temp.video_list[i] = group[a].video_list[i]+
//                                     frand(1)*(group[b].video_list[i]-group[c].video_list[i])+0.5;
//                else
                    temp.video_list[i] = best_item.video_list[i]
                                     + frand(1) * (group[a].video_list[i] + group[b].video_list[i]
                                                   - group[c].video_list[i] - group[d].video_list[i]) + 0.5;
                temp.video_list[i] = std::min(global->nodeMaxLevel[i]+1, temp.video_list[i]);
                temp.video_list[i] = std::max(0, temp.video_list[i]);
            }
            else{
                temp.video_list[i] = item.video_list[i];
            }
        }

//        printf("b %d\n", global->get_time()-global->start_time);
        assess_item(temp);
//        printf("e %d\n", global->get_time()-global->start_time);

        if(temp.cost < item.cost){
            item = temp;
            if (item.cost < best_item.cost)
                best_item = item;
//                std::swap(item, best_item);
        }
//        else{
//            double r = frand(1);
//            if(r > item.cost*1.0/temp.cost)
//                item = temp;
//            printf("%lf %lf %d %d\n", r, item.cost*1.0/temp.cost, item.cost, temp.cost);
//        }
    }

    void update(){
        for(int i=0; i<item_num; i++)
            update_item(group[i]);
    }

    void work(){
        init();
//        assess();
//        printf("==%d  ==%d\n", group[0].cost, group[1].cost);
        flag = true;
        for(int k=0; k<iterations; k++){

            int time = global->get_time()-global->start_time;

//            if(time > 40)
//                flag = false;
//
            if (time > 87)
                break;

//            for(int i=0; i<item_num; i++)
//                printf("%d ", group[i].cost);
//            printf("\n");
//            printf("第%d代, cost=%d time=%d\n", k, best_item.cost, time);

            update();
            bianyi(best_item);
//
//            for(int i=0; i<item_num; i++)
//                bianyi(group[i]);

//            std::sort(group, group+item_num*2);
        }
    }

    void bianyi(DE_item& item){

        DE_item t_item = item;
        for(int t=0; t<10; t++) {
            DE_item temp = t_item;

            int p;
            do {
                p = rand() % global->netNodeNum;
            } while (temp.video_list[p] == 0);

            int q = rand() % global->graph[p].size();

            q = global->graph[p][q];
            temp.video_list[p] = 0;
            temp.video_list[q] = rand()%(global->nodeMaxLevel[q]+1);
            //        temp.video_list[q] = std::max(0, temp.video_list[q]);
            assess_item(temp);

            if (temp.cost < item.cost) {
//                group[rand()%item_num] = item;
                item = temp;
//                if (item.cost < best_item.cost) {
//                    std::swap(best_item, item);
//                }
            }
        }
    }
};


#endif //CDN_DE_H
