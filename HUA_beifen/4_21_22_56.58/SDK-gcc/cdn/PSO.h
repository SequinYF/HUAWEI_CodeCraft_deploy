//
// Created by shiyi on 2017/4/14.
//

#ifndef CDN_PSO_H
#define CDN_PSO_H

#include "Global.h"
#include "ZKW.h"
#include "TZKW.h"

#define frand(a) (rand()%100001*a*1.0/100000.0)


struct PSO_item{
    int video_list[NODE_MAXNUM];
    int v[NODE_MAXNUM];
    int cost;
    int best_list[NODE_MAXNUM];
    int best_cost;
};

struct PSO {
    Global* global;
    int item_num;
    int iterations;
    PSO_item group[1000];
    int Gvideo_list[NODE_MAXNUM];
    int Gcost;
    ZKW* zkw;
    PSO_item best_item;

    double c1;  //全局因子
    double c2;  //局部因子
    double w;   //惯性权重
    double r;   //约束因子

    PSO(){
        c1 = 2.5;
        c2 = 1.5;
        w = 0.5;
        r = 0.729;

        iterations = 100000;
        zkw = ZKW::get();
        global = Global::get();

        int node_num = global->netNodeNum;

        if(node_num < 200)
        {
            item_num = 100;
            c1 = 1;
            c2 = 3;
        }
        else if(node_num < 500){
            item_num = 100;
            c1 = 3;
            c2 = 2;
        }
        else if(node_num < 1000){
            item_num = 3;
            c1 = 3;
            c2 = 1;
        }else{
            item_num = 3;
            c1 = 3;
            c2 = 1;
        }
    };

    void make_init(int* Glist, int& Gcost, double p)
    {
        Gcost = INF;

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

            if(Gcost > cost)
            {
                Gcost = cost;
                memcpy(Glist, video_list, sizeof(video_list));
            }
            else if(cost == preCost){
                break;
            }

            preCost = cost;
        }
    }

    void init(){

        Gcost = INF;
        make_init(Gvideo_list, Gcost, 1);

        for(int i=0; i<item_num; i++){
            PSO_item& item = group[i];

            for(int j=0; j<global->netNodeNum; j++){
//                if(rand()%2){
//                    item.video_list[j] = rand()%(global->capSum[j]+1);
//                }
//                else
//                    item.video_list[j] = 0;
                item.v[j] = rand()%global->capSum[j]-global->capSum[j]/2;
//                memcpy(item.best_list, item.video_list, sizeof(int)*global->netNodeNum);
//                item.best_cost = INF;
            }

            make_init(item.video_list, item.cost, rand()%20/10.0);
            make_init(item.best_list, item.best_cost, rand()%20/10.0);


//
//            printf("个体i=%d %d\n", i, item.best_cost);
//            for(int j=0; j<global->netNodeNum; j++) {
//                printf("%d ", item.v[j]);
//            }
//            printf("\n");
//            for(int j=0; j<global->netNodeNum; j++) {
//                printf("%d ", item.video_list[j]);
//            }
//            printf("\n");

        }
//        getchar();
    }

    void assess_item(PSO_item& item){
        zkw->work(item.video_list);
        item.cost = zkw->ans_cost;


        for(int j=0; j<global->netNodeNum; j++) {
            if (item.video_list[j] != 0) {
                int vt = global->hashCapForType[item.video_list[j]];
                item.cost += global->buildCost[j] + global->videoCost[vt].cost;
            }
        }

        if(item.cost < item.best_cost){
            item.best_cost = item.cost;
            memcpy(item.best_list, item.video_list, sizeof(int)*global->netNodeNum);

            if(item.best_cost < Gcost){
                Gcost = item.best_cost;
                memcpy(Gvideo_list, item.best_list, sizeof(int)*global->netNodeNum);
            }
        }
    }

    void assess(){
        ZKW* zkw = ZKW::get();

        for(int i=0; i<item_num; i++){
            PSO_item& item = group[i];

            assess_item(item);

            printf("%d ", group[i].best_cost);
        }

        printf("\n");
    }

    int getId(int now_id){
        int num = rand()%(item_num/2);
        int mx = INF;
        int id = rand()%(item_num+1)-1;

        while(num--){
            int a = rand()%(item_num+1)-1;
            if(a == -1)
                return a;
            if(a == now_id)
                continue;
            if(mx > group[a].cost){
                mx = group[a].cost;
                id = a;
            }
        }
        return id;
    }

    void update(){

        for(int i=0; i<item_num; i++){
            PSO_item& item = group[i];
            int id = getId(i);
            for(int j=0; j<global->netNodeNum; j++){
                item.v[j] *= w;

                if(id == -1) {
                    item.v[j] += int(c1 * frand(1) * (item.best_list[j] - item.video_list[j])
                                     + c2 * frand(1) * (Gvideo_list[j] - item.video_list[j]));
                }
                else{
                    item.v[j] += int(c1 * frand(1) * (item.best_list[j] - item.video_list[j])
                                     + c2 * frand(1) * (group[id].video_list[j] - item.video_list[j]));
                }
//                item.v[j] += c2*frand(1)*(Gvideo_list[j]-item.video_list[j]);

                item.v[j] = std::max(item.v[j], -global->capSum[j]);
                item.v[j] = std::min(item.v[j], global->capSum[j]);

                item.video_list[j] += r*item.v[j];

                item.video_list[j] = std::max(0, item.video_list[j]);
                item.video_list[j] = std::min(global->capSum[j], item.video_list[j]);
            }
//            printf("个体i=%d %d\n", i, item.cost);
//            for(int j=0; j<global->netNodeNum; j++) {
//                printf("%d ", item.v[j]);
//            }
//            printf("\n");
//            for(int j=0; j<global->netNodeNum; j++) {
//                printf("%d ", item.video_list[j]);
//            }
//            printf("\n");
        }
//        getchar();
    }

    void work(){

        for(int i=0; i<iterations; i++) {

            assess();

            int time = global->get_time()-global->start_time;

            printf("第%d代, cost=%d time=%d==\n", i, Gcost, time);

            if (global->get_time() - global->start_time > 200)
                break;

            update();

            for(int i=0; i<item_num; i++)
                bianyi(group[i]);
        }
    }

    void bianyi(PSO_item& item){

        for(int t=0; t<5; t++) {

            PSO_item temp = item;

            int p;
            do {
                p = rand() % global->netNodeNum;
            } while (temp.video_list[p] == 0);

            int q = rand() % global->graph[p].size();

            q = global->graph[p][q];
            temp.video_list[p] = 0;
            temp.video_list[q] = global->capSum[q];
            //        temp.video_list[q] = std::max(0, temp.video_list[q]);
            assess_item(temp);

            if (temp.cost < item.cost) {
                group[rand()%item_num] = item;
                item = temp;
//                if (item.cost < best_item.cost) {
//                    best_item = item;
//                }
            }
        }
    }
};


#endif //CDN_PSO_H
