#pragma once

#include "public/slice.hpp"
#include "handlers.h"

namespace magneto {

typedef std::pair<Slice, const void*> Buf;
typedef std::vector<const Buf*> Bufs;
typedef std::pair<RoutineHandler, size_t> RoutineItem;
typedef std::vector< std::pair<RoutineHandler, size_t> > RoutineItems;
typedef std::pair<int, ProtocolRead*> Response;
typedef std::vector<Response> Responses;
typedef std::vector<int> Errors;

}
