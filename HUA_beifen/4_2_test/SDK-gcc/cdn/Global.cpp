//
// Created by shiyi on 2017/3/31.
//

#include <sys/timeb.h>
#include "Global.h"
#include "ZKW.h"
#include "Dijkstra.h"

void Global::init()
{
    start_time = get_time();

    memset(hashNode, -1, sizeof(hashNode));
    memset(head, -1, sizeof(head));
    memset(capSum, 0, sizeof(capSum));
    sumFlow = edgeNum = 0;

}

void Global::input_file(char* topo[MAX_EDGE_NUM], int line_num)
{
    //当前行
    int line_id = 0;

    //读取网络节点个数，链路个数，消费节点个数
    sscanf(topo[line_id++], "%d %d %d", &netNodeNum, &netEdgeNum, &costNodeNum);
    ++line_id;

    //读取视频内容服务器部署成本
    sscanf(topo[line_id++], "%d", &videoCost);
    ++line_id;

    Dijkstra* dijk = Dijkstra::get();
    dijk->init(netNodeNum);

    //读取链路信息
    for(int i=0; i<netEdgeNum; ++i) {
        int start, end, cap, cost;
        sscanf(topo[line_id++], "%d %d %d %d", &start, &end, &cap, &cost);
        capSum[start] += cap;
        capSum[end] += cap;
        add_edge(start, end, cap, cost);
        add_edge(end, start, cap, cost);

        dijk->addEdge(start, end, cap, cost);
        dijk->addEdge(end, start, cap, cost);
    }
    ++line_id;

    maxVideoNum = costNodeNum;

    //读取消费节点链路信息
    for(int i=0; i<costNodeNum; ++i)
    {
        int cost_id, node_id, need;
        sscanf(topo[line_id++], "%d %d %d", &cost_id, &node_id, &need);
        costEdge[i].cost_id = cost_id;
        costEdge[i].node_id = node_id;
        costEdge[i].flow_need = need;

        //设置超级汇
        add_edge(node_id, netNodeNum+1, need, 0);
        sumFlow += need;
        hashNode[node_id] = cost_id;
    }

    memcpy(tedge, edge, sizeof(tedge));
    memcpy(thead, head, sizeof(thead));
    tedgeNum = edgeNum;

    //设置超级源
    for(int i=0; i<netNodeNum; i++)
    {
        add_edge(netNodeNum, i, INF, MAX_COST);
    }
}

int Global::dfs_getPath(int from, int depth, int cap)
{
    ZKW* zkw = ZKW::get();

    dfs_tmp[depth] = from;

    if(from == zkw->des)
    {
        for(int i=1; i<depth; ++i){
            routeList[routeListNum].push_back(dfs_tmp[i]);
        }
        routeList[routeListNum].push_back(hashNode[dfs_tmp[depth-1]]);
        routeList[routeListNum].push_back(cap);
        ++routeListNum;

        now_flow += cap;
        return cap;
    }
    dfs_vis[from] = true;

    int ret = 0;
    for(int i=zkw->head[from]; i!=-1; i=zkw->edge[i].next){
        int to = zkw->edge[i].v;
        if(!dfs_vis[to] && i^1 && zkw->edge[i].pre_cap > 0) {
            ret = dfs_getPath(to, depth + 1, std::min(cap, zkw->edge[i].pre_cap));
            zkw->edge[i].pre_cap -= std::min(ret, zkw->edge[i].pre_cap);
            if(ret != 0)
                return ret;
        }
    }
    dfs_vis[from] = false;

    return ret;
}

void Global::out_goodResult(bool* video_list) {
    ZKW* zkw = ZKW::get();

    now_flow = 0;
    routeListNum = 0;
    resultLen = 0;

    for (int i = 0; i < zkw->edge_num; i++) {
        zkw->edge[i].pre_cap -= zkw->edge[i].cap;
    }

    memset(dfs_vis, 0, sizeof(dfs_vis));
    while (dfs_getPath(zkw->src, 0, INF)) {
        memset(dfs_vis, 0, sizeof(dfs_vis));
    }

    resultLen += sprintf(resultStr + resultLen, "%d\n\n", routeListNum);
    for (int i = 0; i < routeListNum; ++i) {
        int len = routeList[i].size();
        for (int j = 0; j < len - 1; ++j) {
            resultLen += sprintf(resultStr + resultLen, "%d ", routeList[i][j]);
        }
        resultLen += sprintf(resultStr + resultLen, "%d\n", routeList[i][len - 1]);
    }
    resultStr[resultLen - 1] = '\0';

}

void Global::out_badResult()
{
    resultLen += sprintf(resultStr + resultLen, "%d\n\n", costNodeNum);

    for(int i=0; i<costNodeNum; ++i){
        resultLen += sprintf(resultStr + resultLen, "%d %d %d\n",
                               costEdge[i].node_id, costEdge[i].cost_id, costEdge[i].flow_need);
    }
    resultStr[resultLen-1] = '\0';
}


int Global::get_time()
{
    struct timeb rawtime;
    struct tm * timeinfo;
    ftime(&rawtime);
//    timeinfo = localtime(&rawtime.time);

    int ms = rawtime.millitm;
    unsigned long s = rawtime.time;

//    printf("date/time is: %s \tused time is %lu s %d ms.\n", asctime(timeinfo), s, ms);

    return s;
}