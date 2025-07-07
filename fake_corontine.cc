
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

#include <memory>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

enum CoroutineState
{
    INIT,
    READING,
    PROCESSING,
    WRITING,
    DONE
};

class Coroutine
{
  public:
    Coroutine(int fd) : fd_(fd), state_(INIT), data_()
    {
    }

    ~Coroutine()
    {
        if (fd_ >= 0)
            ::close(fd_);
    }

    int fd() const
    {
        return fd_;
    }
    bool isDone() const
    {
        return state_ == DONE;
    }

    // 协程 resume，根据 state 和 ev 处理
    void resume(uint32_t ev, epoll_event &outEv)
    {
        outEv.data.ptr = this;
        outEv.events = 0;
        switch (state_)
        {
        case INIT:
            state_ = READING;
            outEv.events = EPOLLIN;
            break;
        case READING:
            if (ev & EPOLLIN)
            {
                char buf[1024];
                ssize_t n = ::recv(fd_, buf, sizeof(buf), 0);
                if (n > 0)
                {
                    data_.assign(buf, n);
                    state_ = PROCESSING;
                    outEv.events = EPOLLOUT;
                }
                else
                {
                    state_ = DONE;
                    return;
                }
            }
            break;
        case PROCESSING:
            data_ = "echo: " + data_;
            std::cout << "data_:" << data_ << std::endl;
            state_ = WRITING;
            outEv.events = EPOLLOUT;
            break;
        case WRITING:
            if (ev & EPOLLOUT)
            {
                ::send(fd_, data_.c_str(), data_.size(), 0);
                state_ = DONE;
            }
            break;
        case DONE:
            state_ = DONE;
            break;
        }
    }

  private:
    int fd_;
    CoroutineState state_;
    std::string data_;
};

static int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_nonblocking(listen_fd);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(12345);
    bind(listen_fd, (sockaddr *)&addr, sizeof(addr));
    listen(listen_fd, SOMAXCONN);

    int epfd = epoll_create1(0);
    epoll_event ev{};
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    std::map<int, std::shared_ptr<Coroutine>> cos;

    const int MAX_EVENTS = 10;
    std::vector<epoll_event> events(MAX_EVENTS);

    while (true)
    {
        int n = epoll_wait(epfd, events.data(), MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i)
        {
            auto &e = events[i];
            if (e.data.fd == listen_fd)
            {
                // 接受新连接
                int cfd = ::accept(listen_fd, nullptr, nullptr);
                set_nonblocking(cfd);
                auto co = std::make_shared<Coroutine>(cfd);
                cos[cfd] = co;

                epoll_event cev{};
                cev.data.ptr = co.get();
                cev.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &cev);
            }
            else
            {
                // I/O 复用到协程
                Coroutine *co = static_cast<Coroutine *>(e.data.ptr);
                epoll_event cev{};
                co->resume(e.events, cev);

                if (co->isDone())
                {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, co->fd(), nullptr);
                    close(co->fd()); // ✅ 关闭 socket fd，释放内核资源
                    cos.erase(co->fd());
                }
                else
                {
                    epoll_ctl(epfd, EPOLL_CTL_MOD, co->fd(), &cev);
                }
            }
        }
    }
    return 0;
}
