#pragma once

#include "../model.h"
#include "conf_normal.h"

namespace magneto {

class ConfServices {
 public:
  typedef std::tr1::unordered_set<Remote, Remote::HashRemote, Remote::EqRemote> Remotes;
  typedef std::tr1::unordered_map<std::string, Service*> Services;
  typedef std::tr1::unordered_map<std::string, ServicesSet*> ServicesSets;
  typedef std::tr1::unordered_map< std::string, std::vector<std::string> > SetToServices;
 
 private:
  static const std::string kFilenameServicesConf;

 public:
  ConfServices();

  bool Init(const std::string& filedir, const ConfNormal& conf_normal);
  const Remotes& GetRemotes() const { return remotes_; }
  const Services& GetServices() const { return services_; }
  const ServicesSets& GetServicesSets() const { return services_sets_; }

  inline const Service* GetService(const std::string& service) const;
  inline const ServicesSet* GetServicesSet(const std::string& services_set) const;
  inline const std::vector<std::string>* GetServiceNames(const std::string services_set) const;

  virtual ~ConfServices();
 
 private: 
  bool BuildServicesSets_(const JsonType& conf);
  bool BuildServices_(const JsonType& conf);
  bool BuildService_(
      size_t no,
      const std::string& service_tag,
      const JsonType& service_desc, 
      Service& service);

  bool BuildRemote_(const JsonType& conf, Remote& remote);
 
 private:
  const ConfNormal* conf_normal_;
  JsonType* conf_services_;

  Remotes remotes_;
  Services services_;
  ServicesSets services_sets_;
  SetToServices set_to_services_;
};

const Service* ConfServices::GetService(const std::string& service) const {
  Services::const_iterator iter = services_.find(service);
  return services_.end() != iter ? iter->second : NULL;
}

const ServicesSet* ConfServices::GetServicesSet(const std::string& services_set) const {
  ServicesSets::const_iterator iter = services_sets_.find(services_set);
  return services_sets_.end() != iter ? iter->second : NULL;
}

const std::vector<std::string>* ConfServices::GetServiceNames(const std::string services_set) const {
  SetToServices::const_iterator iter = set_to_services_.find(services_set);
  return set_to_services_.end() != iter ? &(iter->second) : NULL;
}

}
