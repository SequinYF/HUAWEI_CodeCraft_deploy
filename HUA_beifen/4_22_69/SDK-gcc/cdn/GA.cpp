//
// Created by shiyi on 2017/4/1.
//

#include <cstdlib>
#include "GA.h"
#include "ZKW.h"
#include "TZKW.h"

void GA_ITEM::init() {
    Global* global = Global::get();
    for (int i = 0; i < Global::get()->netNodeNum; i++) {
        if(rand()%2){
            video_list[i] = rand()%global->capSum[i];
        }
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
    Global* global = Global::get();
    ZKW* zkw = ZKW::get();

    zkw->work(video_list);
    cost = zkw->ans_cost;

    for(int j=0; j<global->netNodeNum; j++) {
        if (video_list[j] != 0) {
            int vt = global->hashCapForType[video_list[j]];
            cost += global->buildCost[j] + global->videoCost[vt].cost;
        }
    }
}

void GA_ITEM::variation1(int f) {
    int num = rand() % 100;
    if (num > f)
        return;

    num = rand() % Global::get()->netNodeNum;
    int p = rand()%5-2;
    if(rand()&1) {
        this->video_list[num] = std::min(this->video_list[num]+p, Global::get()->nodeMaxLevel[num]+1);
    }else{
        this->video_list[num] = std::max(this->video_list[num]-p, 0);
    }

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

    for(int i=0; i<global->netNodeNum; i++) {
        boy.video_list[i] = father.video_list[i];
        girl.video_list[i] = mother.video_list[i];
        if (father.video_list[i] != mother.video_list[i]) {
            if (rand() & 1) {
                std::swap(boy.video_list[i], girl.video_list[i]);
            }
        }
    }

    boy.assess();
    girl.assess();
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
    int len = GA_ITEM_NUM;
    int sum = 0;
    for(int i=0; i<GA_ITEM_NUM; i++)
        sum += group[i].cost;

    new_group[0] = group[0];
    for(int i=1; i<GA_ITEM_NUM; i+=2){
        int fa = getId2(len);
        int mo;
        do{
            mo = getId2(len);
        }while(mo == fa);

        cross_item(new_group[i], new_group[i+1], group[fa], group[mo]);
    }
}

void GA::make_init1(GA_ITEM& item, double p)
{
    Global* global = Global::get();
    ZKW *zkw = ZKW::get();

    int v[NODE_MAXNUM];

    for (int i = 0; i < global->netNodeNum; i++)
        node_cost[i] = global->buildCost[i]*p/global->capSum[i]+0.5;

    int cnt = 13;
    int pre = 0;
    int videoCost = INF;
    while(cnt) {

        TZKW *tzkw = new TZKW;

        tzkw->init(global->tedge, global->tedgeNum, global->netNodeNum + 2, global->thead, global->netNodeNum,
                   global->netNodeNum + 1);

        for (int i = 0; i < global->netNodeNum; i++) {
            if (global->hashNode[i] != -1) {
                tzkw->add_edge(global->netNodeNum, i, INF, node_cost[i]);
            }
        }
        tzkw->costflow();

        memset(v, 0, sizeof(v));

        for (int i = tzkw->head[tzkw->src]; i != -1; i = tzkw->edge[i].next) {

            EDGE& e = tzkw->edge[i];

            if (tzkw->edge[i].cap != tzkw->edge[i].pre_cap) {
                int vt = global->hashCapForType[e.pre_cap - e.cap];
                v[e.v] = vt+1;
                node_cost[e.v] = (global->buildCost[e.v] + global->videoCost[vt].cost) * 1.0 / (e.pre_cap - e.cap);
            }
            else
                node_cost[e.v] = INF;

        }
        zkw->work(v);


        int cost = zkw->ans_cost;
        for(int i=0; i<global->netNodeNum; i++){
            if(v[i] != 0)
                cost += global->videoCost[v[i]-1].cost + global->buildCost[i];
        }
        printf("V%d %d\n", cost, zkw->ans_flow);
        global->get_time();

        if(videoCost > cost)
        {
            videoCost = cost;
            memcpy(item.video_list, v, sizeof(v));
        }
    }

    item.cost = zkw->ans_cost;
    for(int i=0; i<global->netNodeNum; i++){
        if(v[i] != 0)
            item.cost += global->videoCost[v[i]-1].cost + global->buildCost[i];
    }
}

void GA::make_init2(GA_ITEM& item, double p)
{
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

void GA::init()
{
    Global* global = Global::get();
    group[0].cost = INF;
//    if(global->netNodeNum > 500) {
        make_init2(group[0], 1);
    group[0].assess();
        for (int i = 1; i < GA_ITEM_NUM; i++) {
//            make_init2(group[i], rand() % 20 * 1.0 / 10);
            group[i].init();
            group[i].assess();
        }
//    }
//    else {
//        for(int i=0; i<GA_ITEM_NUM; i++) {
//            make_init1(group[i], 3+10.0*(i+1)/GA_ITEM_NUM);
//            group[i].assess();
//        }
//    }
//    for(int i=1; i<GA_ITEM_NUM; i++) {
//        do {
//            group[i].init();
//            group[i].assess();
////        printf("init%d p=%lf  cost=%d flow=%d\n", i, 20.0*(i+1)/GA_ITEM_NUM, group[i].cost, group[i].flow);
//        } while (group[i].cost == INF);
//    }
    for(int i=0; i<GA_ITEM_NUM; i++)
        printf("%d:%d\n", i, group[i].cost);
}

void GA::work() {
    Global* global = Global::get();

    best_item.cost = INF;
    best_iter = 0;

    for (int i = 0; i < GA_ITERATIONS; ++i) {

        sort();

//        for(int c=0; c<GA_ITEM_NUM; c++){
//            group[c].dst = 0;
//            for(int j=c; j<GA_ITEM_NUM; j++){
//                int t = 0;
//                for(int k=0; k<global->netNodeNum; i++){
//                    if(group[c].video_list[k] != group[c].video_list[k])
//                }
//            }
//        }

        if (best_item.cost > group[0].cost) {
            best_item = group[0];
            best_iter = i;
        }

        ++flag[best_item.cost];

//        if(flag[best_item.cost] % 10 == 0) {
//            GA_ITEM_NUM += 10;
//            GA_VARITION_FLAG = false;
//            printf("灾变\n");
//        }
//                printf("个体数：%d\n", GA_ITEM_NUM);
//        }
//            if(flag[best_item.cost]%10 == 0){
//                for(int k=GA_ITEM_NUM/4; k<GA_ITEM_NUM; k++)
//                    group[k].init();
//            }

//        if(global->netNodeNum < 200 && best_iter+150 < i)
//            break;

        if (global->get_time() - global->start_time > 86)
            break;


        printf("第%d代： cost = %d group[1]: %d [%d] time = %d\n", i + 1, best_item.cost, group[1].cost, flag[best_item.cost], global->get_time() - global->start_time);

        cross();
        std::swap(group, new_group);
//        variation();
    }

//        printf("第1： cost = %d  flow = %d\n", group[0].cost, group[0].flow);
//        printf("第2： cost = %d  flow = %d\n", group[1].cost, group[1].flow);
//        printf("第3： cost = %d  flow = %d\n", group[2].cost, group[2].flow);
}
