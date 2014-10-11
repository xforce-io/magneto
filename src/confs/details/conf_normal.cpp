#include "../conf_normal.h"

namespace magneto {

const std::string ConfNormal::kFilenameNormalConf = "normal.conf";

ConfNormal::ConfNormal() :
  client_keepalive_sec_(0),
  long_conn_keepalive_sec_(0) {}

bool ConfNormal::Init(const std::string& filedir) {
  const JsonType* listen_addrs;
  JsonType::DictType::const_iterator iter;
  std::stringstream ss;

  JsonType* conf_normal = JsonType::CreateConf(filedir+"/"+kFilenameNormalConf);
  MAG_FAIL_HANDLE_FATAL(NULL==conf_normal || !conf_normal->IsDict(), 
      "fail_init_service_conf dir[" << filedir << "]")

  conf_normal->DumpJson(ss);
  NOTICE("init_conf_normal[" << ss.str() << "]");

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["num_schedulers"].IsInt()
      || (*conf_normal)["num_schedulers"].AsInt() <= 0, 
      "fail_get_num_schedulers" )
  num_schedulers_ = (*conf_normal)["num_schedulers"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["num_agents"].IsInt()
      || (*conf_normal)["num_agents"].AsInt() <= 0, 
      "fail_get_num_agents" )
  num_agents_ = (*conf_normal)["num_agents"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["client_keepalive_sec"].IsInt()
      || (*conf_normal)["client_keepalive_sec"].AsInt() < 0, 
      "fail_get_client_keepalive_sec" )
  client_keepalive_sec_ = (*conf_normal)["client_keepalive_sec"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["max_num_clients_keepalive"].IsInt()
      || (*conf_normal)["max_num_clients_keepalive"].AsInt() < 0, 
      "fail_get_max_num_clients_keepalive" )
  max_num_clients_keepalive_ = (*conf_normal)["max_num_clients_keepalive"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["long_conn_keepalive_sec"].IsInt()
      || (*conf_normal)["long_conn_keepalive_sec"].AsInt() < 0, 
      "fail_get_long_conn_keepalive_sec" )
  long_conn_keepalive_sec_ = (*conf_normal)["long_conn_keepalive_sec"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["ctimeo_ms"].IsInt()
      || (*conf_normal)["ctimeo_ms"].AsInt() < 0, 
      "fail_get_ctimeo_ms" )
  ctimeo_ms_ = (*conf_normal)["ctimeo_ms"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["wtimeo_ms"].IsInt()
      || (*conf_normal)["wtimeo_ms"].AsInt() < 0, 
      "fail_get_wtimeo_ms" )
  wtimeo_ms_ = (*conf_normal)["wtimeo_ms"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["rtimeo_ms"].IsInt()
      || (*conf_normal)["rtimeo_ms"].AsInt() < 0, 
      "fail_get_rtimeo_ms" )
  rtimeo_ms_ = (*conf_normal)["rtimeo_ms"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["listen_addrs"].IsDict(), 
      "fail_get_listen_addrs" )

  listen_addrs_.clear();

  listen_addrs = &((*conf_normal)["listen_addrs"]);
  for (iter = listen_addrs->AsDict().begin();
      iter != listen_addrs->AsDict().end();
      ++iter) {
    ListenAddr listen_addr;
    bool ret = ConfHelper::ParseServiceTag(
        iter->first, 
        listen_addr.name,
        listen_addr.category);
    MAG_FAIL_HANDLE_FATAL(!ret || !iter->second.IsStr(), "invalid_listen_addr[" << iter->first << "]")

    listen_addr.addr.Assign(iter->second.AsStr());
    listen_addrs_.push_back(listen_addr);
  }

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["size_scheduler_mailbox"].IsInt()
      || (*conf_normal)["size_scheduler_mailbox"].AsInt() <= 0, 
      "fail_get_size_scheduler_mailbox" )
  size_scheduler_mailbox_ = (*conf_normal)["size_scheduler_mailbox"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["size_agent_mailbox"].IsInt()
      || (*conf_normal)["size_agent_mailbox"].AsInt() <= 0, 
      "fail_get_size_agent_mailbox" )
  size_agent_mailbox_ = (*conf_normal)["size_agent_mailbox"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["size_stack"].IsInt()
      || (*conf_normal)["size_stack"].AsInt() <= 0, 
      "fail_get_size_stack" )
  size_stack_ = (*conf_normal)["size_stack"].AsInt();

  MAG_FAIL_HANDLE_FATAL(
      !(*conf_normal)["low_latency"].IsInt(),
      "fail_get_low_latency" )
  low_latency_ = (*conf_normal)["low_latency"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["default_long_conns"].IsInt()
      || (*conf_normal)["default_long_conns"].AsInt() < 0, 
      "fail_get_default_long_conns" )
  default_long_conns_ = (*conf_normal)["default_long_conns"].AsInt();

  MAG_FAIL_HANDLE_FATAL( 
      !(*conf_normal)["default_weight"].IsInt()
      || (*conf_normal)["default_weight"].AsInt() <= 0, 
      "fail_get_default_weight" )
  default_weight_ = (*conf_normal)["default_weight"].AsInt();

  MAG_DELETE(conf_normal)
  return true;

  ERROR_HANDLE:
  MAG_DELETE(conf_normal)
  return false;
}

}
