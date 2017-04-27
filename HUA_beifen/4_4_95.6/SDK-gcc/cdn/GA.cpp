//
// Created by shiyi on 2017/4/1.
//

#include <cstdlib>
#include "GA.h"
#include "ZKW.h"
#include "TZKW.h"

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

    if(this->flow != global->sumFlow)
        this->cost = INF;

//    Global::get()->get_time();
//    printf("-=-==-=end\n");
}

void GA_ITEM::variation1(int f) {
    int num = rand() % 100;
    if (num > f)
        return;

    num = rand() % Global::get()->netNodeNum;
    this->video_list[num] ^= 1;

    this->assess();
}

void GA_ITEM::variation2(int f) {
    int num = rand() % 100;
    if (num > f)
        return;
    int cnt = rand()%8;
    while(cnt--) {
        num = rand() % Global::get()->netNodeNum;
        this->video_list[num] ^= 1;
    }
    this->assess();
}

//--------------------------------------

void GA::variation(){
//    if(GA_VARITION_FLAG)
        for(int i=1; i<GA_ITEM_NUM; i++)
            group[i].variation1(1000);
//    else
//        for(int i=1; i<GA_ITEM_NUM; i++)
//            group[i].variation2(1000);
}

void GA::cross_item(GA_ITEM &boy, GA_ITEM &girl, GA_ITEM &father, GA_ITEM &mother) {
    Global* global = Global::get();

    int left = rand()%global->netNodeNum;
    int right = rand()%global->netNodeNum;
    if(left > right)
        std::swap(left, right);

    for(int i=0; i<left; i++){

        boy.video_list[i] = father.video_list[i];
        girl.video_list[i] = mother.video_list[i];
    }
    for(int i=left; i<=right; i++){
        boy.video_list[i] = mother.video_list[i];
        girl.video_list[i] = father.video_list[i];
    }
    for(int i=right+1; i<global->netNodeNum; i++){
        boy.video_list[i] = father.video_list[i];
        girl.video_list[i] = mother.video_list[i];
    }

//    boy.assess();
//    girl.assess();
}

int GA::getId1(int sum, int len){
    int num = rand()%sum;

    for(int i=0; i<GA_ITEM_NUM; i++){
        num -= group[len-1].cost+1-group[i].cost;
        if(num <= 0)
            return i;
    }
    return rand()%len;
}

int GA::getId2(int len){

    int num = rand()%GA_ITEM_NUM;
    int mx = INF;
    int id = rand()%len;
    while(num--){
        int a = rand()%len;
        if(mx > group[a].cost){
            mx = group[a].cost;
            id = a;
        }
    }
    return id;
}

void GA::cross() {
    int len = GA_ITEM_NUM/5;

    for(int i=len; i<GA_ITEM_NUM; i+=2){
        int fa = getId2(len);
        int mo;
        do{
            mo = getId2(len);
        }while(mo == fa);

        cross_item(group[i], group[i+1], group[fa], group[mo]);
    }
}

void GA::make_init(GA_ITEM& item, double p)
{
    Global* global = Global::get();
    TZKW* tzkw = new TZKW;
    tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
               global->netNodeNum + 1);

    for(int i=0; i<global->netNodeNum; i++) {
        if (global->hashNode[i] != -1) {
            tzkw->add_edge(global->netNodeNum, i, INF, global->videoCost*p / global->capSum[i]+0.5);
//            printf("%d %lf\n", (int)(global->videoCost*1.0/global->capSum[i]+0.5), global->videoCost*1.0/global->capSum[i]);
        }
    }

    tzkw->costflow();

    memset(item.video_list, 0, sizeof(item.video_list));

    for(int i=tzkw->head[tzkw->src]; i!=-1; i=tzkw->edge[i].next) {
        if(i&1)
            continue;
        if(tzkw->edge[i].cap != tzkw->edge[i].pre_cap) {
            item.video_list[tzkw->edge[i].v] = true;
        }
    }
}

void GA::init()
{
    Global* global = Global::get();

    for(int i=0; i<GA_ITEM_NUM; i++) {
        make_init(group[i], 3+10.0*(i+1)/GA_ITEM_NUM);
        group[i].assess();
    }
//    for(int i=1; i<GA_ITEM_NUM; i++) {
////        do {
//            group[i].init();
//            group[i].assess();
////        printf("init%d p=%lf  cost=%d flow=%d\n", i, 20.0*(i+1)/GA_ITEM_NUM, group[i].cost, group[i].flow);
////        } while (group[i].cost == INF);
//    }
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
        }

        ++flag[best_item.cost];

        if(flag[best_item.cost] % 20 == 0) {
            GA_ITEM_NUM += 10;
//            GA_VARITION_FLAG = false;
//            printf("灾变\n");
        }
//                printf("个体数：%d\n", GA_ITEM_NUM);
//        }
//            if(flag[best_item.cost]%10 == 0){
//                for(int k=GA_ITEM_NUM/4; k<GA_ITEM_NUM; k++)
//                    group[k].init();
//            }

        if(global->netNodeNum < 200 && best_iter+150 < i)
            break;

        if (global->get_time() - global->start_time > 86)
            break;


//        printf("第%d代： cost = %d group[1]: %d [%d] flow = %d time = %d\n", i + 1, best_item.cost, group[1].cost, flag[best_item.cost], best_item.flow, global->get_time() - global->start_time);
//
        cross();
//        std::swap(group, new_group);
        variation();
    }

//        printf("第1： cost = %d  flow = %d\n", group[0].cost, group[0].flow);
//        printf("第2： cost = %d  flow = %d\n", group[1].cost, group[1].flow);
//        printf("第3： cost = %d  flow = %d\n", group[2].cost, group[2].flow);
}
