#pragma once

#include "public.h"
#include "../protocols/pool_protocols.h"
#include "../model.h"

namespace magneto {

class ConfNormal {
 public:
  typedef std::vector<ListenAddr> ListenAddrs;
  
 private: 
  static const std::string kFilenameNormalConf;

 public:
  ConfNormal();

  bool Init(const std::string& filedir);

  size_t GetNumSchedulers() const { return num_schedulers_; }
  size_t GetNumAgents() const { return num_agents_; }
  time_t GetClientKeepaliveSec() const { return client_keepalive_sec_; }
  size_t GetMaxNumClientsKeepalive() const { return max_num_clients_keepalive_; }
  time_t GetLongConnKeepaliveSec() const { return long_conn_keepalive_sec_; }
  time_t GetCtimeoMs() const { return ctimeo_ms_; }
  time_t GetWtimeoMs() const { return wtimeo_ms_; }
  time_t GetRtimeoMs() const { return rtimeo_ms_; }
  const ListenAddrs& GetListenAddrs() const { return listen_addrs_; }
  size_t GetSizeSchedulerMailbox() const { return size_scheduler_mailbox_; }
  size_t GetSizeAgentMailbox() const { return size_agent_mailbox_; }
  size_t GetSizeStack() const { return size_stack_; }
  bool GetLowLatency() const { return low_latency_; }
  size_t GetDefaultLongConns() const { return default_long_conns_; }
  size_t GetDefaultWeight() const { return default_weight_; }
 
 private:
  size_t num_schedulers_;
  size_t num_agents_;
  time_t client_keepalive_sec_;
  size_t max_num_clients_keepalive_;
  time_t long_conn_keepalive_sec_;
  time_t ctimeo_ms_;
  time_t wtimeo_ms_;
  time_t rtimeo_ms_;
  ListenAddrs listen_addrs_;
  size_t size_scheduler_mailbox_;
  size_t size_agent_mailbox_;
  size_t size_stack_;
  bool low_latency_;
  size_t default_long_conns_;
  size_t default_weight_;
};

}
