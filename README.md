magneto
=======

***A c++ network framework***

*Features*
* Coroutine based design, which lets you to write asyc network logics in a sync fashion  
* Parallel downstream visit Apis and health check mechanism are supported
* Fully configurable
* Protocol codecs extensible
* Hign performance. Pingpong test 14w pqs, Broadcast test 21w qps, in 8 core cpu


*Dependencies*
* log4cplus

*Usage*
* cd magneto && mkdir build && cd build
* cmake ../ && make clean && make -j4 -s
* include files and lib are created in "magneto/" directory
* enjoy

*Features in the future (depends on needs)*
* Supporte http protocol 
* Enhance redis protocol
* Further optimized
