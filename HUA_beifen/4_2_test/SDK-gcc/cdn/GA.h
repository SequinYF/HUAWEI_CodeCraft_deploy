//
// Created by shiyi on 2017/4/1.
//

#ifndef CDN_GA_H
#define CDN_GA_H

#include "Global.h"

struct GA_ITEM{

    bool video_list[NODE_MAXNUM];
    int cost;
    int flow;
    int video_num;

    bool operator<(const GA_ITEM& t) const
    {
        return this->cost < t.cost;
    }

    void init();

    bool check();

    void assess();

    void variation(int f, GA_ITEM& first);


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

    GA(int node_num)
    {
        GA_ITERATIONS = 10000;
        GA_VATRIATION = 100;

        if(node_num < 200)
        {
            GA_ITEM_NUM = 40;

        }
        else if(node_num < 500){
            GA_ITEM_NUM = 40;

        }
        else if(node_num < 1000){
            GA_ITEM_NUM = 40;

        }

        group = agroup;
        new_group = bgroup;
    }

    void init(bool* v, int value);

    void assess(){

        for(int i=0; i<GA_ITEM_NUM; i++)
            group[i].assess();
    }

    void variation();

    int getId(int sum, int len);

    void cross_item(GA_ITEM& boy, GA_ITEM& girl, GA_ITEM& father, GA_ITEM& mother);

    void cross();

    void sort(){
        std::sort(group, group+GA_ITEM_NUM);
    }

    void work();
};


#endif //CDN_GA_H
