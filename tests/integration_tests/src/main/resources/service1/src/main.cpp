#include "projects/magneto/magneto/magneto.h"

using namespace magneto;

Magneto::Magneto* magneto;

void RapidHandler(const ProtocolRead& protocol_read, void* args) {
  UNUSE(args)

  const ProtocolReadRapid& protocol_read_rapid = SCAST<const ProtocolReadRapid&>(protocol_read);
  if ( 1 != protocol_read_rapid.Header()->size ) {
    WARN("ret[" << protocol_read_rapid.Len() << "]");
  }

  char resp;
  if ("plus" == protocol_read.GetReqInfo().listen_addr->name) {
    resp = *(protocol_read_rapid.Data()) + 1;
  } else {
    resp = *(protocol_read_rapid.Data()) - 1;
  }

  std::pair<const char*, size_t> buf = std::make_pair(&resp, 1);
  magneto->WriteBack(buf, 100);
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
