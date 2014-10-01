magneto
=======

***A c++ network framework***

*Notice*
* First tag version is expected to be released in november

*Features*
* Coroutine based design, which lets you to write asyc network logics in a sync fashion  
* Parallel downstream visit Apis and health check mechanism are supported
* Fully configurable
* Protocol codecs extensible
* Hign performance. Pingpong test 14w pqs, Broadcast test 21w qps, in 8 core cpu


*Dependencies*
* log4cplus, version 1.1 or higher

*Usage*
* add include directory of log4cplus into parameters of "include_directories" in magneto/CMakeLists.txt
* cd magneto && mkdir build && cd build
* cmake ../ && make clean && make -j4 -s
* include files and lib are created in "magneto/" directory
* enjoy

*Features in the future (depends on needs)*
* Supporte http protocol codec
* Services.conf hot loaded supported
* Enhance redis protocol codec
* Further optimizations

