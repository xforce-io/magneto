#pragma once 

#include "common.h"

namespace magneto {

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

/*
 * Range
 */
template <typename T>
struct Range {
  static bool Check(int64_t int64) { 
    return int64 >= std::numeric_limits<T>::min() 
      && int64 <= std::numeric_limits<T>::max();
  }
};

template <>
struct Range<size_t> {
  static bool Check(int64_t int64) { return int64>=0; }
};

/*
 * IsArithmetic
 */
template <typename T>
struct IsArithmetic {
  enum {R = std::__is_arithmetic<T>::__value};
};

/* 
 * IsPOD
 * notice : this definition is not valid, should be update with is_pod when
 *    comiler supports c++0x
 */
template <typename T>
struct IsPOD {
  enum {R = IsArithmetic<T>::R};
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
