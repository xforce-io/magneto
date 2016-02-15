#include "../conf_services.h"
#include "../../protocols/pool_protocols.h"

namespace xforce { namespace magneto {

const std::string ConfServices::kFilenameServicesConf = "services.conf";

ConfServices::ConfServices() :
  conf_services_(NULL) {}

bool ConfServices::Init(const std::string& filedir, const ConfNormal& conf_normal) {
  bool ret;
  std::stringstream ss;
  
  conf_normal_ = &conf_normal;

  std::string filepath = filedir+"/"+kFilenameServicesConf;
  FILE* fp = fopen(filepath.c_str(), "r");
  if (NULL==fp) {
    return true;
  }
  fclose(fp);

  conf_services_ = JsonType::CreateConf(filepath);
  XFC_FAIL_HANDLE_FATAL(NULL==conf_services_ || !conf_services_->IsDict(), 
      "fail_init_service_conf dir[" << filedir.c_str() << "]")

  conf_services_->DumpJson(ss);
  NOTICE("init_conf_services[" << ss.str() << "]");

  ret = BuildServices_((*conf_services_)["services"]);
  XFC_FAIL_HANDLE_FATAL(!ret, "fail_build_services");

  ret = BuildServicesSets_((*conf_services_)["services_sets"]);
  XFC_FAIL_HANDLE_FATAL(!ret, "fail_build_services_sets");
  return true;

  ERROR_HANDLE:
  return false;
}

ConfServices::~ConfServices() {
  XFC_DELETE(conf_services_)

  ServicesSets::iterator iter_services_sets;
  for (
      iter_services_sets = services_sets_.begin();
      iter_services_sets != services_sets_.end();
      ++iter_services_sets) {
    XFC_DELETE(iter_services_sets->second)
  }

  Services::iterator iter_services;
  for (iter_services = services_.begin(); iter_services != services_.end(); ++iter_services) {
    XFC_DELETE(iter_services->second);
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
      
    XFC_NEW_DECL(services_set, ServicesSet, ServicesSet)
    for (size_t i=0; i < services_wt.AsList().size(); ++i) {
      if (!services_wt[i].IsStr()) {
        XFC_DELETE(services_set)
        return false;
      }

      const std::string& service_name = services_wt[i].AsStr();
      Services::const_iterator iter = services_.find(service_name);
      if (services_.end() == iter) {
        WARN("no_service_defined[" << service_name << "]");
        XFC_DELETE(services_set)
        return false;
      }
      services_set->push_back(iter->second);
      ret_insert.first->second.push_back(service_name);
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
    XFC_NEW_DECL(service, Service, Service)
    bool ret = BuildService_(no_service++, iter->first, iter->second, *service);
    if (!ret) {
      XFC_DELETE(service)
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
  XFC_FAIL_HANDLE(!ret)

  XFC_FAIL_HANDLE(!service_desc.IsDict() || !service_desc["remotes"].IsList() )

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
    XFC_FAIL_HANDLE(!ret)

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

  if (conf["name"].IsStr() && conf["name"].AsStr() != "") {
    remote.name = conf["name"].AsStr();
  } else {
    remote.name = conf["addr"].AsStr();
  }

  bool ret = remote.addr.Assign(conf["addr"].AsStr());
  if (!ret) return false;

  if (conf["weight"].IsInt()) {
    XFC_FAIL_HANDLE(conf["weight"].AsInt() < 0)
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

  iter = remotes_.find(remote.name);
  if (remotes_.end() == iter) {
    remotes_.insert(std::make_pair(remote.name, remote));
    XFC_FAIL_HANDLE_FATAL(
        remotes_.size() > LocalLimits::kNumRemotes,
        "too_many_remotes[" << remotes_.size() << "|" << LocalLimits::kNumRemotes << "]")
  } else {
    XFC_FAIL_HANDLE_FATAL(
        remotes_.size() > LocalLimits::kNumRemotes,
        "dup_remotes[" << remote.name << "]")
  }
  return true;

  ERROR_HANDLE:
  return false;
}

}}
