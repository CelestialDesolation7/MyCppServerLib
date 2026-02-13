#include "include/Epoll.h"
#include "include/InetAddress.h"
#include "include/Socket.h"
#include "include/util.h"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <iterator>
#include <netinet/in.h>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define READ_BUFFER 1024

int main() {
  // 创建服务器 socket
  Socket *serv_sock = new Socket();
  InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
  serv_sock->bind(serv_addr);

  serv_sock->listen();
  Epoll *ep = new Epoll();

  setnonblocking(serv_sock->getFd());

  ep->addFd(serv_sock->getFd(), EPOLLIN | EPOLLET);

  std::cout << "[server] Server start success!" << std::endl;

  while (true) {
    // 这里本来要我们自己调用 epoll_wait
    std::vector<epoll_event> events = ep->poll();
    int nfds = events.size();
    for (int i = 0; i < nfds; ++i) {
      // 情况A：新连接
      if (events[i].data.fd == serv_sock->getFd()) {
        InetAddress *client_addr = new InetAddress();
        int client_sockfd = serv_sock->accept(client_addr);
        std::cout << "[server] new client fd" << client_sockfd
                  << "! IP:" << inet_ntoa(client_addr->addr.sin_addr)
                  << " PortL " << ntohs(client_addr->addr.sin_port)
                  << std::endl;

        setnonblocking(client_sockfd);
        ep->addFd(client_sockfd, EPOLLIN | EPOLLET);
        delete client_addr;
      }
      // 情况B：收数据
      else if (events[i].events & EPOLLIN) {
        int sockfd = events[i].data.fd;
        char buf[READ_BUFFER];
        while (true) {
          bzero(&buf, sizeof(buf));
          ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
          if (bytes_read > 0) {
            std::cout << "[server] message received from client fd " << sockfd
                      << ": " << buf << std::endl;
            write(sockfd, buf, bytes_read);
          } else if (bytes_read == -1 && errno == EINTR) {
            continue;
          } else if (bytes_read == -1 &&
                     ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            break;
          } else if (bytes_read == 0) {
            std::cout << "[server] EOF received from client fd " << sockfd
                      << " and it's now disconnected" << std::endl;
            close(sockfd);
            break;
          }
        }
      }
    }
  }
  delete serv_sock;
  delete serv_addr;
  delete ep;
  return 0;
}