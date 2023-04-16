# MinorWebserver

一个linux下的Modern C++实现的简易webserver，经压测可实现上万并发。

# 特点

> - 基于非阻塞IO+EPOLL(ET)+Reactor模型的多线程高并发模型
> - c++11编写的高效线程池
> - 使用主从状态机+正则表达式解析HTTP报文，支持get/post
> - 封装vector实现可自我增长的读写缓存区
> - 基于阻塞队列实现的异步日志系统
> - 用小根堆实现的高效定时器

# 鸣谢

以下项目或文章给予了我宝贵的帮助

[TinyWebserver](https://github.com/qinguoyi/TinyWebServer)

[Webserver](https://github.com/markparticle/WebServer)

Linux高性能服务器编程，游双著.

Linux多线程服务端编程：使用muduo C++网络库，陈硕著.
