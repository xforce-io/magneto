CC=gcc
CXX=g++
CXXFLAGS=-g -W -Wall -Dprivate=public  -Dprotected=public -Wno-deprecated -Wno-invalid-offsetof -fno-builtin-memcmp -DONLINE_TEST -DUT_TEST -std=c++11 -DDEBUG_MODE -DMONITOR -O2
CFLAGS=-g -pipe -W -Wall -fPIC
CPPFLAGS=-D_GNU_SOURCE -D__64BIT__ -D__STDC_LIMIT_MACROS -DVERSION=\'0.0.0.0\'
INCPATH= -I../../../../../../../../ -I$(XLIB_DIR)/include/ -I/usr/local/include/ -I../src/ -I../../../../../../../..//projects/magneto/output/include -I../../../../../../../..//lib/public/output/include -I../../../../../../../..//lib/log/output/include -I../../../../../../../..//lib/monitor/output/include -I../../../../../../../..//lib/conf/output/include

.PHONY:all
all: check_md5 bin/agent 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mall[0m']"
	@echo "make all done"

COMAKE_MD5=cbc9168129360921939a28d657dddc31  COMAKE

.PHONY:check_md5
check_md5:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mcheck_md5[0m']"
	@echo "$(COMAKE_MD5)">.comake2.md5
	@md5sum -c --status .comake2.md5
	@rm -f .comake2.md5

.PHONY:clean
clean:
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40clean[0m']"
	rm -rf bin/agent
	rm -rf output
	rm -rf src/main.o

bin/agent: \
  src/main.o 
	@echo "[[1;32;40mCOMAKE:BUILD[0m][Target:'[1;32;40mbin/agent[0m']"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o bin/agent \
  src/main.o \
	-Xlinker "-(" 	../../../../../../../..//projects/magneto/output/lib/libmagneto.a \
	../../../../../../../..//lib/public/output/lib/libpublic.a \
	../../../../../../../..//lib/log/output/lib/liblog.a \
	../../../../../../../..//lib/monitor/output/lib/libmonitor.a \
	../../../../../../../..//lib/conf/output/lib/libconf.a \
  -L$(XLIB_DIR)/lib/ -L/usr/local/lib/ -Llib/ -lpthread -lrt -Xlinker "-)"
		mkdir -p ./output/bin output/include/
		cp -f bin/agent ./output/bin
		cp -rf src/* ./output/include/ &> /dev/null
		find output/ -regex ".*\.c\|.*\.cpp\|.*\.cc\|.*\.o" | xargs -i rm -rf {}

src/main.o : src/main.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(INCPATH) $(DEP_INCPATH) -o src/main.o src/main.cpp

