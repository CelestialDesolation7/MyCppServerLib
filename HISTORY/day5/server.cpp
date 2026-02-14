#include "Channel.h"
#include "Epoll.h"
#include "InetAddress.h"
#include "Socket.h"
#include "util.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
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
  serv_sock->setnonblocking();

  Channel *servChannel = new Channel(ep, serv_sock->getFd());
  servChannel->enableReading();
  std::cout << "[server] Server start success!" << std::endl;

  while (true) {
    // 这里本来要我们自己调用 epoll_wait
    std::vector<Channel *> activeChannels = ep->poll();
    int nfds = activeChannels.size();
    for (int i = 0; i < nfds; ++i) {
      int chFd = activeChannels[i]->getFd();

      // 情况A：新连接
      if (chFd == serv_sock->getFd()) {
        InetAddress *client_addr = new InetAddress();
        int client_sockfd = serv_sock->accept(client_addr);

        std::cout << "[server] new client fd" << client_sockfd
                  << "! IP:" << inet_ntoa(client_addr->addr.sin_addr)
                  << " PortL " << ntohs(client_addr->addr.sin_port)
                  << std::endl;

        Socket *client_sock = new Socket(client_sockfd);
        client_sock->setnonblocking();

        Channel *clientChannel = new Channel(ep, client_sockfd);
        clientChannel->enableReading();
        // 现在存在内存泄漏，以后处理
      }
      // 情况B：收数据
      else if (activeChannels[i]->getEvents() & EPOLLIN) {
        char buf[READ_BUFFER];
        while (true) {
          bzero(&buf, sizeof(buf));
          ssize_t bytes_read = read(chFd, buf, sizeof(buf));
          if (bytes_read > 0) {
            std::cout << "[server] message received from client fd " << chFd
                      << ": " << buf << std::endl;
            write(chFd, buf, bytes_read);
          } else if (bytes_read == -1 && errno == EINTR) {
            continue;
          } else if (bytes_read == -1 &&
                     ((errno == EAGAIN) || (errno == EWOULDBLOCK))) {
            break;
          } else if (bytes_read == 0 ||
                     (bytes_read == -1 && errno != EINTR && errno != EAGAIN)) {
            Channel *chToRemove = activeChannels[i];
            std::cout << "[server] EOF received from client fd " << chFd
                      << " and it's now disconnected" << std::endl;
            close(chFd);
            delete chToRemove;
            activeChannels[i] = nullptr;
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