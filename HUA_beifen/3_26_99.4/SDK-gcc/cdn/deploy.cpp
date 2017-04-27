#include "deploy.h"
#include <stdio.h>

int GA::start_time;

EDGE* ITEM::edge;
int ITEM::edge_num;
int* ITEM::head;
int ITEM::node_num;
int ITEM::video_cost;
int ITEM::sum_flow;
int* ITEM::capSum;
CostEdge* ITEM::costEdge;
int ITEM::costNodeNum;

ZKW ZKW::zkw;

//输入数据
int G_netNodeNum;       					//网络节点数量
int G_netEdgeNum;       					//网络链路数量
int G_costNodeNum;      					//消费节点数量
int G_sumFlow;                              //总消费流量大小
EDGE G_edge[EDGE_MAXNUM];					//网络流初始邻接表
int G_edgeNum;								//初始邻接表边总个数
int G_head[NODE_MAXNUM];					//网络流邻接表头指针
CostEdge G_costEdge[COST_MAXNUM];			//保存消费节点信息
int G_videoCost;							//视频服务器部署成本
int G_hashNode[DATA_INF];					//hash消费节点虚拟id
int G_capSum[NODE_MAXNUM];                  //每个点最大流量

//输出数据
char G_resultStr[NODE_MAXNUM*COST_MAXNUM];  //保存最终结果
int G_resultLen;
vector<int> G_routeList[NODE_MAXNUM*22];
int G_routeListNum;

int dfs_tmp[NODE_MAXNUM];
bool dfs_vis[NODE_MAXNUM];
int now_flow;

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
//    srand(time(NULL));
    get_time();

    parse_file(topo, line_num);

    solve();

//    printf("[%s]\n", G_resultStr);
	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性，如果有解，第一行只有一个数据；第二行为空；第三行开始才是具体的数据，数据之间用一个空格分隔开)
	write_result(G_resultStr, filename);

}

void solve()
{
    init_ITEM();

    GA ga;
    ga.work();

    if(ga.best_item.cost == INF || ga.best_item.cost >= G_videoCost*G_costNodeNum) {
        out_badResult();
        return;
    }

    int cnt = 0;
    for(int i=0; i<G_netNodeNum; i++)
        if(ga.best_item.video_list[i])
            ++cnt;
    ZKW::zkw.init(G_edge, G_edgeNum, G_netNodeNum + 2, G_head, G_netNodeNum, G_netNodeNum + 1,
              ga.best_item.video_list);
    ZKW::zkw.costflow();

    ZKW::zkw.ans_cost += cnt*G_videoCost;

    out_goodResult(ga.best_item.video_list);

//    if(ZKW::zkw.ans_flow == G_sumFlow) {
//        printf("V:");
//    }else
//        printf("X:");
//    printf("cost=%d flow=%d sum_flow:%d now_flow:%d\n", ZKW::zkw.ans_cost, ZKW::zkw.ans_flow, G_sumFlow, now_flow);
}

//输入解析
void parse_file(char* topo[MAX_EDGE_NUM], int line_num) {

    memset(G_hashNode, -1, sizeof(G_hashNode));
    memset(G_head, -1, sizeof(G_head));
    memset(G_capSum, 0, sizeof(G_capSum));
    G_sumFlow = G_edgeNum = 0;

    //当前行
    int line_id = 0;

    //读取网络节点个数，链路个数，消费节点个数
    sscanf(topo[line_id++], "%d %d %d", &G_netNodeNum, &G_netEdgeNum, &G_costNodeNum);
    line_id++;

    //读取视频内容服务器部署成本
    sscanf(topo[line_id++], "%d", &G_videoCost);
    ++line_id;

    //读取链路信息
    for(int i=0; i<G_netEdgeNum; ++i) {
        int start, end, cap, cost;
        sscanf(topo[line_id++], "%d %d %d %d", &start, &end, &cap, &cost);
        G_capSum[start] += cap;
        G_capSum[end] += cap;
        add_edge(start, end, cap, cost);
        add_edge(end, start, cap, cost);
    }
    ++line_id;

    //读取消费节点链路信息
    for(int i=0; i<G_costNodeNum; ++i)
    {
        int cost_id, node_id, need;
        sscanf(topo[line_id++], "%d %d %d", &cost_id, &node_id, &need);
        G_costEdge[i].cost_id = cost_id;
        G_costEdge[i].node_id = node_id;
        G_costEdge[i].flow_need = need;

        //设置超级汇
        add_edge(node_id, G_netNodeNum+1, need, 0);
        G_sumFlow += need;

        G_hashNode[node_id] = cost_id;
    }
}

void add_edge(int u, int v, int cap, int cost)
{
    G_edge[G_edgeNum].v = v;
    G_edge[G_edgeNum].cap = cap;
    G_edge[G_edgeNum].pre_cap = cap;
    G_edge[G_edgeNum].cost = cost;
    G_edge[G_edgeNum].next = G_head[u];
    G_head[u] = G_edgeNum++;

    G_edge[G_edgeNum].v = u;
    G_edge[G_edgeNum].cap = 0;
    G_edge[G_edgeNum].cost = -cost;
    G_edge[G_edgeNum].next = G_head[v];
    G_head[v] = G_edgeNum++;
}

int dfs_getPath(int from, int depth, int cap)
{
    dfs_tmp[depth] = from;

    if(from == ZKW::zkw.des)
    {
        for(int i=1; i<depth; ++i){
            G_routeList[G_routeListNum].push_back(dfs_tmp[i]);
        }
        G_routeList[G_routeListNum].push_back(G_hashNode[dfs_tmp[depth-1]]);
        G_routeList[G_routeListNum].push_back(cap);
        ++G_routeListNum;

        now_flow += cap;
        return cap;
    }
    dfs_vis[from] = true;

    int ret = 0;
    for(int i=ZKW::zkw.head[from]; i!=-1; i=ZKW::zkw.edge[i].next){
        int to = ZKW::zkw.edge[i].v;
        if(!dfs_vis[to] && i^1 && ZKW::zkw.edge[i].pre_cap > 0) {
            ret = dfs_getPath(to, depth + 1, min(cap, ZKW::zkw.edge[i].pre_cap));
            ZKW::zkw.edge[i].pre_cap -= min(ret, ZKW::zkw.edge[i].pre_cap);
            if(ret != 0)
                return ret;
        }
    }
    dfs_vis[from] = false;

    return ret;
}

void out_goodResult(bool* video_list) {
    now_flow = 0;
    G_routeListNum = 0;
    G_resultLen = 0;

    for (int i = 0; i < ZKW::zkw.edge_num; i++) {
        ZKW::zkw.edge[i].pre_cap -= ZKW::zkw.edge[i].cap;
    }

    memset(dfs_vis, 0, sizeof(dfs_vis));
    while (dfs_getPath(ZKW::zkw.src, 0, INF)) {
        memset(dfs_vis, 0, sizeof(dfs_vis));
    }

    G_resultLen += sprintf(G_resultStr + G_resultLen, "%d\n\n", G_routeListNum);
    for (int i = 0; i < G_routeListNum; ++i) {
        int len = G_routeList[i].size();
        for (int j = 0; j < len - 1; ++j) {
            G_resultLen += sprintf(G_resultStr + G_resultLen, "%d ", G_routeList[i][j]);
        }
        G_resultLen += sprintf(G_resultStr + G_resultLen, "%d\n", G_routeList[i][len - 1]);
    }
    G_resultStr[G_resultLen - 1] = '\0';

}

void out_badResult()
{
    G_resultLen += sprintf(G_resultStr + G_resultLen, "%d\n\n", G_costNodeNum);

    for(int i=0; i<G_costNodeNum; ++i){
        G_resultLen += sprintf(G_resultStr + G_resultLen, "%d %d %d\n",
                               G_costEdge[i].node_id, G_costEdge[i].cost_id, G_costEdge[i].flow_need);
    }
    G_resultStr[G_resultLen-1] = '\0';
}

void init_ITEM()
{
    ITEM::edge = G_edge;
    ITEM::edge_num = G_edgeNum;
    ITEM::head = G_head;
    ITEM::node_num = G_netNodeNum;
    ITEM::video_cost = G_videoCost;
    ITEM::sum_flow = G_sumFlow;
    ITEM::capSum = G_capSum;
    ITEM::costEdge = G_costEdge;
    ITEM::costNodeNum = G_costNodeNum;

    GA::start_time = get_time();
}

int get_time()
{
    struct timeb rawtime;
    struct tm * timeinfo;
    ftime(&rawtime);
    timeinfo = localtime(&rawtime.time);

    int ms = rawtime.millitm;
    unsigned long s = rawtime.time;

//    printf("date/time is: %s \tused time is %lu s %d ms.\n", asctime(timeinfo), s, ms);

    return s;
}
