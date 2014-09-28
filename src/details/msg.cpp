#include "../msg.h"
#include "../biz_procedure.h"

namespace magneto {

void MsgSession::BuildForSession(
    ucontext_t& biz_ctx_arg,
    BizProcedure& biz_procedure_arg,
    std::vector<Talk>& talks_arg,
    time_t timeo_ms_arg,
    int error_arg) {
  biz_ctx = &biz_ctx_arg;
  biz_procedure = &biz_procedure_arg;
  talks = &talks_arg;
  timeo_ms = timeo_ms_arg;
  error = error_arg;
}

void MsgDestruct::BuildForDestruct(
    int id_procedure_arg,
    int fd_client_arg,
    time_t fd_client_starttime_sec_arg,
    ProtocolRead* protocol_read_arg,
    const BizProcedure& biz_procedure) {
  const BizProcedure::ServiceCache& service_cache = biz_procedure.GetServiceCache();

  id_procedure = id_procedure_arg;
  fd_client = fd_client_arg;
  fd_client_starttime_sec = fd_client_starttime_sec_arg;
  protocol_read = protocol_read_arg;
  BizProcedure::ServiceCache::const_iterator iter;
  if (service_cache.size() <= MsgDestruct::kSizeSmallCache) {
    small_cache.num = service_cache.size();
    size_t num_fds=0;
    for (iter = service_cache.begin(); iter != service_cache.end(); ++iter) {
      small_cache.small_cache[num_fds++] = iter->second;
    }
    big_cache = NULL;
  } else {
    typedef std::vector< std::pair<int, const Remote*> > BigCache;
    MAG_NEW(big_cache, BigCache)
    for (iter = service_cache.begin(); iter != service_cache.end(); ++iter) {
      big_cache->push_back(iter->second);
    }
  }
}

}
