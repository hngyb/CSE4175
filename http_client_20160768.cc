#include <iostream>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

#define MAXDATASIZE 1000

int main(int argc, char *argv[])
{
    int sockfd;
    int numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo;
    int rv;
    char s[INET_ADDRSTRLEN];

    if (argc != 2) // 인자가 한 개가 아닐 경우 에러메시지 출력
    {
        fprintf(stderr, "usage: http_client http://hostname[:port][/path/to/file]\n");
        exit(1);
    }

    string str_argv = argv[1];
    if (str_argv.substr(0, 7) != "http://") // 인자 앞 부분에 http:// 가 없을 경우 에러메시지 출력
    {
        fprintf(stderr, "usage: http_client http://hostname[:port][/path/to/file]\n");
        exit(1);
    }

    string str_hostname;
    string str_port = "80"; // default 포트번호: 80
    string str_path = "/";  // default 경로: /
    size_t pos_path = 0;
    size_t pos_port = 0;

    if ((pos_path = str_argv.find("/", 7)) != string::npos) // path 부분의 첫 index 찾기
    {
        str_path = str_argv.substr(pos_path);                   // path를 string 형으로 저장
        if ((pos_port = str_argv.find(":", 7)) != string::npos) // port 부분의 첫 index 찾기
        {
            str_port = str_argv.substr(pos_port + 1, pos_path - pos_port - 1); // port를 string 형으로 저장
            str_hostname = str_argv.substr(7, pos_port - 7);                   // hostname을 string 형으로 저장
        }
        else // port 부분이 없을 시
        {
            str_hostname = str_argv.substr(7, pos_path - 7);
        }
    }
    else // path 부분이 없을 시
    {
        pos_path = str_argv.length();
        if ((pos_port = str_argv.find(":", 7)) != string::npos)
        {
            str_port = str_argv.substr(pos_port + 1, pos_path - pos_port - 1); // port를 string 형으로 저장
            str_hostname = str_argv.substr(7, pos_port - 7);                   // hostname을 string 형으로 저장
        }
        else
        {
            str_hostname = str_argv.substr(7, pos_path - 7); // hostname을 string 형으로 저장
        }
    }

    // string to char
    char *hostname = new char[str_hostname.length() + 1];
    char *port = new char[str_port.length() + 1];
    char *path = new char[str_path.length() + 1];
    strcpy(hostname, str_hostname.c_str());
    strcpy(port, str_port.c_str());
    strcpy(path, str_path.c_str());

    // cout << hostname << ',' << port << ',' << path << '\n';

    memset(&hints, 0, sizeof hints); // hints의 메모리 시작점부터 hints의 size만큼 0으로 모두 지정
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // TCP

    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1)
    {
        perror("client: socket");
        return 1;
    }

    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
    {
        close(sockfd);
        perror("connect");
        exit(1);
    }

    // inet_ntop(servinfo->ai_family, &((struct sockaddr_in *)servinfo->ai_addr)->sin_addr, s, sizeof s);
    // printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);

    string http_request = "GET " + str_path + " HTTP/1.1\r\n" + "Host: " + str_hostname + ":" + str_port + "\r\n\r\n";
    strcpy(buf, http_request.c_str());
    if (send(sockfd, buf, strlen(buf), 0) == -1)
    {
        perror("send");
        close(sockfd);
        exit(1);
    }

    if ((numbytes = recv(sockfd, buf, sizeof buf, 0)) == -1)
    {
        perror("recv");
        close(sockfd);
        exit(1);
    }
    buf[numbytes] = '\0';

    string http_response = buf;
    string header;
    string content_length;
    string status_code;
    string payload;
    size_t pos_header = 0;
    size_t pos_status_code = 0;
    size_t pos_content_length = 0;

    if ((pos_header = http_response.find("\r\n\r\n")) != string::npos) // header 부분과 payload 부분 추출
    {
        header = http_response.substr(0, pos_header);
        payload = http_response.substr(pos_header + 4);
    }

    if ((pos_status_code = http_response.find("\r\n")) != string::npos) // status code 찾아서 출력
    {
        status_code = http_response.substr(0, pos_status_code);
        cout << status_code << endl;
    }

    if ((pos_content_length = http_response.find("Content-Length")) != string::npos) // Content-Length의 값 찾아서 저장
    {
        pos_content_length += 16;
        content_length = http_response.substr(pos_content_length, http_response.find("\r\n", pos_content_length) - pos_content_length);
    }
    else
    {
        if ((pos_content_length = http_response.find("content-length")) != string::npos)
        {
            pos_content_length += 16;
            content_length = http_response.substr(pos_content_length, http_response.find("\r\n", pos_content_length) - pos_content_length);
        }
        else
        {
            if ((pos_content_length = http_response.find("Content-length")) != string::npos)
            {
                pos_content_length += 16;
                content_length = http_response.substr(pos_content_length, http_response.find("\r\n", pos_content_length) - pos_content_length);
            }
            else
            {
                cout << "Content-Length not specified." << endl;
                exit(1);
            }
        }
    }

    ofstream fout;
    fout.open("20160768.out");
    fout << payload;

    size_t remain_data = atoi(content_length.c_str()) - payload.size();
    while (remain_data > 0) // Content-Length의 payload를 모두 receive할 수 있도록 recv 반복 호출
    {
        if ((numbytes = recv(sockfd, buf, sizeof buf, 0)) == -1)
        {
            perror("recv");
            close(sockfd);
            exit(1);
        }
        buf[numbytes] = '\0';
        fout << buf;
        remain_data -= numbytes;
    }
    fout.close();
    cout << content_length << " bytes written to 20160768.out" << endl;

    return 0;
}
