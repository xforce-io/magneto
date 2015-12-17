#pragma once

#include <cstddef>
#include <new>
#include <vector>
#include <list>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <assert.h>

#include <limits>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

#include <ucontext.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

#include <malloc.h>

#ifdef MEMPROFILE
#include "mem_profile.h"
#endif

#ifdef MONITOR_TYPE
#include "gmonitor.h"
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

#ifndef UNUSE
#define UNUSE(arg) { (void)(arg); }
#endif

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(type_name) \
	type_name(const type_name&); \
	void operator=(const type_name &);
#endif

#ifndef RCAST
#define RCAST reinterpret_cast
#endif

#ifndef SCAST
#define SCAST static_cast
#endif

#ifndef CCAST
#define CCAST const_cast
#endif

#ifndef likely
#define likely(x) __builtin_expect ((x), true)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect ((x), false)
#endif

/*
 * log4cplus macros, borrowed from robo
 */

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/timehelper.h>

#define LOGGER_SYS_INIT(property_file) \
    log4cplus::PropertyConfigurator::doConfigure(property_file);
#define LOGGER_SYS_INIT_DYNAMIC(watcher, property_file, check_interval_ms)  \
    log4cplus::ConfigureAndWatchThread watcher(property_file, check_interval_ms);

#define LOGGER_CLASS_DECL(logger) \
    static log4cplus::Logger logger;
#define LOGGER_CLASS_IMPL(logger, classname) \
    log4cplus::Logger classname::logger = log4cplus::Logger::getInstance(#classname);

#define LOGGER_EXTERN_DECL(logger) \
    extern  log4cplus::Logger logger;
#define LOGGER_IMPL(logger, name)  \
    log4cplus::Logger logger = log4cplus::Logger::getInstance(name);

#define LOGGER_STATIC_DECL_IMPL(logger,name) \
    static log4cplus::Logger logger = log4cplus::Logger::getInstance(name);

#define TRACE(log)  LOG4CPLUS_TRACE(xforce_logger, log)
#define DEBUG(log) LOG4CPLUS_DEBUG(xforce_logger, log)
#define NOTICE(log) LOG4CPLUS_INFO(xforce_logger, log)
#define WARN(log) LOG4CPLUS_WARN(xforce_logger, log)
#define ERROR(log)  LOG4CPLUS_ERROR(xforce_logger, log)
#define FATAL(log) LOG4CPLUS_FATAL(xforce_logger, log)

#define TRACE_LOG(logger, log)  LOG4CPLUS_TRACE(logger, log)
#define DEBUG_LOG(logger, log) LOG4CPLUS_DEBUG(logger, log)
#define NOTICE_LOG(logger, log) LOG4CPLUS_INFO(logger, log)
#define WARN_LOG(logger, log) LOG4CPLUS_WARN(logger, log)
#define ERROR_LOG(logger, log)  LOG4CPLUS_ERROR(logger, log)
#define FATAL_LOG(logger, log) LOG4CPLUS_FATAL(logger, log)

/* end of log4cplus macros */

#ifndef XFC_FAIL_HANDLE
#define XFC_FAIL_HANDLE(expr) \
  do { \
    if (unlikely(expr)) goto ERROR_HANDLE; \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_AND_SET
#define XFC_FAIL_HANDLE_AND_SET(expr, set) \
  do { \
    if (unlikely(expr)) { \
      set; \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_STDOUT
#define XFC_FAIL_HANDLE_STDOUT(expr, fmt, arg...) \
  do { \
    if(unlikely(expr)) { \
        printf(fmt, ##arg); \
        goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_WARN
#define XFC_FAIL_HANDLE_WARN(op, str) \
  do { \
    if(unlikely(op)) { \
      WARN(str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_CONTINUE
#define XFC_FAIL_HANDLE_CONTINUE(expr) \
  if (unlikely(expr)) { \
    continue; \
  }
#endif

#ifndef XFC_FAIL_HANDLE_WARN_CONTINUE
#define XFC_FAIL_HANDLE_WARN_CONTINUE(expr, str) \
  if (unlikely(expr)) { \
    WARN(str); \
    continue; \
  }
#endif

#ifndef XFC_FAIL_HANDLE_WARN_AND_SET
#define XFC_FAIL_HANDLE_WARN_AND_SET(expr, set, str) \
  do { \
    if (unlikely(expr)) { \
      set; \
      WARN(str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_WARN_LOG
#define XFC_FAIL_HANDLE_WARN_LOG(logger, op, str) \
  do { \
    if(unlikely(op)) { \
      WARN_LOG(logger, str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_WARN_LOG_CONTINUE
#define XFC_FAIL_HANDLE_WARN_LOG_CONTINUE(logger, expr, str) \
  if (unlikely(expr)) { \
    WARN_LOG(logger, str); \
    continue; \
  }
#endif

#ifndef XFC_FAIL_HANDLE_WARN_LOG_AND_SET
#define XFC_FAIL_HANDLE_WARN_LOG_AND_SET(logger, expr, set, str) \
  do { \
    if (unlikely(expr)) { \
      set; \
      WARN_LOG(logger, str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_FATAL
#define XFC_FAIL_HANDLE_FATAL(op, str) \
  do { \
    if(unlikely(op)) { \
      FATAL(str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_FATAL_CONTINUE
#define XFC_FAIL_HANDLE_FATAL_CONTINUE(expr, str) \
  if (unlikely(expr)) { \
    FATAL(str); \
    continue; \
  }
#endif

#ifndef XFC_FAIL_HANDLE_FATAL_AND_SET
#define XFC_FAIL_HANDLE_FATAL_AND_SET(expr, set, str) \
  do { \
    if (unlikely(expr)) { \
      set; \
      FATAL(str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_FATAL_LOG
#define XFC_FAIL_HANDLE_FATAL_LOG(logger, op, str) \
  do { \
    if(unlikely(op)) { \
      FATAL_LOG(logger, str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_FAIL_HANDLE_FATAL_LOG_CONTINUE
#define XFC_FAIL_HANDLE_FATAL_LOG_CONTINUE(logger, expr, str) \
  if (unlikely(expr)) { \
    FATAL_LOG(logger, str); \
    continue; \
  }
#endif

#ifndef XFC_FAIL_HANDLE_FATAL_LOG_AND_SET
#define XFC_FAIL_HANDLE_FATAL_LOG_AND_SET(logger, expr, set, str) \
  do { \
    if (unlikely(expr)) { \
      set; \
      FATAL_LOG(logger, str); \
      goto ERROR_HANDLE; \
    } \
  } while(0);
#endif

#ifndef XFC_ASSERT
#define XFC_ASSERT(expr) \
  assert(expr);
#endif

#ifdef DEBUG_MODE

  #ifndef XFC_BUG
  #define XFC_BUG(expr) \
    XFC_ASSERT(!(expr))
  #endif

#else

  #ifndef XFC_BUG
  #define XFC_BUG(expr) \
    if (unlikely(expr)) { \
      FATAL("bug_on["__FILE__":" << __LINE__ << "]"); \
    }
  #endif

#endif

#ifndef XFC_STOP
#define XFC_STOP(expr) \
  XFC_ASSERT(!(expr))
#endif

#ifndef XFC_STOP_FATAL
#define XFC_STOP_FATAL(expr, str) \
  FATAL(str); \
  XFC_BUG(expr)
#endif

#ifndef XFC_STATIC_PROFILE
#define XFC_STATIC_PROFILE(item, num) \
  do { \
    char _buf[xforce::kMaxSizeStaticProfilekey]; \
    sprintf(_buf, item"_%s_%d", __FILE__, __LINE__); \
    xforce::GMonitor::Get().Inc("static_profile", _buf, num); \
  } while (0);
#endif

#ifdef MEMPROFILE

#define XFC_NEW(member, constructor) \
  do { \
    member = ::new (std::nothrow) constructor; \
    if (xforce::MemProfile::GetFlag()) { \
      char _buf[xforce::kMaxSizeMemProfilekey]; \
      sprintf(_buf, "new_%s_%d", __FILE__, __LINE__); \
      xforce::GMonitor::Get().Inc("mem_profile", _buf, malloc_usable_size(member)); \
    } \
  } while(0);

#define XFC_NEW_DECL(member, type, constructor) \
  type* member = ::new (std::nothrow) constructor; \
  if (xforce::MemProfile::GetFlag()) { \
    char _buf[xforce::kMaxSizeMemProfilekey]; \
    sprintf(_buf, "new_%s_%d", __FILE__, __LINE__); \
    xforce::GMonitor::Get().Inc("mem_profile", _buf, malloc_usable_size(member)); \
  }

#define XFC_NEW_AND_RET(type) \
  type* member = ::new (std::nothrow) type; \
  if (xforce::MemProfile::GetFlag()) { \
    char _buf[xforce::kMaxSizeMemProfilekey]; \
    sprintf(_buf, "new_%s_%d", __FILE__, __LINE__); \
    xforce::GMonitor::Get().Inc("mem_profile", _buf, malloc_usable_size(member)); \
  } \
  return member; \

#define XFC_MALLOC(member, type, size) \
  do { \
    member = reinterpret_cast<type>(::malloc(size)); \
    if (xforce::MemProfile::GetFlag()) { \
      char _buf[xforce::kMaxSizeMemProfilekey]; \
      sprintf(_buf, "new_%s_%d", __FILE__, __LINE__); \
      xforce::GMonitor::Get().Inc("mem_profile", _buf, malloc_usable_size(member)); \
    } \
  } while(0);

#define XFC_REALLOC(new_member, old_member, type, size) \
  do { \
    new_member = reinterpret_cast<type>(::realloc(old_member, size)); \
  } while(0);

#define XFC_DELETE(member) \
  do { \
    if (likely(NULL!=member)) { \
      if (xforce::MemProfile::GetFlag()) { \
        char _buf[xforce::kMaxSizeMemProfilekey]; \
        sprintf(_buf, "delete_%s_%d", __FILE__, __LINE__); \
        xforce::GMonitor::Get().Inc("mem_profile", _buf, malloc_usable_size(member)); \
      } \
      delete member; \
      member=NULL; \
    } \
  } while(0);

#define XFC_DELETE_ARRAY(member) \
  do { \
    if (likely(NULL!=member)) { \
      if (xforce::MemProfile::GetFlag()) { \
        char _buf[xforce::kMaxSizeMemProfilekey]; \
        sprintf(_buf, "delete_%s_%d", __FILE__, __LINE__); \
        xforce::GMonitor::Get().Inc("mem_profile", _buf, malloc_usable_size(member)); \
      } \
      delete [] member; \
      member=NULL; \
    } \
  } while(0);

#define XFC_FREE(member) \
  do { \
    if (likely(NULL!=member)) { \
      if (xforce::MemProfile::GetFlag()) { \
        char _buf[xforce::kMaxSizeMemProfilekey]; \
        sprintf(_buf, "delete_%s_%d", __FILE__, __LINE__); \
        xforce::GMonitor::Get().Inc("mem_profile", _buf, malloc_usable_size(member)); \
      } \
      ::free(member); \
      member=NULL; \
    } \
  } while(0);

#else

#define XFC_NEW(member, constructor) \
  do { \
    member = ::new (std::nothrow) constructor; \
  } while(0);

#define XFC_NEW_DECL(member, type, constructor) \
  type* member = ::new (std::nothrow) constructor; \

#define XFC_NEW_AND_RET(type) \
  return ::new (std::nothrow) type; \

#define XFC_MALLOC(member, type, size) \
  do { \
    member = reinterpret_cast<type>(::malloc(size)); \
  } while(0);

#define XFC_REALLOC(new_member, old_member, type, size) \
  do { \
    new_member = reinterpret_cast<type>(::realloc(old_member, size)); \
  } while(0);

#define XFC_DELETE(member) \
  do { \
    if (likely(NULL!=member)) { \
      delete member; \
      member=NULL; \
    } \
  } while(0);

#define XFC_DELETE_ARRAY(member) \
  do { \
    if (likely(NULL!=member)) { \
      delete [] member; \
      member=NULL; \
    } \
  } while(0);

#define XFC_FREE(member) \
  do { \
    if(likely(NULL!=member)) { \
      ::free(member); \
      member=NULL; \
    } \
  } while(0);

#endif

#ifndef XFC_RAII_CHECK
#define XFC_RAII_CHECK(ret) \
  if (unlikely(!init_)) { return ret; }
#endif

#ifndef XFC_RAII_INIT
#define XFC_RAII_INIT(ret) \
  if (unlikely(!init_) && unlikely(!Init_())) { return ret; }
#endif

#ifndef XFC_RAII_SUPER_INIT
#define XFC_RAII_SUPER_INIT(ret) \
  if (unlikely(!Super::init_) && unlikely(!Init_())) { return ret; }
#endif

#ifndef XFC_CONCAT
#define XFC_CONCAT(X, Y) X##Y
#endif

#ifndef XFC_STATIC_ASSERT
#define XFC_STATIC_ASSERT(expr) \
  enum { \
    Assert = sizeof(CompileTimeError<(expr)!=0>), \
  };
#endif

#ifndef XFC_STATIC_ASSERT_MSG
#define XFC_STATIC_ASSERT_MSG(expr, msg) \
  enum { \
    XFC_CONCAT(ERROR_##msg, __LINE__) = \
      sizeof(CompileTimeError<expr != 0 >) \
  }
#endif
/**/

/*
 * CAS
 */
#ifndef CAS
#define CAS(ptr, oldval, newval) \
  __sync_val_compare_and_swap(ptr, oldval, newval)
#endif

#ifndef CAS_bool
#define CAS_bool(ptr, oldval, newval) \
  __sync_bool_compare_and_swap(ptr, oldval, newval)
#endif

/*
 * memory barriers
 */
#ifndef MB
#define MB() \
  __asm__ __volatile__("mfence":::"memory");
#endif

namespace xforce {

LOGGER_EXTERN_DECL(xforce_logger);

class NoneType {};

static const size_t kMaxSizeStaticProfilekey=512;
static const size_t kMaxSizeMemProfilekey=512;

/*
 * static assert, from loki
 */
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

}
