magneto
=======

A c++ network framework

Features:
1. Coroutine based design, which lets you to write asyc network logics in a sync fashion
2. Parallel downstream visit Apis and health check mechanism are supported
3. Fully configurable
4. Protocol codecs extensible
4. Hign performance. Pingpong test 14w pqs, Broadcast test 21w qps, in 8 core cpu

Dependencies:
1. log4cplus

Usage:
1. cd magneto && mkdir build && cd build
2. cmake ../ && make clean && make -j4 -s
3. include files and lib are created in "magneto/" directory
4. enjoy

Features in the future: (depends on needs)
1. Supporte http protocol 
2. Enhance redis protocol
3. Further optimized
