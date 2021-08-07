#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
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

    // neighbor 정보 입력
    for (i = 0; i < nodeNum; i++)
    {
        routingTable[i][i][1] = i;
    }

    topologyFile.close();
}

int distance_vector()
{
    /*
    distance vector 라우팅을 실행하는 함수
    */
    int i, j, k;
    int dist;
    int changed = 0;

    for (i = 0; i < nodeNum; i++)
    {
        for (j = 0; j < nodeNum; j++)
        {
            if (routingTable[i][j][3] == 1) // direct neighnor에 대해서
            {
                dist = routingTable[i][j][4];
                for (k = 0; k < nodeNum; k++)
                {
                    if (routingTable[i][k][2] <= 0 && routingTable[j][k][2] > 0)
                    { // 새로운 연결 업데이트
                        if (i != k)
                        {
                            routingTable[i][k][2] = routingTable[j][k][2] + dist;
                            routingTable[i][k][1] = j;
                            changed = 1;
                        }
                    }

                    // tie breaking rule: ID 값이 작은 노드를 선택 (<-앞 번호의 노드 부터 확인)
                    if (routingTable[i][k][2] > 0 && routingTable[j][k][2] <= 0)
                    { // 새로운 연결 업데이트
                        if (j != k)
                        {
                            routingTable[j][k][2] = routingTable[i][k][2] + dist;
                            routingTable[j][k][1] = i;
                            changed = 1;
                        }
                    }

                    if (routingTable[i][k][2] > routingTable[j][k][2] + dist)
                    { // 짧은 경로로 업데이트
                        routingTable[i][k][2] = routingTable[j][k][2] + dist;
                        routingTable[i][k][1] = j;
                        changed = 1;
                    }

                    if (routingTable[i][k][2] + dist < routingTable[j][k][2])
                    { // 짧은 경로로 업데이트
                        routingTable[j][k][2] = routingTable[i][k][2] + dist;
                        routingTable[j][k][1] = i;
                        changed = 1;
                    }
                }
            }
        }
    }

    return changed;
}

void print_routing_table(ofstream &output)
{
    /*
    라우팅 테이블을 output 파일에 출력하는 함수
    */
    int i, j, k;

    // routing table 파일 출력
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
        for (i = 0; i < hops.size(); i++)
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
    int no_converged = 1;
    int change = 1;
    int line_num = 1;
    ofstream output("output_dv.txt");

    if (argc != 4) // 인자가 세 개가 아닐 경우 에러메시지 출력
    {
        fprintf(stderr, "usage: distvec topologyfile messagefile changesfile\n");
        exit(1);
    }

    topology = argv[1];
    messages = argv[2];
    changes = argv[3];

    // topology로부터 라우팅 테이블 초기화
    initialize_routing_table_from_topology(topology);

    //converged 상태가 될 때까지 distance vector 반복 실행
    while (no_converged != 0)
    {
        no_converged = 0;
        no_converged = distance_vector();
    }
    // 초기 라우팅 테이블 출력
    print_routing_table(output);

    // 메시지 처리
    send_messages(messages, output);

    // 변경 내용 적용 및 라우팅 테이블과 메시지 출력
    while (change != 0)
    {
        change = 0;
        change = apply_changes(changes, topology, line_num);
        if (change == 1)
        {
            no_converged = 1;
            while (no_converged != 0)
            {
                no_converged = 0;
                no_converged = distance_vector();
            }
            print_changed_routing_table(output);
            send_messages(messages, output);
        }
    }

    cout << "Complete. Output file written to output_dv.txt." << '\n';

    output.close();
    return 0;
}