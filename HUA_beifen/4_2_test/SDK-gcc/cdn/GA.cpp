//
// Created by shiyi on 2017/4/1.
//

#include <cstdlib>
#include "GA.h"
#include "ZKW.h"

void GA_ITEM::init() {

    for (int i = 0; i < Global::get()->netNodeNum; i++) {
        this->video_list[i] = rand() % 100 > 50 ? 1 : 0;
    }

}

bool GA_ITEM::check()
{
    Global* global = Global::get();
    int flow = 0;
    for(int i=0; i<global->netNodeNum; i++)
    {
        flow += global->capSum[i];
    }

    return flow >= global->sumFlow;
}

void GA_ITEM::assess(){
//    printf("-=-=-=begin\n");
//    Global::get()->get_time();

    ZKW* zkw = ZKW::get();
    Global* global = Global::get();

    zkw->work(this->video_list);

    this->cost = zkw->ans_cost;
    this->flow = zkw->ans_flow;
    this->video_num = zkw->video_num;

    if(this->flow != global->sumFlow)
        this->cost = INF;

//    Global::get()->get_time();
//    printf("-=-==-=end\n");
}

void GA_ITEM::variation(int f, GA_ITEM& first) {
    int num = rand() % 100;
    if (num > f)
        return;

    memcpy(this->video_list, first.video_list, sizeof(first.video_list));

    do{
        num = rand() % Global::get()->netNodeNum;
    }while(this->video_list[num]);
    this->video_list[num] = false;
}

//--------------------------------------

void GA::variation() {
    int len = GA_ITEM_NUM / 4;

    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += group[len - 1].cost + 1 - group[i].cost;
    }

    for (int i = len; i < GA_ITEM_NUM; i++) {
        int a = getId(sum, len);
        group[i].variation(100, group[a]);
    }
}

void GA::cross_item(GA_ITEM &boy, GA_ITEM &girl, GA_ITEM &father, GA_ITEM &mother) {
    Global* global = Global::get();

    int num = global->netNodeNum;
    int cnt = 0;

    for(int i=0; i<num; i++){
        if(father.video_list[i] && mother.video_list[i]) {
            boy.video_list[i] = true;
            girl.video_list[i] = true;
            cnt++;
        }
        else if(!(father.video_list[i] || mother.video_list[i])) {
            boy.video_list[i] = false;
            girl.video_list[i] = false;
        }
    }

    for(int i=cnt; i<global->maxVideoNum; i++){
        int a;
        do{
            a = rand()%num;
        }while(boy.video_list[a]);
        boy.video_list[a] = true;

        do{
            a = rand()%num;
        }while(girl.video_list[a]);
        girl.video_list[a] = true;

    }
    //可以优化  提前判定
    boy.cost = girl.cost = INF;
}


int GA::getId(int sum, int len){
    int num = rand()%sum;

    for(int i=0; i<GA_ITEM_NUM; i++){
        num -= group[len-1].cost+1-group[i].cost;
        if(num <= 0)
            return i;
    }
    return 0;
}

void GA::cross() {
    int len = GA_ITEM_NUM/4;

    int sum = 0;
    for(int i=0; i<len; i++){
        sum += group[len-1].cost+1-group[i].cost;
    }

    for(int i=len; i<GA_ITEM_NUM; i+=2){
        int fa = getId(sum, len);
        int mo;
        do{
            mo = getId(sum, len);
        }while(mo == fa);

        cross_item(group[i], group[i+1], group[fa], group[mo]);
    }
}

void GA::init(bool* v, int value)
{
    Global* global = Global::get();

    if(global->netNodeNum > 500) {
        memcpy(group[0].video_list, v, sizeof(bool)*NODE_MAXNUM);
    }
    else if(global->netNodeNum > 200){
        memcpy(group[0].video_list, v, sizeof(bool)*NODE_MAXNUM);
    } else{
//            group[0].init();
        memcpy(group[0].video_list, v, sizeof(bool)*NODE_MAXNUM);
    }

    group[0].cost = value;
    group[0].video_num = global->maxVideoNum;

    for(int i=1; i<GA_ITEM_NUM; i++){
        do{
            group[i].init();
            group[i].assess();
        }while(group[i].cost == INF);
    }
}

void GA::work() {
    Global* global = Global::get();

    best_item.cost = INF;
    best_iter = 0;

    for (int i = 0; i < GA_ITERATIONS; ++i) {

        sort();

        if (best_item.cost > group[0].cost) {
            best_item = group[0];
            best_iter = i;
            global->maxVideoNum = std::min(global->maxVideoNum, best_item.video_num);
        }

        ++flag[best_item.cost];
        if(flag[best_item.cost] % 10 == 0){
            GA_ITEM_NUM += 10;
//                printf("个体数：%d\n", GA_ITEM_NUM);
        }
//            if(flag[best_item.cost]%10 == 0){
//                for(int k=GA_ITEM_NUM/4; k<GA_ITEM_NUM; k++)
//                    group[k].init();
//            }

//        if(global->netNodeNum < 200 && best_iter+150 < i)
//            break;

        if (global->get_time() - global->start_time > 86)
            break;


        printf("第%d代： cost = %d  num=%d  group[0]: %d [%d] flow = %d time = %d\n", i + 1, best_item.cost, global->maxVideoNum, group[0].cost, flag[best_item.cost], best_item.flow, global->get_time() - global->start_time);

        cross();
//        std::swap(group, new_group);
        variation();

        assess();
    }

//        printf("第1： cost = %d  flow = %d\n", group[0].cost, group[0].flow);
//        printf("第2： cost = %d  flow = %d\n", group[1].cost, group[1].flow);
//        printf("第3： cost = %d  flow = %d\n", group[2].cost, group[2].flow);
}
