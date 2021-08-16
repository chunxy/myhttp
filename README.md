## execution
1. currently run compile.sh
2. run build/server *port_number*
3. ping the server at **localhost:***port_number*

## test
```
1000 fetches, 8 max parallel, 2.993e+06 bytes, in 0.452735 seconds
2993 mean bytes/connection
2208.8 fetches/sec, 6.61093e+06 bytes/sec
msecs/connect: 0.104882 mean, 0.533 max, 0.018 min
msecs/first-response: 3.39544 mean, 28.149 max, 2.103 min
HTTP response codes:
  code 200 -- 1000
```

## 零碎的知识点
- [epoll_wait interrupted system call](https://blog.csdn.net/xidomlove/article/details/8274732)，多线程如何相应信号？
- pthread_mutex_lock和pthread_cond_wait搭配使用
- 主线程新增事件记得sleep以提升响应率
- [REUSEADDR导致无法监听？](https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux)
- Reactor模型
- http_session和http_server中将handler放在第一位，以便于类型转换（伪继承）
- 任务队列的抽象
- http_session的状态转换与监听事件变更
- socket接口标志的含义
  - https://man7.org/linux/man-pages/man7/socket.7.html
  - https://man7.org/linux/man-pages/man2/socket.2.html
- getaddrinfo和gethostbyname/gethostbyaddr
  - https://stackoverflow.com/questions/52727565/client-in-c-use-gethostbyname-or-getaddrinfo
  - https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
- timer相关
  - https://zhuanlan.zhihu.com/p/205609198
  - https://www.codenong.com/cs106560073/
  - https://man7.org/linux/man-pages/man2/timer_create.2.html
  - https://man7.org/linux/man-pages/man7/sigevent.7.html
- eventfd
  - https://zhuanlan.zhihu.com/p/40572954
  - https://www.cnblogs.com/developing/p/10887779.html
- accept epollet non-block
  - https://blog.csdn.net/qq_43684922/article/details/89388059
- [http_load使用](https://www.jianshu.com/p/c869f96ed929)

## todo
- [ ] timer
- [ ] error handling
- [ ] http method handling
- [ ] “析构函数”
- [ ] 自定义宏处理
- [ ] multi-line function formatting
- [ ] 使用sendfile传输文件
- [ ] filename至少长255
- [ ] 无用头文件
- [ ] makefile
