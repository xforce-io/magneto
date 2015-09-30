#pragma once

#include "../common.h"
#include "../thread_privacy/thread_privacy.h"
#include "../pool_objs/pool_objs.hpp"

namespace xforce {

struct JsonValType {
 public: 
  enum Type {
    kNull,
    kBool,
    kInt,
    kDouble,
    kStr,
    kList,
    kDict,
    kInvalid,
  };
};

class JsonType;

struct SharedJsonVal {
 public: 
  typedef SharedJsonVal Self;
  typedef std::string StrType;
  typedef std::vector<JsonType> ListType;
  typedef std::tr1::unordered_map<std::string, JsonType> DictType;
 
  typedef PoolObjs<StrType> PoolStr;
  typedef PoolObjs<ListType> PoolList;
  typedef PoolObjs<DictType> PoolDict;

 private:
  static const size_t kTPDIndexPoolStr=1; 
  static const size_t kTPDIndexPoolList=2; 
  static const size_t kTPDIndexPoolDict=3; 
 
 public:
  inline explicit SharedJsonVal(JsonValType::Type type = JsonValType::kNull);
  void Reset(JsonValType::Type);

  void IncRefCnt();
  void DecRefCnt(bool use_pool=true);
  Self* Copy();

  inline static StrType* GetStr();
  inline static void FreeStr(StrType* str);
  inline static ListType* GetList();
  inline static void FreeList(ListType* list);
  inline static DictType* GetDict();
  inline static void FreeDict(DictType* dict);

  static JsonType* ParseJson(const char* str);
  void DumpJson(std::ostream& ostr) const;

  class JsonParseUtil {
   public:
    static JsonType* ParseJson(const char*& str);

   private:
    static JsonType* ReadinNull_(const char*& str);
    static JsonType* ReadinTrue_(const char*& str);
    static JsonType* ReadinFalse_(const char*& str);
    static JsonType* ReadinStr_(const char*& str);
    static JsonType* ReadinList_(const char*& str);
    static JsonType* ReadinDict_(const char*& str);
    static JsonType* ReadInDigits_(const char*& str);

    static std::string* ReadinStrRaw_(const char*& str);
    static std::pair<std::string, JsonType>* ReadDictPair_(const char*& str);
    static void IgnoreBlanks_(const char*& str);
  };

 public:
  JsonValType::Type type;
  uint16_t ref_cnt;
  union {
    bool bool_val;
    int64_t int_val;
    double double_val;
    StrType* str_val;
    ListType* list_val;
    DictType* dict_val;
  } data;

 private:
  static ThreadPrivacy thread_privacy_;
};

class JsonType {
 public:
  typedef JsonType Self;
  typedef SharedJsonVal::StrType StrType;
  typedef SharedJsonVal::ListType ListType;
  typedef SharedJsonVal::DictType DictType;
 
 public:
  inline explicit JsonType(JsonValType::Type type = JsonValType::kNull);
  inline JsonType(const JsonType& json_val);

  void Reset(JsonValType::Type type);

  Self& operator=(bool bool_val);
  Self& operator=(int int_val);
  Self& operator=(int64_t int_val);
  Self& operator=(double double_val);
  Self& operator=(const char* str_val);
  Self& operator=(const std::string& str_val) { return operator=(str_val.c_str()); }
  Self& operator=(const JsonType& json_val);

  inline bool operator==(bool bool_val);
  inline bool operator==(int int_val);
  inline bool operator==(int64_t int_val);
  inline bool operator==(double double_val);
  inline bool operator==(const char* str_val);
  inline bool operator==(const std::string& str_val);

  JsonValType::Type Type() const { return shared_json_val_->type; }
  inline size_t Size() const;

  bool IsNull() const { return JsonValType::kNull == shared_json_val_->type; }
  bool IsBool() const { return JsonValType::kBool == shared_json_val_->type; }
  bool IsInt() const { return JsonValType::kInt == shared_json_val_->type; }
  bool IsDouble() const { return JsonValType::kDouble == shared_json_val_->type; }
  bool IsStr() const { return JsonValType::kStr == shared_json_val_->type; }
  bool IsList() const { return JsonValType::kList == shared_json_val_->type; }
  bool IsDict() const { return JsonValType::kDict == shared_json_val_->type; }

  bool AsBool() const { return shared_json_val_->data.bool_val; }
  int64_t AsInt() const { return shared_json_val_->data.int_val; }
  double AsDouble() const { return shared_json_val_->data.double_val; }
  const StrType& AsStr() const { return *(shared_json_val_->data.str_val); }
  const ListType& AsList() const { return *(shared_json_val_->data.list_val); }
  const DictType& AsDict() const { return *(shared_json_val_->data.dict_val); }

  inline const Self& operator[](size_t index) const;
  inline Self& operator[](size_t index);
  inline const Self& operator[](const std::string& key) const;
  inline Self& operator[](const std::string& key);

  static const JsonType* ParseJson(const char* str);
  void DumpJson(std::ostream& ostr) const;

  static const JsonType* CreateConf(const std::string& filepath);

  virtual ~JsonType();
 
 protected:
  void IncRefCnt() { shared_json_val_->IncRefCnt(); }
  void DecRefCnt() { shared_json_val_->DecRefCnt(); }

 private:
  inline const Self& Null_() const;
  inline Self& Null_();
  void Reset_(JsonValType::Type type = JsonValType::kInvalid, bool at_exit=false); 
  inline Self& Normalize_();

 private:
  SharedJsonVal* shared_json_val_;

  friend class SharedJsonVal;
};

SharedJsonVal::SharedJsonVal(JsonValType::Type type) {
  Reset(type);
}

JsonType::StrType* SharedJsonVal::GetStr() {
  JsonType::StrType* str = thread_privacy_.Get<PoolStr>(kTPDIndexPoolStr)->Get();
  str->clear();
  return str;
}

void SharedJsonVal::FreeStr(StrType* str) {
  thread_privacy_.Get<PoolStr>(kTPDIndexPoolStr)->Free(str);
}

JsonType::ListType* SharedJsonVal::GetList() {
  JsonType::ListType* list = thread_privacy_.Get<PoolList>(kTPDIndexPoolList)->Get();
  list->clear();
  return list;
}

void SharedJsonVal::FreeList(ListType* list) {
  thread_privacy_.Get<PoolList>(kTPDIndexPoolList)->Free(list);
}

JsonType::DictType* SharedJsonVal::GetDict() {
  JsonType::DictType* dict = thread_privacy_.Get<PoolDict>(kTPDIndexPoolDict)->Get();
  dict->clear();
  return dict;
}

void SharedJsonVal::FreeDict(DictType* dict) {
  thread_privacy_.Get<PoolDict>(kTPDIndexPoolDict)->Free(dict);
}

JsonType::JsonType(JsonValType::Type type) {
  XFC_NEW(shared_json_val_, SharedJsonVal(type))
}

JsonType::JsonType(const JsonType& json_val) {
  shared_json_val_=NULL;
  operator=(json_val);
}

bool JsonType::operator==(bool bool_val) {
  return JsonValType::kBool == shared_json_val_->type 
    && bool_val == shared_json_val_->data.bool_val;
}

bool JsonType::operator==(int int_val) {
  return operator==(int64_t(int_val));
}

bool JsonType::operator==(int64_t int_val) {
  return JsonValType::kInt == shared_json_val_->type 
    && int_val == shared_json_val_->data.int_val;
}

bool JsonType::operator==(double double_val) {
  return JsonValType::kDouble == shared_json_val_->type 
    && ( shared_json_val_->data.double_val - double_val < std::numeric_limits<double>::epsilon() )
    && ( double_val - shared_json_val_->data.double_val < std::numeric_limits<double>::epsilon() );
}

bool JsonType::operator==(const char* str_val) {
  return JsonValType::kStr == shared_json_val_->type
    && 0 == strcmp(str_val, shared_json_val_->data.str_val->c_str());
}

bool JsonType::operator==(const std::string& str_val) {
  return operator==(str_val.c_str());
}

size_t JsonType::Size() const {
  switch (shared_json_val_->type) {
    case JsonValType::kNull : return 0;
    case JsonValType::kBool : return 1;
    case JsonValType::kInt  : return 1;
    case JsonValType::kDouble : return 1;
    case JsonValType::kStr : return 1;
    case JsonValType::kList : return shared_json_val_->data.list_val->size();
    default : return shared_json_val_->data.dict_val->size();
  }
}

const JsonType& JsonType::operator[](size_t index) const {
  if (unlikely( JsonValType::kList != shared_json_val_->type
      || shared_json_val_->data.list_val->size() <= index )) {
    return Null_();
  }
  return (*(shared_json_val_->data.list_val))[index];
}

JsonType& JsonType::operator[](size_t index) {
  if (JsonValType::kList != shared_json_val_->type) {
    Reset_(JsonValType::kList);
  }
  Normalize_();

  ListType* list = shared_json_val_->data.list_val;
  if (list->size() <= index ) {
    list->resize(index+1);
  }
  return (*list)[index]; 
}

const JsonType& JsonType::operator[](const std::string& key) const {
  if (unlikely( JsonValType::kDict != shared_json_val_->type )) {
    return Null_();
  }

  DictType::const_iterator iter = shared_json_val_->data.dict_val->find(key);
  return shared_json_val_->data.dict_val->end() != iter ? iter->second : Null_();
}

JsonType& JsonType::operator[](const std::string& key) {
  if (unlikely( JsonValType::kDict != shared_json_val_->type )) {
    Reset_(JsonValType::kDict);
  }
  Normalize_();

  DictType::iterator iter = shared_json_val_->data.dict_val->find(key);
  if (shared_json_val_->data.dict_val->end() == iter) { 
    iter = (shared_json_val_->data.dict_val->insert(
        std::pair<std::string, JsonType>(key, JsonType(JsonValType::kDict)))).first;
  }
  return iter->second;
}

const JsonType& JsonType::Null_() const {
  static Self null_val(JsonValType::kNull);
  return null_val;
}

JsonType& JsonType::Null_() {
  static Self null_val(JsonValType::kNull);
  return null_val;
}

JsonType& JsonType::Normalize_() {
  if (1 != shared_json_val_->ref_cnt) {
    shared_json_val_->DecRefCnt();
    shared_json_val_ = shared_json_val_->Copy();
  }
  return *this;
}

}
