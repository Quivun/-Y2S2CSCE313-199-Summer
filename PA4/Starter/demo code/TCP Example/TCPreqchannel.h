#ifndef _TCPreqchannel_H_
#define _TCPreqchannel_H_

#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>
using namespace std;

class TCPRequestChannel
{
private:
    int sockfd; // analogous to rfd and wrd in fiforeqchan

    int server(string port)
    {
        struct addrinfo hints, *serv;
        struct sockaddr_storage theirAddress;
        socklen_t sin_size;
        char s[INET6_ADDRSTRLEN];
        int rv;

        memset(&hints, 0, sizeof(hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE; // use self ip

        if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &serv)) != 0){
            cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
            return -1;
        }
        // Here, the socket acts as the master socket and requires a bind call
        if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv-> ai_protocol)) == -1){
            perror("server: socket");
            return -1;
        }
        if (bind(sockfd, serv-> ai_addr, serv->ai_addrlen) == -1 ){
            close(sockfd);
            perror("server: bind");
            return -1;
        }
        freeaddrinfo(serv); // all done with this structure

        if(listen(sockfd,20) == -1){
            perror("listen");
            exit(1);
        }
    }
    int client(string host, string port)
    {
        struct addrinto hints, *res;

        // loads up address struct with getaddrinfo();
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        int status;
        // getaddrinfo("www.example.com", "3490", &hints, &res);
        if ((status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res)) !=)
        {
            cerr << "getaddrinfo: " << gai_strerror(status) << endl;
            return -1;
        }

        // socket creation
        this->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0)
        {
            perror("Socket Creation Error");
            return -1;
        }

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
        {
            perror("Connection Error");
            return -1;
        }
    }

public:
    TCPRequestChannel(const string host, const string port, int _side)
    {
        if (_side == 0)
        {
            server(_port);
        }
        else
        {
            client(_host, _port);
        }
    }
    TCPRequestChannel(int _s)
    {
        sockfd = _s;
    }
    ~TCPRequestChannel()
    {
        close(sockfd);
    }

    int cread(void *buf, int len)
    {
        return recv(sockfd, buf, len, 0);
    }
    int cwrite(void *buf, int len)
    {
        return send(sockfd, buf, len, 0);
    }
    int getsocket()
    {
        return sockfd;
    }
};

#endif