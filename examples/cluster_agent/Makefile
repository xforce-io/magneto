CC=gcc
CXX=g++
CXXFLAGS=-g -pipe -W -Wall -fPIC
CFLAGS=-g -pipe -W -Wall -fPIC
CPPFLAGS=-g -W -Wall -Wno-deprecated -Wno-invalid-offsetof -DIDX_LOG_STDOUT -fno-builtin-memcmp -std=c++11 -O2
INCPATH= -I../../../../ -I/usr/local/include -Isrc/ -I../../../..//lib/public/output/include -I../../../..//lib/log/output/include -I../../../..//lib/monitor/output/include -I../../../..//projects/magneto/output/include -I../../../..//lib/conf/output/include

.PHONY:all
all: check_md5 bin/cluster_agent 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

COMAKE_MD5=9c9d60ab97d68bcb01b3e06a6f183efd  COMAKE

.PHONY:check_md5
check_md5:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcheck_md5[0m']"
	@echo "$(COMAKE_MD5)">.comake2.md5
	@md5sum -c --status .comake2.md5
	@rm -f .comake2.md5

.PHONY:clean
clean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40clean[0m']"
	rm -rf bin/cluster_agent
	rm -rf output
	rm -rf src/cluster_agent.o

bin/cluster_agent: \
  src/cluster_agent.o 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mbin/cluster_agent[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o bin/cluster_agent \
  src/cluster_agent.o \
	-Xlinker "-(" 	../../../..//lib/public/output/lib/libpublic.a \
	../../../..//lib/log/output/lib/liblog.a \
	../../../..//lib/monitor/output/lib/libmonitor.a \
	../../../..//projects/magneto/output/lib/libmagneto.a \
	../../../..//lib/conf/output/lib/libconf.a \
  -L/usr/local/lib -lpthread -lrt -lz -Xlinker "-)"
		mkdir -p ./output/bin output/include/
		cp -f bin/cluster_agent ./output/bin
		cp -rf src/* ./output/include/ &> /dev/null
		find output/ -regex ".*\.c\|.*\.cpp\|.*\.cc\|.*\.o" | xargs -i rm -rf {}

src/cluster_agent.o : src/cluster_agent.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(INCPATH) $(DEP_INCPATH) -o src/cluster_agent.o src/cluster_agent.cpp

