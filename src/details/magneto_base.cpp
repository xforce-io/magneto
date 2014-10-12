#include <signal.h>

#include "../magneto_base.h"

#include "../confs/confs.h"
#include "../schedulers/schedulers.h"
#include "../agents/agents.h"

#ifdef MONITOR
#include "lib/public/public.h"
#endif

namespace magneto {

MagnetoBase::MagnetoBase() :
  confs_(NULL),
  schedulers_(NULL),
  agents_(NULL),
  io_basic_(NULL) {}

bool MagnetoBase::Init(
    const std::string& conf_service_dir,
    const ReqHandler& req_handler,
    const RoutineItems* routine_items,
    void* args,
    bool& end) {
  end_=&end;

  InitSignals_();

  int ret;
  MAG_NEW(confs_, Confs)
  ret = confs_->Init(conf_service_dir);
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_load_versioned_conf_services[" << conf_service_dir << "]")

  NOTICE("succ_init_conf[" << conf_service_dir << "]");

  MAG_NEW(schedulers_, Schedulers)
  MAG_NEW(agents_, Agents)

  ret = schedulers_->Init(*confs_, *agents_, req_handler, routine_items, args);
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_init_schedulers")

  ret = agents_->Init(*confs_, *schedulers_);
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_init_agents")

  MAG_NEW(io_basic_, IOBasic)
  ret = io_basic_->Init(*confs_, *agents_);
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_init_io_basic")

  ret = agents_->Start();
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_start_agents")

  ret = schedulers_->Start();
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_start_schedulers")
  return true;

  ERROR_HANDLE:
  MAG_DELETE(io_basic_)
  MAG_DELETE(agents_)
  MAG_DELETE(schedulers_)
  MAG_DELETE(confs_)
  return false;
}

void MagnetoBase::Stop() {
  while (!*end_) {
    sleep(1);
  }

  if (NULL!=agents_) {
    agents_->Stop();
  }

  if (NULL!=schedulers_) {
    schedulers_->Stop();
  }
}

MagnetoBase::~MagnetoBase() {
  Stop();
  MAG_DELETE(io_basic_)
  MAG_DELETE(agents_)
  MAG_DELETE(schedulers_)
  MAG_DELETE(confs_)
}

void MagnetoBase::InitSignals_() {
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sigaction(SIGPIPE, &sa, 0);
  sigaction(SIGHUP, &sa, 0);
  sigaction(SIGCHLD, &sa, 0);
}

}
