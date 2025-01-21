//
// Created by Samuli on 10/30/24.
//

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <unistd.h>
#include <cstring>
#include "BasicServer.h"

void ServerStart()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1234);

    int successfulBind = bind(socketfd,
                              (struct sockaddr *) &addr, sizeof(addr));

    if (successfulBind != 0) {
        std::cerr << "Error binding socket\n";
        close(socketfd);
        return;
    }

    if (listen(socketfd, 5) < 0) {
        std::cerr << "Error listening\n";
        close(socketfd);
        return;
    }

    struct pollfd fds[1];
    fds[0].fd = socketfd;

    char buff[512];

    for (int i = 0; i < 20; i++) {
        int ret = poll(fds, 1, 1000);
        if (ret < 0) {
            std::cerr << "Error\n";
            return;
        }

        if (ret > 0) {
            read(socketfd, buff, sizeof(buff));
            std::cout << buff << "\n";
            // TODO: accepting connection and reading data
        }
    }

    close(socketfd);
}