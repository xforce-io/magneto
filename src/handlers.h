#pragma once

namespace xforce { namespace magneto {

class ProtocolRead;

typedef void (*ReqHandler)(const ProtocolRead& protocol_read, void* args);
typedef void (*RoutineHandler)(void* args);

}}
