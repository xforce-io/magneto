#include "../conf_services.h"
#include "../../protocols/pool_protocols.h"

namespace magneto {

const std::string ConfServices::kFilenameServicesConf = "services.conf";

ConfServices::ConfServices() :
  conf_services_(NULL) {}

bool ConfServices::Init(const std::string& filedir, const ConfNormal& conf_normal) {
  bool ret;
  std::stringstream ss;
  
  conf_normal_ = &conf_normal;

  conf_services_ = JsonType::CreateConf(filedir+"/"+kFilenameServicesConf);
  MAG_FAIL_HANDLE_FATAL(NULL==conf_services_ || !conf_services_->IsDict(), 
      "fail_init_service_conf dir[" << filedir.c_str() << "]")

  conf_services_->DumpJson(ss);
  NOTICE("init_conf_services[" << ss.str() << "]");

  ret = BuildServices_((*conf_services_)["services"]);
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_build_services");

  ret = BuildServicesSets_((*conf_services_)["services_sets"]);
  MAG_FAIL_HANDLE_FATAL(!ret, "fail_build_services_sets");
  return true;

  ERROR_HANDLE:
  return false;
}

ConfServices::~ConfServices() {
  MAG_DELETE(conf_services_)

  ServicesSets::iterator iter_services_sets;
  for (
      iter_services_sets = services_sets_.begin();
      iter_services_sets != services_sets_.end();
      ++iter_services_sets) {
    MAG_DELETE(iter_services_sets->second)
  }

  Services::iterator iter_services;
  for (iter_services = services_.begin(); iter_services != services_.end(); ++iter_services) {
    MAG_DELETE(iter_services->second);
  }
}

bool ConfServices::BuildServicesSets_(const JsonType& conf) {
  if (!conf.IsDict()) return false;

  const JsonType::DictType& services_sets = conf.AsDict();
  JsonType::DictType::const_iterator iter;
  for (iter = services_sets.begin(); iter != services_sets.end(); ++iter) {
    const JsonType& services_wt = iter->second;
    if (!services_wt.IsList()) return false;

    std::pair<SetToServices::iterator, bool> ret_insert = 
      set_to_services_.insert(std::make_pair(iter->first, std::vector<std::string>()));
      
    MAG_NEW_DECL(services_set, ServicesSet, ServicesSet)
    for (size_t i=0; i < services_wt.AsList().size(); ++i) {
      if (!services_wt[i].IsStr()) {
        MAG_DELETE(services_set)
        return false;
      }

      const std::string& service_name = services_wt[i].AsStr();
      Services::const_iterator iter = services_.find(service_name);
      if (services_.end() == iter) {
        WARN("no_service_defined[" << service_name << "]");
        MAG_DELETE(services_set)
        return false;
      }
      services_set->push_back(iter->second);
      ret_insert.first->push_back(service_name);
    }
    services_sets_.insert(std::make_pair(iter->first, services_set));
  }
  return true;
}

bool ConfServices::BuildServices_(const JsonType& conf) {
  if (!conf.IsDict()) return false;

  size_t no_service=0;
  const JsonType::DictType& services = conf.AsDict();
  JsonType::DictType::const_iterator iter;
  for (iter = services.begin(); iter != services.end(); ++iter) {
    MAG_NEW_DECL(service, Service, Service)
    bool ret = BuildService_(no_service++, iter->first, iter->second, *service);
    if (!ret) {
      MAG_DELETE(service)
      return false;
    }

    services_.insert(std::make_pair(service->name, service));
  }
  return true;
}

bool ConfServices::BuildService_(
    size_t no,
    const std::string& service_tag, 
    const JsonType& service_desc, 
    Service& service) {
  service.no = no;
  bool ret = ConfHelper::ParseServiceTag(
      service_tag, 
      service.name, 
      service.protocol_category);
  MAG_FAIL_HANDLE(!ret)

  MAG_FAIL_HANDLE(!service_desc.IsDict() || !service_desc["remotes"].IsList() )

  if (!service_desc["strategy"].IsStr()) {
    service.service_strategy = ServiceStrategy::kDefaultStrategy;
  } else {
    service.service_strategy = ServiceStrategy::GetServiceStrategy(service_desc["strategy"].AsStr());
    if (ServiceStrategy::kInvalid == service.service_strategy) {
      service.service_strategy = ServiceStrategy::kDefaultStrategy;
    }
  }

  service.weight_all = 0;
  service.remotes.resize(service_desc["remotes"].AsList().size());
  for (size_t i=0; i < service_desc["remotes"].AsList().size(); ++i) {
    ret = BuildRemote_(service_desc["remotes"][i], service.remotes[i]);
    MAG_FAIL_HANDLE(!ret)

    service.weight_all += service.remotes[i].weight;
  }

  if (service_desc["retry"].IsInt() && service_desc["retry"].AsInt() >= 0) {
    service.retry = service_desc["retry"].AsInt();
  } else {
    service.retry = 0;
  }
  return true;

  ERROR_HANDLE:
  FATAL("fail_build_service[" << service_tag << "]");
  return false;
}

bool ConfServices::BuildRemote_(const JsonType& conf, Remote& remote) {
  Remotes::iterator iter;

  if ( !conf.IsDict() || !conf["addr"].IsStr() ) return false;

  bool ret = remote.addr.Assign(conf["addr"].AsStr());
  if (!ret) return false;

  if (conf["weight"].IsInt()) {
    MAG_FAIL_HANDLE(conf["weight"].AsInt() < 0)
    remote.weight = conf["weight"].AsInt();
  } else {
    remote.weight = conf_normal_->GetDefaultWeight();
  }

  if (conf["long_conns"].IsInt()) {
    remote.long_conns = conf["long_conns"].AsInt();
  } else {
    remote.long_conns = conf_normal_->GetDefaultLongConns();
  }

  if (conf["ctimeo_ms"].IsInt()) {
    remote.ctimeo_ms = conf["ctimeo_ms"].AsInt();
  } else {
    remote.ctimeo_ms = conf_normal_->GetCtimeoMs();
  }

  if (conf["rtimeo_ms"].IsInt()) {
    remote.rtimeo_ms = conf["rtimeo_ms"].AsInt();
  } else {
    remote.rtimeo_ms = conf_normal_->GetRtimeoMs();
  }

  if (conf["wtimeo_ms"].IsInt()) {
    remote.wtimeo_ms = conf["wtimeo_ms"].AsInt();
  } else {
    remote.wtimeo_ms = conf_normal_->GetWtimeoMs();
  }

  iter = remotes_.find(remote);
  if (remotes_.end() == iter) {
    remotes_.insert(remote);
    MAG_FAIL_HANDLE_FATAL(
        remotes_.size() > LocalLimits::kNumRemotes,
        "too_many_remotes[" << remotes_.size() << "|" << LocalLimits::kNumRemotes << "]")
  }  
  return true;

  ERROR_HANDLE:
  return false;
}

}
