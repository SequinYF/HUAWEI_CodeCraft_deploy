//
// Created by shiyi on 2017/4/10.
//

#ifndef CDN_ACO_H
#define CDN_ACO_H

#include "Global.h"
#include "ZKW.h"
#include "TZKW.h"

struct Ant {

    int video_list[NODE_MAXNUM];
    int cost;

    bool operator<(const Ant& t) const
    {
        return this->cost < t.cost;
    }
};


struct ACO {
    Global* global;
    ZKW* zkw;
    int ant_num;
    int iterations;
    double Q;
    double alpha;
    double pMax;
    double pMin;
    double pheromone[NODE_MAXNUM][MAX_TYPE];
    Ant best_ant;
    Ant group[1000];

    ACO(){
        Q = 1000;
        iterations = 10000;
        global = Global::get();
        zkw = ZKW::get();
        ant_num = 10;
        alpha = 0.8;
        pMax = 1000000;
        pMin = 10;
    }

    void make_init(Ant& item, double p){
        int video_list[NODE_MAXNUM];

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

        item.cost = INF;

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
                    nodeCost[e.v] = (global->buildCost[e.v] + global->videoCost[vt].cost) * 1.0 / (e.pre_cap - e.cap);
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
            printf("%d == %d\n", zkw->ans_cost, cost);

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

    int getId(int id){
        Global* global = Global::get();
        double sum = 0;
        for(int i=0; i<=global->nodeMaxLevel[id]; i++)
            sum += pheromone[id][i];
        double r = rand()%10001*1.0/10000.0 * sum;
        for(int i=0; i<=global->nodeMaxLevel[id]; i++){
            r -= pheromone[id][i];
            if(r < 0)
                return i;
        }
        return global->nodeMaxLevel[id];
    }

    void init_item(Ant& item){
        for(int i=0; i<global->netNodeNum; i++){
            item.video_list[i] = getId(i);
        }
    }

    void init(){
        for(int i=0; i<global->netNodeNum; i++)
            for(int j=0; j<global->videoMaxLevel; j++)
                pheromone[i][j] = Q;

        make_init(group[0], 1);
        for(int i=1; i<global->netNodeNum; i++)
            init_item(group[i]);
    }

    void assess(Ant& item){
        zkw->work(item.video_list);

        item.cost = zkw->ans_cost;
        for(int i=0; i<global->netNodeNum; i++) {

            if (item.video_list[i] != 0) {
                item.cost += global->buildCost[i];
                item.cost += global->videoCost[item.video_list[i]-1].cost;
            }
        }
    }

    void update(){
        int min_cost = group[0].cost;
        int mi = 0;
        for(int i=1; i<ant_num; i++){
            if(min_cost > group[i].cost){
                min_cost = group[i].cost;
                mi = i;
            }
        }

        if(best_ant.cost > group[mi].cost)
            best_ant = group[mi];

        double sum = 0;
        for (int i = 0; i < global->netNodeNum; i++)
            sum += group[i].cost;

        sum /= global->netNodeNum;

        for(int i=0; i<global->netNodeNum; i++) {
            for (int j = 0; j < global->videoMaxLevel; j++) {
                pheromone[i][j] *= 0.5;
                pheromone[i][j] = std::max(pheromone[i][j], pMin);
            }
            pheromone[i][group[mi].video_list[i]] += Q/group[i].cost;
            pheromone[i][group[mi].video_list[i]] = std::min(pheromone[i][group[mi].video_list[i]], pMax);
        }
    }

    void work(){
        best_ant.cost = INF;

        for(int c=0; c<iterations; c++) {
            for(int i=0; i<global->netNodeNum; i++)
                assess(group[i]);

            std::sort(group, group+ant_num);

            for(int i=0; i<ant_num; i++)
                printf("%d ", group[i].cost);
            printf("\n");

            update();

            printf("%d代 cost=%d\n", c, best_ant.cost);

            for(int i=1; i<ant_num; i++)
                init_item(group[i]);
        }
    }
};


#endif //CDN_ACO_H
