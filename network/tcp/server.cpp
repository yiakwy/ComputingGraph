//
//  main.cpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 22/7/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//

#include <iostream>
#include "select.hpp"
#include "kqueue.hpp"


int main(int argc, const char * argv[]) {
    // return select_server(argc, (char**)argv);
    return kqueue_server(argc, (char**)argv);
}
