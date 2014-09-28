#include "../magneto.h"
#include "../magneto_base.h"

namespace magneto {

Magneto::Magneto() :
  magneto_base_(NULL) {}

bool Magneto::Init(
    const std::string& conf_service_dir,
    const ReqHandler& req_handler,
    const RoutineItems* routine_items,
    void* args,
    bool& end) {
  MAG_NEW(magneto_base_, MagnetoBase)
  return magneto_base_->Init(conf_service_dir, req_handler, routine_items, args, end);
}

int Magneto::Write(
    const std::string& services_set,
    const Bufs& bufs,
    time_t timeo_ms,
    std::vector<int>& errors) {
  return magneto_base_->Write(services_set, bufs, timeo_ms, errors);
}

int Magneto::Read(
    const std::string& services_set,
    time_t timeo_ms,
    Responses& results) {
  return magneto_base_->Read(services_set, timeo_ms, results);
}

int Magneto::Talks(
    const std::string& services_set,
    const std::vector<const Buf*>& bufs,
    time_t timeo_ms,
    Responses& results) {
  return magneto_base_->Talks(services_set, bufs, timeo_ms, results);
}

int Magneto::Write(
    const std::string& service, 
    const Buf& buf, 
    time_t timeo_ms) {
  return magneto_base_->Write(service, buf, timeo_ms);
}

int Magneto::Read(
    const std::string& service, 
    time_t timeo_ms, 
    ProtocolRead*& protocol) {
  return magneto_base_->Read(service, timeo_ms, protocol);
}

int Magneto::SimpleTalk(
    const std::string& service,
    const Buf& buf,
    time_t timeo_ms,
    ProtocolRead*& protocol) {
  return magneto_base_->SimpleTalk(service, buf, timeo_ms, protocol);
}

int Magneto::WriteBack(const Buf& buf, time_t timeo_ms) {
  return magneto_base_->WriteBack(buf, timeo_ms);
}

void Magneto::FreeTalks() {
  return magneto_base_->FreeTalks();
}

void Magneto::Stop() { 
  magneto_base_->Stop(); 
}

Magneto::~Magneto() {
  MAG_DELETE(magneto_base_)
}

}
