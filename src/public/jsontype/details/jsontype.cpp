#include "../jsontype.h"

namespace magneto {

ThreadPrivacy SharedJsonVal::thread_privacy_;

void SharedJsonVal::Reset(JsonValType::Type type_arg) {
  type=type_arg;
  ref_cnt=1;
  switch (type) {
    case JsonValType::kStr :
      data.str_val = GetStr();
      break;
    case JsonValType::kList : 
      data.list_val = GetList();
      break;
    case JsonValType::kDict :
      data.dict_val = GetDict();
      break;
    default :
      break;
  }
}

void SharedJsonVal::IncRefCnt() {
  ++ref_cnt;
}

void SharedJsonVal::DecRefCnt(bool use_pool) {
  --ref_cnt;
  if (0==ref_cnt) {
    if (use_pool) {
      switch (type) {
        case JsonValType::kStr : FreeStr(data.str_val); break;
        case JsonValType::kList : FreeList(data.list_val); break;
        case JsonValType::kDict : FreeDict(data.dict_val); break;
        default : break;
      }
    } else {
      switch (type) {
        case JsonValType::kStr : MAG_DELETE(data.str_val); break;
        case JsonValType::kList : MAG_DELETE(data.list_val); break;
        case JsonValType::kDict : MAG_DELETE(data.dict_val); break;
        default : break;
      }
    }
  }
}

SharedJsonVal* SharedJsonVal::Copy() {
  MAG_NEW_DECL(shared_json_val, SharedJsonVal, SharedJsonVal(type))
  switch (type) {
    case JsonValType::kNull :
      break;
    case JsonValType::kBool :
      shared_json_val->data.bool_val = data.bool_val;
      break;
    case JsonValType::kInt :
      shared_json_val->data.int_val = data.int_val;
      break;
    case JsonValType::kDouble :
      shared_json_val->data.double_val = data.double_val;
      break;
    case JsonValType::kStr :
      shared_json_val->data.str_val->assign(*(data.str_val));
      break;
    case JsonValType::kList :
      *(shared_json_val->data.list_val) = *(data.list_val);
      break;
    default :
      *(shared_json_val->data.dict_val) = *(data.dict_val);
      break;
  }
  return shared_json_val;
}

JsonType* SharedJsonVal::ParseJson(const char* str) {
  return JsonParseUtil::ParseJson(str);
}

void SharedJsonVal::DumpJson(std::ostream& ostr) const {
  switch (type) {
    case JsonValType::kNull :
      ostr << "null";
      break;
    case JsonValType::kBool :
      ostr << data.bool_val;
      break;
    case JsonValType::kInt :
      ostr << data.int_val;
      break;
    case JsonValType::kDouble :
      ostr << data.double_val;
      break;
    case JsonValType::kStr :
      ostr << "\"" << *(data.str_val) << "\"";
      break;
    case JsonValType::kList :
      ostr << "[";
      if (data.list_val->size()) {
        ListType::iterator iter = data.list_val->begin();
        (*iter).DumpJson(ostr);
        for (++iter; iter != data.list_val->end(); ++iter) {
          ostr << ",";
          (*iter).DumpJson(ostr);
        }
      }
      ostr << "]";
      break;
    default :
      ostr << "{";
      if (data.dict_val->size()) {
        DictType::iterator iter = data.dict_val->begin();
        ostr << "\"" << iter->first << "\":";
        iter->second.DumpJson(ostr);
        for (++iter; iter != data.dict_val->end(); ++iter) {
          ostr << ", \"" << iter->first << "\":";
          iter->second.DumpJson(ostr);
        }
      }
      ostr << "}";
      break;
  }
}

JsonType* SharedJsonVal::JsonParseUtil::ParseJson(const char*& str) {
  IgnoreBlanks_(str);
  switch (*str) {
    case '\0' : return NULL;
    case 'n' : return ReadinNull_(str);
    case 't' : return ReadinTrue_(str);
    case 'f' : return ReadinFalse_(str);
    case '"' : return ReadinStr_(str);
    case '[' : return ReadinList_(str); 
    case '{' : return ReadinDict_(str);
    default : 
      if (*str>='0' && *str<='9') {
        return ReadInDigits_(str);
      } else {
        return NULL;
      }
  }
  return NULL;
}

JsonType* SharedJsonVal::JsonParseUtil::ReadinNull_(const char*& str) {
  size_t len_null = strlen("null");
  if (strlen(str) < len_null || 0 != memcmp(str, "null", len_null)) {
    return NULL;
  }
  str+=len_null;
  MAG_NEW_DECL(null_val, JsonType, JsonType(JsonValType::kNull))
  return null_val;
}

JsonType* SharedJsonVal::JsonParseUtil::ReadinTrue_(const char*& str) {
  size_t len_true = strlen("true");
  if (strlen(str) < len_true || 0 != memcmp(str, "true", len_true)) {
    return NULL;
  }
  str+=len_true;
  MAG_NEW_DECL(bool_val, JsonType, JsonType(JsonValType::kBool))
  bool_val->shared_json_val_->data.bool_val = true;
  return bool_val;
}

JsonType* SharedJsonVal::JsonParseUtil::ReadinFalse_(const char*& str) {
  size_t len_false = strlen("false");
  if (strlen(str) < len_false || 0 != memcmp(str, "false", len_false)) {
    return NULL;
  }
  str+=len_false;
  MAG_NEW_DECL(bool_val, JsonType, JsonType(JsonValType::kBool))
  bool_val->shared_json_val_->data.bool_val = false;
  return bool_val;
}

JsonType* SharedJsonVal::JsonParseUtil::ReadinStr_(const char*& str) {
  MAG_NEW_DECL(str_val, JsonType, JsonType(JsonValType::kStr))
  std::string* tmp_str = ReadinStrRaw_(str);
  if (NULL==tmp_str) return NULL;

  str_val->shared_json_val_->data.str_val->assign(*tmp_str);
  MAG_DELETE(tmp_str)
  return str_val;
}

JsonType* SharedJsonVal::JsonParseUtil::ReadinList_(const char*& str) {
  ++str;
  IgnoreBlanks_(str);
  if ('\0'==*str) return NULL;

  MAG_NEW_DECL(list_val, JsonType, JsonType(JsonValType::kList))
  while (']'!=*str && '\0'!=*str) {
    IgnoreBlanks_(str);
    JsonType* json_val = ParseJson(str);
    if (NULL==json_val) {
      MAG_DELETE(list_val)
      return NULL;
    }

    list_val->shared_json_val_->data.list_val->push_back(*json_val);
    MAG_DELETE(json_val)
    IgnoreBlanks_(str);
    if (','==*str) {
      ++str;
      continue;
    }
  }
  return '\0'!=*str++ ? list_val : NULL;
}

JsonType* SharedJsonVal::JsonParseUtil::ReadinDict_(const char*& str) {
  ++str;
  IgnoreBlanks_(str);
  if ('\0'==*str) return NULL;

  MAG_NEW_DECL(dict_val, JsonType, JsonType(JsonValType::kDict))
  while ('}'!=*str && '\0'!=*str) {
    IgnoreBlanks_(str);
    std::pair<std::string, JsonType>* dict_pair = ReadDictPair_(str);
    if (NULL==dict_pair) {
      MAG_DELETE(dict_val)
      return NULL;
    }

    dict_val->shared_json_val_->data.dict_val->insert(*dict_pair);
    MAG_DELETE(dict_pair)
    IgnoreBlanks_(str);
    if (','==*str) {
      ++str;
      continue;
    }
  }
  return '\0'!=*str++ ? dict_val : NULL;
}

JsonType* SharedJsonVal::JsonParseUtil::ReadInDigits_(const char*& str) {
  char* endptr;
  long l = strtol(str, &endptr, 10);
  if ('.' == *endptr) {
    double d = strtod(str, &endptr);
    MAG_NEW_DECL(int_val, JsonType, JsonType(JsonValType::kDouble))
    int_val->shared_json_val_->data.double_val = d;
    str=endptr;
    return int_val;
  } else {
    MAG_NEW_DECL(int_val, JsonType, JsonType(JsonValType::kInt))
    int_val->shared_json_val_->data.int_val = l;
    str=endptr;
    return int_val;
  }
}

std::string* SharedJsonVal::JsonParseUtil::ReadinStrRaw_(const char*& str) {
  if ('"' != *str) return NULL;

  const char* start=++str;
  while ('\0'!=*str) {
    if ( '"'==*str && ( start==str || '\\'!=*str ) ) {
      break;
    }
    ++str;
  }

  if ('\0'==*str) {
    return NULL;
  } else {
    MAG_NEW_DECL(str_val, std::string, std::string)
    str_val->assign(start, str-start);
    ++str;
    return str_val;
  }
}

std::pair<std::string, JsonType>* 
SharedJsonVal::JsonParseUtil::ReadDictPair_(const char*& str) {
  std::pair<std::string, JsonType>* res_pair;
  JsonType* json_val = NULL;
  std::string* str_val = ReadinStrRaw_(str);

  MAG_FAIL_HANDLE(NULL==str_val || 0 == str_val->length())

  IgnoreBlanks_(str);
  MAG_FAIL_HANDLE(':'!=*str++)
  IgnoreBlanks_(str);

  json_val = ParseJson(str);
  MAG_FAIL_HANDLE(NULL==json_val)

  res_pair = new std::pair<std::string, JsonType>(*str_val, *json_val);
  MAG_DELETE(str_val)
  MAG_DELETE(json_val)
  return res_pair;

  ERROR_HANDLE:
  MAG_DELETE(str_val)
  MAG_DELETE(json_val)
  return NULL;
}

void SharedJsonVal::JsonParseUtil::IgnoreBlanks_(const char*& str) {
  while ( ( '\n'==*str || '\t'==*str || ' '==*str ) && '\0'!=*str ) {
    ++str;
  }
}

#define ASSIGN_NAIVE_TYPE(data_type, val_type, data_member) \
  JsonType& JsonType::operator=(data_type data_val) { \
    if (NULL!=shared_json_val_) { \
      if (shared_json_val_->ref_cnt > 1 || val_type != shared_json_val_->type) { \
        shared_json_val_->DecRefCnt(); \
      } \
      \
      if (0 == shared_json_val_->ref_cnt) { \
        shared_json_val_->ref_cnt = 1; \
      } else { \
        shared_json_val_=NULL; \
      } \
    } \
    \
    if (NULL==shared_json_val_) { \
      MAG_NEW(shared_json_val_, SharedJsonVal(val_type)) \
    } else { \
      shared_json_val_->type = val_type; \
    } \
    shared_json_val_->data.data_member = data_val; \
    return *this; \
  }

ASSIGN_NAIVE_TYPE(bool,    JsonValType::kBool,   bool_val)
ASSIGN_NAIVE_TYPE(int64_t, JsonValType::kInt,    int_val)
ASSIGN_NAIVE_TYPE(int,     JsonValType::kInt,    int_val)
ASSIGN_NAIVE_TYPE(double,  JsonValType::kDouble, double_val)

#undef ASSIGN_NAIVE_TYPE

JsonType& JsonType::operator=(const char* str_val) {
  if (NULL!=shared_json_val_) {
    if (shared_json_val_->ref_cnt > 1 || JsonValType::kStr != shared_json_val_->type) {
      shared_json_val_->DecRefCnt();
    }

    if (0 == shared_json_val_->ref_cnt) {
      shared_json_val_->ref_cnt = 1;
      shared_json_val_->data.str_val = SharedJsonVal::GetStr();
    } else {
      shared_json_val_=NULL;
    }
  }

  if (NULL==shared_json_val_) {
    MAG_NEW(shared_json_val_, SharedJsonVal(JsonValType::kStr))
  } else {
    shared_json_val_->type = JsonValType::kStr;
  }
  shared_json_val_->data.str_val->assign(str_val);
  return *this;
}

JsonType& JsonType::operator=(const JsonType& other) {
  if (this == &other) return *this;

  Reset_();
  CCAST<JsonType&>(other).IncRefCnt();
  shared_json_val_ = other.shared_json_val_;
  return *this;
}

void JsonType::Reset_(JsonValType::Type type, bool at_exit) {
  if (NULL!=shared_json_val_) {
    shared_json_val_->DecRefCnt(at_exit ? false : true);
    if (0 != shared_json_val_->ref_cnt) {
      shared_json_val_=NULL;
    } else {
      if (JsonValType::kInvalid == type) {
        MAG_DELETE(shared_json_val_)
      } else {
        shared_json_val_->Reset(type);
      }
      return;
    }
  }

  if (JsonValType::kInvalid != type) {
    MAG_NEW(shared_json_val_, SharedJsonVal(type))
  }
}

JsonType* JsonType::ParseJson(const char* str) {
  return SharedJsonVal::ParseJson(str);
}

void JsonType::DumpJson(std::ostream& ostr) const {
  shared_json_val_->DumpJson(ostr);
}

JsonType* JsonType::CreateConf(const std::string& filepath) {
  FILE* fp = fopen(filepath.c_str(), "r");
  if (NULL==fp) return NULL;

  char block[4096];
  std::string conf_str;
  while (true) {
    size_t ret = fread(block, 1, sizeof(block), fp);
    if (ret>0) {
      conf_str.append(block, ret);
    } else {
      break;
    }
  }
  fclose(fp);
  return ParseJson(conf_str.c_str()); 
}

JsonType::~JsonType() {
  Reset_(JsonValType::kInvalid, true);
}

}
