//
//  epoll.hpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 17/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//
//  epoll was introduced in Linux 2.6. It provides best readiness notification facility in Linux just like what is for other *nix system like select and poll.
//
//  select: can monitor up to FD_SETSIZE file descriptors
//  poll: is expensive to perform linear scan to check readiness notification
//
//  Note: MacOS X does not support epoll. But it supports <Kernel Queues> which is very similar. You can also practice it in a Linxu vertual machine.

#ifndef epoll_hpp
#define epoll_hpp

#include <stdio.h>

#endif /* epoll_hpp */
