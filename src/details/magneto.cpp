#include "../magneto.h"
#include "../magneto_base.h"

namespace xforce { namespace magneto {

Magneto::Magneto() :
  magneto_base_(NULL) {}

bool Magneto::Init(
    const std::string& conf_service_dir,
    const ReqHandler& req_handler,
    const RoutineItems* routine_items,
    void* args,
    bool& end) {
  XFC_NEW(magneto_base_, MagnetoBase)
  return magneto_base_->Init(conf_service_dir, req_handler, routine_items, args, end);
}

bool Magneto::Start() {
    return magneto_base_->Start();
}

const std::vector<std::string>* Magneto::GetServiceNames(const std::string& services_set) {
  return magneto_base_->GetServiceNames(services_set);
}

int Magneto::Write(
    const std::string& services_set,
    const Bufs& bufs,
    time_t timeo_ms,
    Errors& errors) {
  return magneto_base_->Write(services_set, bufs, timeo_ms, errors);
}

int Magneto::Read(
    const std::string& services_set,
    time_t timeo_ms,
    Responses& responses) {
  return magneto_base_->Read(services_set, timeo_ms, responses);
}

int Magneto::ParaTalks(
    const std::string& services_set,
    const Bufs& bufs,
    time_t timeo_ms,
    Responses& responses) {
  return magneto_base_->ParaTalks(services_set, bufs, timeo_ms, responses);
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
    ProtocolRead*& protocol_read) {
  return magneto_base_->Read(service, timeo_ms, protocol_read);
}

int Magneto::SimpleTalk(
    const std::string& service,
    const Buf& buf,
    time_t timeo_ms,
    ProtocolRead*& protocol_read) {
  return magneto_base_->SimpleTalk(service, buf, timeo_ms, protocol_read);
}

int Magneto::Write(
    const std::string& service,
    const std::string& remote,
    const Buf& buf,
    time_t timeo_ms) {
  return magneto_base_->Write(service, remote, buf, timeo_ms);
}

int Magneto::Read(
    const std::string& service,
    const std::string& remote, 
    time_t timeo_ms, 
    ProtocolRead*& protocol_read) {
  return magneto_base_->Read(service, remote, timeo_ms, protocol_read);
}

int Magneto::SimpleTalk(
    const std::string& service,
    const std::string& remote,
    const Buf& buf,
    time_t timeo_ms,
    ProtocolRead*& protocol_read) {
  return magneto_base_->SimpleTalk(service, remote, buf, timeo_ms, protocol_read);
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
  XFC_DELETE(magneto_base_)
}

}}
