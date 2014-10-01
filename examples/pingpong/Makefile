CC=gcc
CXX=g++
CXXFLAGS=-g -pipe -W -Wall -fPIC
CFLAGS=-g -pipe -W -Wall -fPIC
CPPFLAGS=-g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -DMONITOR -std=c++11 -O2
INCPATH= -I../../../../xlib/ -I/usr/local/include -Isrc/ -I../../../../xlib//lib/public/output/include -I../../../../xlib//lib/monitor/output/include -I../../../../xlib//../github/magneto/output/include -I../../../../xlib//lib/log/output/include -I../../../../xlib//lib/conf/output/include

.PHONY:all
all: check_md5 bin/pingpong 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

COMAKE_MD5=a4fd808744ab00567bc7f5c5544c7c96  COMAKE

.PHONY:check_md5
check_md5:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcheck_md5[0m']"
	@echo "$(COMAKE_MD5)">.comake2.md5
	@md5sum -c --status .comake2.md5
	@rm -f .comake2.md5

.PHONY:clean
clean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40clean[0m']"
	rm -rf bin/pingpong
	rm -rf output
	rm -rf src/pingpong.o

bin/pingpong: \
  src/pingpong.o 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mbin/pingpong[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o bin/pingpong \
  src/pingpong.o \
	-Xlinker "-(" 	../../../../xlib//lib/public/output/lib/libpublic.a \
	../../../../xlib//lib/monitor/output/lib/libmonitor.a \
	../../../../xlib//../github/magneto/output/lib/libmagneto.a \
	../../../../xlib//lib/log/output/lib/liblog.a \
	../../../../xlib//lib/conf/output/lib/libconf.a \
  -L/usr/local/lib -lpthread -lrt -lz -llog4cplus -Xlinker "-)"
		mkdir -p ./output/bin output/include/
		cp -f bin/pingpong ./output/bin
		cp -r src/* output/include/ &> /dev/null
		find output/ -regex ".*\.c\|.*\.cpp\|.*\.cc\|.*\.o" | xargs -i rm -rf {}

src/pingpong.o : src/pingpong.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(INCPATH) $(DEP_INCPATH) -o src/pingpong.o src/pingpong.cpp

