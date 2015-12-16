#pragma once 

#include "common.h"

namespace xforce {

/*
 * If
 */
template <bool cond, typename A, typename B>
struct If{
  typedef A Type;
};

template <typename A, typename B>
struct If<false, A, B> {
  typedef B Type;
};

/*
 * SameType
 */
template <typename A, typename B>
struct SameType {
  enum {R = std::__are_same<A, B>::__value};
};

template<typename T>
struct ReflectorF {
  const T& operator() (const T& t) const { return t; }
};

template < typename T, typename CompF=std::equal_to<T> >
class PtrEqF {
 public:
  explicit PtrEqF(const CompF comp) : comp_(comp) {}

  bool operator()(const T* x, const T* y) const { return comp_(*x, *y); }
  
 private:
  CompF comp_;
};

template < typename T, typename HashF=std::tr1::hash<T> >
class PtrHashF {
 public:
  explicit PtrHashF(const HashF hash) : hash_(hash) {}

  size_t operator() (T* val) const { return hash_(*val); }
  
 private:
  HashF hash_;  
};

}
