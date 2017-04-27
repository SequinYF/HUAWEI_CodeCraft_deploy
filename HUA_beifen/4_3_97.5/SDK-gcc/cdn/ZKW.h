//
// Created by shiyi on 2017/3/31.
//

#ifndef CDN_ZKW_H
#define CDN_ZKW_H

#include "Global.h"

struct ZKW {

    int video_num;
    int ans_cost;
    int ans_flow;
    int edge_num;
    int node_num;
    int src;
    int des;
    EDGE* edge;
    int* head;   //别忘记初始化
    int dis[NODE_MAXNUM];
    bool vis[NODE_MAXNUM];
    int in[NODE_MAXNUM];
    int pre_node[NODE_MAXNUM];
    int pre_edge[NODE_MAXNUM];


    static ZKW* get(){
        static ZKW instance;
        return &instance;
    }

    void init(EDGE* edge, int edge_num, int node_num, int* head, int src, int des);

    void work(bool* video_list);

    void prepare(bool* video_list);

    int spfa();

    void clear();

    int dfs(int now_node, int max_f);

    bool change();

    void costflow();

};


#endif //CDN_ZKW_H
