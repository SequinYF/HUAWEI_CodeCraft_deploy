//
// Created by shiyi on 2017/4/13.
//

#ifndef CDN_MYACO_H
#define CDN_MYACO_H

#include "Global.h"

struct Ant{
    int cost;
    int video_list[NODE_MAXNUM];
};

struct MyACO {
    int ant_num;
    int node_num;
    int iterations;
    Global* global;

    double evaporation_rate;
    double pheroomone_rate;
    double alpha = 1;
    double beta = 2;
    double pheromone[NODE_MAXNUM][MAX_TYPE];

    int getId(int id){
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

    void init_ant(Ant& ant) {
        ant.cost = 0;
        for (int i = 0; i < global->netNodeNum; i++) {
            ant.video_list[i] = getId(i);
            ant.cost += global->videoCost[ant.video_list[i] - 1].cost;
            if (ant.video_list[i] != 0)
                ant.cost += global->buildCost[i];
        }
    }


};


#endif //CDN_MYACO_H
