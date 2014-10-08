#include "projects/magneto/magneto/magneto.h"

using namespace magneto;

Magneto::Magneto* magneto;

void RapidHandler(const ProtocolRead& protocol_read, void* args) {
  UNUSE(args)

  if (Protocol::kRapid == protocol_read.GetCategory()) {
    const ProtocolReadRapid& protocol_read_rapid = SCAST<const ProtocolReadRapid&>(protocol_read);
    if ( 1 != protocol_read_rapid.Header()->size ) {
      WARN("ret[" << protocol_read_rapid.Len() << "]");
    }

    Magneto::Magneto::Buf buf = std::make_pair(protocol_read_rapid.Data(), 1);
    std::vector<const Magneto::Magneto::Buf*> bufs;
    bufs.push_back(&buf);
    bufs.push_back(&buf);
    std::vector<std::pair<int, ProtocolRead*>> results;
    char resp=0;
    int ret;
    NOTICE("[%s]", protocol_read.GetServiceName().c_str());
    if ( "plus_and_minus" == protocol_read.GetServiceName() ) {
      ret = magneto->Talks("plus_and_minus", bufs, 100, results);
    } else if ( "plus_and_plus" == protocol_read.GetServiceName() ) {
      ret = magneto->Talks("plus_and_plus", bufs, 100, results);
    } else if ( "minus_and_minus" == protocol_read.GetServiceName() ) {
      ret = magneto->Talks("minus_and_minus", bufs, 100, results);
    } else {
      ret=0;
      FATAL("unknown_service[" << protocol_read.GetServiceName() << "]");
    }

    if (0==ret) {
      for (size_t i=0; i < results.size(); ++i) {
        resp += ( 0 == results[i].first ? results[i].second->Data()[0] : 0);
      }
    } else {
      resp=0;
    }

    buf = std::make_pair(&resp, 1);
    magneto->WriteBack(buf, 100);
  } else if (Protocol::kPing == protocol_read.GetCategory()) {
    magneto->WriteBack(std::make_pair("p", 1), 100);
  } else {
    FATAL("unknown_category[" << protocol_read.GetCategory() << "]");
  }
}

int main(int argc, char** argv) {
  UNUSE(argc)
  UNUSE(argv)

  magneto = new Magneto::Magneto;
  bool end=false;
  bool ret = magneto->Init("conf/", RapidHandler, NULL, NULL, end);
  if (!ret) {
    FATAL("fail_init_magneto exit..");
    return 1;
  }

  sleep(10000);
  end=true;
  magneto->Stop();
  delete magneto;
  return 0;
}
