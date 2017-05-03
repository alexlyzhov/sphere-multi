#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <queue>

#include "Client.h"

Client::Client() {}

Client::Client(int efd, int socket) : efd(efd), socket(socket) {}

int Client::write_out(char* buf, size_t len)
// append data to the output queue and tell efd to listen for "writable" events
{
    // create and initialize write task
    WriteTask t;
    t.buf = new char[len];
    t.size = len;
    t.written = 0;
    strncpy(t.buf, buf, len);

    // push the output string to the waiting queue
    output.push(t);

    // tell epoll to listen for 'socket is writeable' events
    epoll_event event;
    event.data.fd = socket;
    event.events = EPOLLIN | EPOLLOUT;

    int s = epoll_ctl(efd, EPOLL_CTL_MOD, socket, &event);
    if (s == -1) {
        perror("epoll_ctl in Client::write_out()");
        return -1;
    }
    return 0; // success
}

int Client::flush()
{
    WriteTask& t = output.front();
    size_t write_size = t.size - t.written;

    int n_written = write(socket, t.buf + t.written, write_size);
    if (n_written == -1) {
        perror("write in Client::flush()");
        return -1;
    }
    t.written += (size_t) n_written;

    if (t.written == t.size) {
        output.pop();
        delete[] t.buf;
    }

    if (output.empty()) {
        epoll_event event;
        event.data.fd = socket;
        event.events = EPOLLIN; // disable EPOLLOUT events because there is nothing to write

        int s = epoll_ctl(efd, EPOLL_CTL_MOD, socket, &event);
        if (s == -1) {
            perror("epoll_ctl in Client::flush()");
            return -1;
        }
    }

    return 0; // success
}