#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>

#define INF 999

using namespace std;

vector<vector<vector<int> > > routingTable;
int nodeNum;

void initialize_routing_table_from_topology(const char *topology)
{
    /*
    topology로부터 라우팅 테이블을 생성하는 함수
    routingTable[nodeNum][nodeNum][5]:
        routingTable[nodeNum][nodeNum][0]: 목적지
        routingTable[nodeNum][nodeNum][1]: 다음
        routingTable[nodeNum][nodeNum][2]: 거리
        routingTable[nodeNum][nodeNum][3]: direct neigbor인 경우 1의 값을 가짐
        routingTable[nodeNum][nodeNum][4]: 링크 사이 거리(cost)
    */
    string line;
    int i, j;

    ifstream topologyFile(topology);
    if (!topologyFile)
    {
        fprintf(stderr, "Error: open input file\n");
        exit(1);
    }

    // initialize routing table
    getline(topologyFile, line);
    nodeNum = line[0] - '0';
    routingTable.assign(nodeNum, vector<vector<int> >(nodeNum, vector<int>(5, 0)));

    for (i = 0; i < nodeNum; i++)
    {
        for (j = 0; j < nodeNum; j++)
        {
            routingTable[i][j][0] = j;
        }
    }

    while (getline(topologyFile, line))
    {
        int src = line[0] - '0';
        int des = line[2] - '0';
        int cost = line[4] - '0';

        routingTable[src][des][2] = cost;
        routingTable[src][des][1] = des;
        routingTable[src][des][3] = 1;
        routingTable[src][des][4] = cost;
        routingTable[des][src][2] = cost;
        routingTable[des][src][1] = src;
        routingTable[des][src][3] = 1;
        routingTable[des][src][4] = cost;
    }

    // direct neighbor 정보 입력
    for (i = 0; i < nodeNum; i++)
    {
        routingTable[i][i][1] = i;
    }

    topologyFile.close();
}

void update_routing_table(int node, vector<vector<int> > lsp)
{
    /*
    linkstate 라우팅 이후, 라우팅 테이블을 업데이트 하는 함수
    */
    int i;
    int parent = -1;
    int dist = 0;
    int next_node;

    for (i = 0; i < nodeNum; i++)
    {
        // 목적지가 자신인 경우 업데이트 x
        if (node == i)
            continue;

        // 목적지까지의 거리가  INF(999)인 경우 거리를 -999(연결x)로 업데이트
        if (lsp[i][0] == INF)
        {
            routingTable[node][i][2] = -999;
            continue;
        }

        // 부모 노드를 통해 다음 노드 탐색
        dist = lsp[i][0];
        next_node = i;
        while (true)
        {
            parent = lsp[next_node][1];
            if (parent == node)
                break;
            next_node = parent;
        }
        // 다음 노드와 거리 업데이트
        routingTable[node][i][1] = next_node;
        routingTable[node][i][2] = dist;
    }
}

void linkstate()
{
    /*
    linkstate 라우팅을 실행하는 함수
    dij[nodeNum][3]: 다익스트라 수행을 위한 자료구조 (최초 INF(999)로 초기화)
        dij[nodeNum][0]: distance
        dij[nodeNum][1]: parent
        dij[nodeNum][2]: 방문 여부
    */
    int i, j, k;
    vector<int> SPT;
    int source;

    int min_dist, min_node;
    vector<vector<int> > dij;
    int next_node;

    for (i = 0; i < nodeNum; i++)
    {
        // SPT 최초 source 넣기
        source = routingTable[i][i][0];
        SPT.push_back(source);

        dij.assign(nodeNum, vector<int>(3, INF));

        // 현재 자신 노드 처리
        next_node = i;
        dij[i][0] = 0;
        dij[i][1] = 0;
        dij[i][2] = 1; // 방문 노드 체크

        for (j = 0; j < nodeNum; j++)
        {
            min_dist = INF;
            min_node = INF;
            for (k = 0; k < nodeNum; k++)
            {
                if (dij[k][2] != 1 && routingTable[next_node][k][4] != -999) // 방문하지 않았고 link가 끊어지지 않았을 경우
                {
                    if (routingTable[next_node][k][3] == 1 && dij[k][2] != 1) // direct neigbor에 대해서 거리 비교
                    {
                        if (dij[next_node][0] + routingTable[next_node][k][2] <= dij[k][0])
                        {
                            if (dij[next_node][0] + routingTable[next_node][k][2] == dij[k][0])
                            {
                                if (next_node < k) // tie breaking rule: ID 값이 작은 노드를  parent로 선택
                                {
                                    dij[k][0] = dij[next_node][0] + routingTable[next_node][k][2]; // distance 업데이트
                                    dij[k][1] = next_node;                                         // parent 업데이트
                                }
                            }
                            else
                            {
                                dij[k][0] = dij[next_node][0] + routingTable[next_node][k][2]; // distance 업데이트
                                dij[k][1] = next_node;                                         // parent 업데이트
                            }
                        }
                    }
                }
                // SPT에 넣을 node 구하기
                if (dij[k][2] != 1 && dij[k][0] <= min_dist) // 방문하지 않았고 distance가 min_dist보다 작거나 같은 경우
                {
                    if (dij[k][0] == min_dist)
                    {
                        if (k < min_node) // tie breaking rule: ID 값이 작은 노드를 선택
                        {
                            min_dist = dij[k][0];
                            min_node = k;
                        }
                    }
                    else
                    {
                        min_dist = dij[k][0];
                        min_node = k;
                    }
                }
            }
            if (j == nodeNum - 1 || min_node == 999) // 마지막 노드이거나 연결된 링크가 없을 경우
                break;

            SPT.push_back(min_node); // SPT에 노드 넣기
            dij[min_node][2] = 1;    // 방문 노드 체크
            next_node = min_node;    // 다음 노드로 이동
        }
        // 라우팅 테이블 업데이트
        update_routing_table(i, dij);
    }
}

void print_routing_table(ofstream &output)
{
    /*
    라우팅 테이블을 output 파일에 출력하는 함수
    */
    int i, j, k;
    for (i = 0; i < nodeNum; i++)
    {
        for (j = 0; j < nodeNum; j++)
        {
            for (k = 0; k < 3; k++)
            {
                output << routingTable[i][j][k] << ' ';
            }
            output << '\n';
        }
        output << '\n';
    }
}

void send_messages(const char *messages, ofstream &output)
{
    /*
    메시지를 보내고 출력하는 함수
    */
    string line;
    int src, des, cost;
    string message;
    size_t i;

    ifstream messagesFile(messages);
    if (!messagesFile)
    {
        fprintf(stderr, "Error: open input file\n");
        exit(1);
    }

    while (getline(messagesFile, line))
    {
        // source, destination, message, cost 정보 저장
        src = line[0] - '0';
        des = line[2] - '0';
        message = line.substr(4);
        cost = routingTable[src][des][2];

        if (cost <= 0 && src != des)
        { // 송신자로부터 수신자로의 경로가 존재하지 않는 경우
            output << "from " << src << " to " << des << " cost infinite hops unreachable message " << message;
            messagesFile.close();
            return;
        }
        // hops 정보 저장
        vector<int> hops;
        hops.push_back(src);
        int hop = routingTable[src][des][1];
        while (hop != des)
        {
            hops.push_back(hop);
            hop = routingTable[hop][des][1];
        }

        // message 출력
        output << "from " << src << " to " << des << " cost " << cost << " hops ";
        for (i = 0; i != hops.size(); i++)
        {
            output << hops[i] << " ";
        }
        output << "message " << message << '\n';
    }
    output << '\n';

    messagesFile.close();
}

int apply_changes(const char *changes, const char *topology, int &line_num)
{
    /*
    변경 내용 적용하고 라우팅 테이블 업데이트하는 함수
    */
    string line;
    int src, des, cost;
    int change = 1;
    int i;

    ifstream changesFile(changes);
    if (!changesFile)
    {
        fprintf(stderr, "Error: open input file\n");
        exit(1);
    }

    for (i = 0; i < line_num - 1; i++)
    {
        getline(changesFile, line);
    }

    if (!getline(changesFile, line))
    {
        change = 0;
        changesFile.close();

        return change;
    }
    src = line[0] - '0';
    des = line[2] - '0';

    if (line[4] == '-')
    {
        cost = -999;
    }
    else
        cost = line[4] - '0';

    // 변경 내용 적용
    initialize_routing_table_from_topology(topology);
    routingTable[src][des][2] = cost;
    routingTable[src][des][1] = des;
    routingTable[src][des][3] = 1;
    routingTable[src][des][4] = cost;
    routingTable[des][src][2] = cost;
    routingTable[des][src][1] = src;
    routingTable[des][src][3] = 1;
    routingTable[des][src][4] = cost;

    // 링크가 끊어졌을 경우 direct neigbor 처리
    if (cost == -999)
    {
        routingTable[src][des][3] = 0;
        routingTable[des][src][3] = 0;
    }

    line_num += 1;

    changesFile.close();
    return change;
}

void print_changed_routing_table(ofstream &output)
{
    /*
    변화된 라우팅 테이블을 출력하는 함수
    */
    int i, j, k;
    for (i = 0; i < nodeNum; i++)
    {
        for (j = 0; j < nodeNum; j++)
        {
            if ((i != j) && routingTable[i][j][2] <= 0)
            {
                continue;
            }
            else
            {
                for (k = 0; k < 3; k++)
                {

                    output << routingTable[i][j][k] << ' ';
                }
                output << '\n';
            }
        }
        output << '\n';
    }
}

int main(int argc, char *argv[])
{
    const char *topology;
    const char *messages;
    const char *changes;
    int change = 1;
    int line_num = 1;
    ofstream output("output_ls.txt");

    if (argc != 4) // 인자가 세 개가 아닐 경우 에러메시지 출력
    {
        fprintf(stderr, "usage: linkstate topologyfile messagefile changesfile\n");
        exit(1);
    }

    topology = argv[1];
    messages = argv[2];
    changes = argv[3];

    // topology로부터 라우팅 테이블 초기화
    initialize_routing_table_from_topology(topology);

    // link state 라우팅 실행
    linkstate();

    // 초기 라우팅 테이블 출력
    print_routing_table(output);

    // 메시지 처리
    send_messages(messages, output);

    // 변경 내용 적용 및 라우팅 테이블과 메시지 출력
    while (change != 0) // 변경 내용이 없을 때까지 반복
    {
        change = 0;
        change = apply_changes(changes, topology, line_num);

        if (change == 1)
        {
            linkstate();
            print_changed_routing_table(output);
            send_messages(messages, output);
        }
    }

    cout << "Complete. Output file written to output_ls.txt." << '\n';

    output.close();
    return 0;
}