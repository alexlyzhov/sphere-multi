#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <vector>
#include <fstream>

#include "Client.h"

#define MAXEVENTS 256
#define MAX_MSG_CHUNK 4096

int set_nonblocking(int sock_fd) {
    int flags = fcntl(sock_fd, F_GETFL, 0);
    if (flags == -1) {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;

    int s = fcntl(sock_fd, F_SETFL, flags);
    if (s == -1) {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

int create_master(uint port) {
    int socket_fd;

    struct sockaddr_in socket_addr;
    bzero(&socket_addr, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET; // always AF_INET
    socket_addr.sin_port = htons(port); // convert port (unsigned short) to network byte order
    socket_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // convert local address (unsigned long) to network byte order

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int res = bind(socket_fd, (struct sockaddr *) &socket_addr, sizeof(socket_addr)); // assign address
    if (res == -1) {
        return -1;
    }

    res = set_nonblocking(socket_fd);
    if (res == -1) {
        return -1;
    }

    return socket_fd;
}

int main()
{
    int master_socket;
    std::vector<int> slave_sockets;
    std::map<int, Client> clients_map;  // slave_socket -> Client object

    std::fstream port_file("port.txt", std::ios_base::in);
    int port;
    port_file >> port;

    master_socket = create_master(port);
    if (master_socket == -1) {
        perror("master_socket creation");
        return 1;
    }

    int s = listen(master_socket, SOMAXCONN);
    if (s == -1) {
        perror("listen");
        return 1;
    }

    int efd = epoll_create1(0);
    struct epoll_event event;
    struct epoll_event *events;

    event.data.fd = master_socket;
    event.events = EPOLLIN | EPOLLET; // edge-triggered epoll: block if there is no data _unnotified_ about
    s = epoll_ctl(efd, EPOLL_CTL_ADD, master_socket, &event);
    if (s == -1) {
        perror("epoll_ctl");
        return 1;
    }
    events = new epoll_event[MAXEVENTS];

    while (true) {
        int n = epoll_wait(efd, events, MAXEVENTS, -1); // timeout == -1 -> wait indefinitely
        // now n is the number of events fetched
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            // error in socket or it was closed
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & (EPOLLIN | EPOLLOUT)))) { // error
                printf("epoll error at descriptor %d\n", fd);
                clients_map.erase(fd);
                close(fd);
            } else if ((events[i].events & EPOLLOUT) && (fd != master_socket)) { // client socket is writeable, flush data
                Client& c = clients_map[fd];
                if (c.flush() == -1) { // not writeable, exclude this client from map
                    clients_map.erase(fd);
                    close(fd);
                }
            } else if (fd == master_socket) { // accept connection
                while (true) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof(in_addr);
                    infd = accept(master_socket, &in_addr, &in_len);

                    if (infd == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            break; // no more connections to process
                        }
                        else {
                            perror("accept\n");
                            delete[] events;
                            close(master_socket);
                            exit(1);
                        }
                    }

                    set_nonblocking(infd);
                    slave_sockets.push_back(infd);

                    s = getnameinfo(&in_addr, in_len,
                                    hbuf, sizeof(hbuf),
                                    sbuf, sizeof(sbuf),
                                    NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0) {
                        printf("New connection (descriptor: %d, host: %s, port: %s)\n", infd, hbuf, sbuf);
                    }

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1) {
                        perror("epoll_ctl");
                        return 1;
                    }

                    clients_map[infd] = Client(efd, infd);
                }
            } else { // new message
                bool closed = false;
                // read input message chunk-by-chunk
                while (true) {
                    ssize_t count;
                    char buf[MAX_MSG_CHUNK];
                    count = read(fd, buf, sizeof(buf)); // read a chunk
                    if (count == -1) {
                        if (errno != EAGAIN) {
                            perror("read");
                            closed = true;
                        }
                        break;
                    }
                    else if (count == 0) { // EOF; connection closed
                        closed = true;
                        break;
                    }

                    for (std::pair<const int, Client> &kv : clients_map) {
                        int result = kv.second.write_out(buf, count); // enqueue message chunk
                        if (result == -1) {
                            clients_map.erase(fd);
                            close(fd);
                        }
                    }
                }
                if (closed) {
                    printf("connection closed on descriptor %d\n", fd);
                    clients_map.erase(fd);
                    close(fd);
                }
            }
        }
    }

    delete[] events;
    close(master_socket);
}
