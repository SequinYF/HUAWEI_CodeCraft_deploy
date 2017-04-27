//
// Created by shiyi on 2017/4/11.
//

#ifndef NetWorkSimplex_H
#define NetWorkSimplex_H

#include <stdio.h>
#include <iostream>
#include <vector>
#include <assert.h>
#include <string.h>

#define INF                 999999
#define MAX_COST            1000
#define NODE_MAXNUM         10009            //网络节点数量最大值
#define EDGE_MAXNUM         300009           //最大链路数量
#define COST_MAXNUM         10009             //消费节点数量最大值

struct Arc
{
    int start, end;
    int lowerCap, upperCap;
    int cost, flow;
    bool isInTree;

    void init(int st, int ed, int low, int upp, int c, int f, bool isInT){
        this->start = st;
        this->end = ed;
        this->lowerCap = low;
        this->upperCap = upp;
        this->cost = c;
        this->flow = f;
        this->isInTree = isInT;
    }
};

struct SimplexTree
{
    int depth[NODE_MAXNUM];
    int succ[NODE_MAXNUM];
    int pred[NODE_MAXNUM];
    int arcind[NODE_MAXNUM];
};

struct NetWorkSimplex {

    int node_num;
    int arc_num;
    int real_arc_num;
    int demand[NODE_MAXNUM];
    int nodePrice[NODE_MAXNUM];
    Arc arcArr[EDGE_MAXNUM];
    SimplexTree tree;

    NetWorkSimplex(){}

    NetWorkSimplex(int n, int m){
        this->node_num = n;
        this->arc_num = m;
        this->real_arc_num = arc_num - node_num + 1;
    }

    static NetWorkSimplex* get(){
        static NetWorkSimplex* instance = new NetWorkSimplex;
        return instance;
    }

    int maxCost(){
        int mx = 0;
        for(int i=0; i<real_arc_num; i++)
            mx = std::max(arcArr[i].cost, mx);
        return mx;
    }

    void init(int n, int m, int* t_demand){
        this->node_num = n;
        this->arc_num = m;
        this->real_arc_num = arc_num - node_num + 1;

        memcpy(demand, t_demand, sizeof(demand));

        initTree();
        initFlowPrice();

    }

    void initTree(){
        //        memset(&tree, 0, sizeof(tree));

        tree.succ[0] = 1;
        for(int i=1; i<this->node_num; i++){
            tree.depth[i] = 1;
            tree.pred[i] = 0;
            tree.succ[i] = i+1;
            tree.arcind[i-1] = real_arc_num+i-1;
        }
        tree.succ[node_num-1] = 0;
    }

    void initFlowPrice(){
        //        memset(nodePrice, 0, sizeof(nodePrice));
        int tempFlow[NODE_MAXNUM];
        memcpy(tempFlow, demand, sizeof(demand));
        for(int i=0; i<real_arc_num; i++){
            tempFlow[arcArr[i].start] -= arcArr[i].lowerCap;
            tempFlow[arcArr[i].end] += arcArr[i].lowerCap;
        }

        int m = 1+node_num*maxCost();
        for(int i=1; i<node_num; i++){
            if(tempFlow[i] < 0){
                arcArr[real_arc_num+i-1].init(0,i,0,INF,m,-tempFlow[i],true);
                nodePrice[i] = m;
            }else{
                arcArr[real_arc_num+i-1].init(i,0,0,INF,m,tempFlow[i],true);
                nodePrice[i] = -m;
            }
        }
    }

    Arc* findTreeArc(int x, int y) {
        for(int i=0;i<node_num-1;++i){
            Arc* temparc = &arcArr[tree.arcind[i]];

            if((temparc->start==x && temparc->end==y) || (temparc->end==x && temparc->start==y)){
                return temparc;
            }
        }
        return NULL;
    }

    int findPivot(){
        int eps = 0;
        int pivotIndex = -1;
        for(int i=0; i<arc_num; i++){
            Arc* temp = &arcArr[i];

            if(temp->isInTree)
                continue;
            int redcost = temp->cost-nodePrice[temp->end]+nodePrice[temp->start];
            if(temp->flow == temp->upperCap && redcost > 0){
                if(eps < redcost)
                {
                    eps = redcost;
                    pivotIndex = i;
                }
            }
            if(temp->flow == temp->lowerCap && redcost < 0){
                if(eps < -redcost)
                {
                    eps = -redcost;
                    pivotIndex = i;
                }
            }
        }
        return pivotIndex;
    }

    bool simplex(){

        int pivot = findPivot();
        printf("%d %d %d\n", node_num, arc_num, real_arc_num);

        while(pivot != -1){
            Arc* pivotArc = &arcArr[pivot];

            int u, v=0;

            bool uDirection = true;
            int st = pivotArc->start;
            int ed = pivotArc->end;

            if(tree.depth[st] > tree.depth[ed]){
                u = st;
                v = ed;
                if(pivotArc->flow == pivotArc->upperCap)
                    uDirection = false;
            }
            else {
                v = st;
                u = ed;
                if(pivotArc->flow == pivotArc->lowerCap)
                    uDirection = false;
            }

            printf("%d %d %d %d %d\n", pivot, u, v, st, ed);

            std::vector<int> upath, vpath;
            upath.push_back(u);
            vpath.push_back(v);

            printf("%d %d\n", upath.size(), vpath.size());

            while(tree.depth[upath[upath.size()-1]] != tree.depth[v]){
                upath.push_back(tree.pred[upath[upath.size()-1]]);
            }

            printf("%d %d\n", upath.size(), vpath.size());

            while(upath[upath.size()-1] != vpath[0]){
                upath.push_back(tree.pred[upath[upath.size()-1]]);
                vpath.insert(vpath.begin(), tree.pred[vpath[0]]);
            }

            printf("%d %d\n", upath.size(), vpath.size());

            for(int a:upath)
                printf("upath %d\n", a);
            for(int a:vpath)
                printf("vpath %d\n", a);

            std::vector<Arc*> uArcs, vArcs;

            for(int i=0; i<upath.size()-1; i++){
                Arc* t = findTreeArc(upath[i], upath[i+1]);
                assert(t != NULL);
                uArcs.push_back(t);
            }

            for(int i=0; i<vpath.size()-1; i++){
                Arc* t = findTreeArc(vpath[i], vpath[i+1]);
                assert(t != NULL);
                vArcs.push_back(t);
            }

            vArcs.push_back(pivotArc);

            int pivptIndex = vArcs.size()-1;
            for(int i=0; i<uArcs.size(); i++)
                vArcs.push_back(uArcs[i]);
            for(int i=0; i<upath.size(); i++)
                vpath.push_back(upath[i]);

            int eps = 0x3f3f3f3f;
            Arc* f = NULL;
            int e1, e2, f1, f2;
            e1 = e2 = f1 = f2 = -1;

            bool isForwardArc[NODE_MAXNUM];


            for(int i=0; i<vArcs.size(); i++){
                Arc* tmp = vArcs[i];

                if(uDirection){
                    if(tmp->end == vpath[i]){
                        isForwardArc[i] = true;
                        eps = std::min(eps, tmp->upperCap-tmp->flow);
                    }else{
                        isForwardArc[i] = false;
                        eps = std::min(eps, tmp->flow-tmp->lowerCap);
                    }
                }else {
                    if(tmp->end == vpath[i+1]){
                        isForwardArc[i] = true;
                        eps = std::min(eps, tmp->upperCap-tmp->flow);
                    }else{
                        isForwardArc[i] = false;
                        eps = std::min(eps, tmp->flow-tmp->lowerCap);
                    }
                }
            }

            if(uDirection){
                for(int i=0; i<vArcs.size(); i++){
                    Arc* tmp = vArcs[i];

                    if((isForwardArc[i] && tmp->flow+eps==tmp->upperCap)
                       || (!isForwardArc[i] && tmp->flow==tmp->lowerCap+eps)){
                        f = tmp;
                        if(i > pivptIndex){
                            e2 = u;
                            e1 = v;
                        }else {
                            e2 = v;
                            e1 = u;
                        }
                        break;
                    }
                }
            }else{
                for(int i=vArcs.size()-1; i>=0; i--){
                    Arc* tmp = vArcs[i];

                    if((isForwardArc[i] && tmp->flow+eps==tmp->upperCap)
                       || (!isForwardArc[i] && tmp->flow==tmp->lowerCap+eps)){
                        f = tmp;
                        if(i < pivptIndex){
                            e2 = v;
                            e1 = u;
                        }else {
                            e2 = u;
                            e1 = v;
                        }
                        break;
                    }
                }
            }

            if(tree.depth[f->start] > tree.depth[f->end]){
                f2 = f->start;
                f1 = f->end;
            }else{
                f2 = f->end;
                f1 = f->start;
            }
            assert(e1!=-1 && e2!=-1 && f1!=-1 && f2!=-1);
            printf("%d %d %d %d\n", e1, e2, f1, f2);

            int pivotDir;
            if(pivotArc->end == e2)
                pivotDir = 1;
            else
                pivotDir = -1;

            int redCost = pivotArc->cost + nodePrice[pivotArc->start] - nodePrice[pivotArc->end];
            int y = f2;
            nodePrice[y] += pivotDir*redCost;
            while(tree.depth[tree.succ[y]]>tree.depth[f2]){
                y = tree.succ[y];
                nodePrice[y] += pivotDir*redCost;
            }


            for(int i=0; i<vArcs.size(); i++){
                Arc* tmp = vArcs[i];
                if(isForwardArc[i])
                    tmp->flow += eps;
                else
                    tmp->flow -= eps;
            }

            int tmpDepth[NODE_MAXNUM];
            int tmpPre[NODE_MAXNUM];
            memcpy(tmpDepth, tree.depth, sizeof(tmpDepth));
            memcpy(tmpPre, tree.pred, sizeof(tmpPre));

            tmpDepth[e2] = tmpDepth[e1]+1;
            tmpPre[e2] = e1;

            int a = f1;
            int b = tree.succ[e1];
            int i = e2;
            while(tree.succ[a]!=f2){
                a=tree.succ[a];
            }
            int k=i;
            while(tree.depth[tree.succ[k]]>tree.depth[i]){
                k=tree.succ[k];
                tmpDepth[k]=tmpDepth[tree.pred[k]]+1;
            }


            int r = tree.succ[k];

            while(i!=f2){
                int j=i;
                i=tree.pred[i];
                tmpPre[i]=j;
                tmpDepth[i]=tmpDepth[j]+1;

                tree.succ[k]=i;
                k=i;

                while(tree.succ[k]!=j){
//                    printf("%d %d %d\n", k, tree.succ[k], j);

                    k=tree.succ[k];
                    tmpDepth[k]=tmpDepth[tree.pred[k]]+1;
                }

                if(tree.depth[r]>tree.depth[i]){
                    tree.succ[k]=r;
                    while(tree.depth[tree.succ[k]]>tree.depth[i]){
                        k=tree.succ[k];
                        tmpDepth[k]=tmpDepth[tree.pred[k]]+1;
                    }
                    r=tree.succ[k];
                }
            }

            if(e1!=a){
                tree.succ[a]=r;
                tree.succ[e1]=e2;
                tree.succ[k]=b;
            }else{
                tree.succ[e1]=e2;
                tree.succ[k]=r;
            }

            memcpy(tree.depth, tmpDepth, sizeof(tmpDepth));
            memcpy(tree.pred, tmpPre, sizeof(tmpPre));

            pivotArc->isInTree = true;
            f->isInTree = false;

            for(int j=0; j<node_num-1; j++){
                Arc* tmp = &arcArr[tree.arcind[j]];

                if(f->start == tmp->start && f->end == tmp->end){
                    tree.arcind[j] = pivot;
                    break;
                }
            }

            pivot = findPivot();
        }

        for(int i=0; i<node_num-1; i++){
            if(arcArr[real_arc_num+i].isInTree && arcArr[real_arc_num+i].flow != 0)
                return false;
        }

        return true;
    }

    void printCost(){
        int cost = 0, flow = 0;
        for(int i=0; i<real_arc_num; i++) {
//            printf("-===%d:%d %d %d %d %d\n", i, arcArr[i].start, arcArr[i].end, arcArr[i].cost, arcArr[i].flow, demand[arcArr[i].end]);

            cost += arcArr[i].cost * arcArr[i].flow;
            if(demand[arcArr[i].start] < 0)
                flow -= arcArr[i].flow;
            if(demand[arcArr[i].end] < 0)
                flow += arcArr[i].flow;
        }
        printf("the min cost = %d     the flow = %d\n", cost, flow);
    }


};

#endif //CDN_NETWORKSIMPLEX_H
