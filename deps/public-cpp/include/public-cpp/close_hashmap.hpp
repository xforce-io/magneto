#pragma once

#include "common.h"

namespace xforce {

template <
  typename Key, 
  typename Val, 
  typename KeyEqualF = std::equal_to<Key>,
  typename HashF = std::tr1::hash<Key> >
class CloseHashmap {
 private:
  typedef CloseHashmap<Key, Val, KeyEqualF, HashF> Self;
  typedef std::pair<int8_t, std::pair<Key, Val> > Item;
  
 public: 
  class Iterator {
    public:
     typedef CloseHashmap<Key, Val, KeyEqualF, HashF> Master;
     typedef Iterator Self;

    public:
     Iterator();
     Iterator(const Master& master, bool begin);
     Iterator(const Iterator& iter);

     inline void Next();
     bool IsEnd() const { return pos_ == master_->Capacity(); }
     inline std::pair<Key, Val>* operator->();
     Self& operator=(const Iterator& iter);
     inline bool operator==(const Iterator& iter) const;
     bool operator!=(const Iterator& iter) const { return !operator==(iter); }

    private:
     const Master* master_;
     size_t pos_;
  };

 private:
  static const size_t kNumPrimes=28; 
  static const size_t kPrimes[kNumPrimes];

 public:
  bool Init(
      size_t size_array, 
      KeyEqualF key_equal = KeyEqualF(), 
      HashF hash = HashF());

  inline bool Insert(const Key& key, const Val& val);
  inline bool Erase(const Key& key);

  inline const Val* Get(const Key& key) const;
  inline Val* Get(const Key& key);
  size_t Size() const { return num_elm_; }
  size_t Capacity() const { return size_array_; }

  Iterator Begin() { return Iterator(*this, true); }
  Iterator End() { return Iterator(*this, false); }

  virtual ~CloseHashmap();
 
 private:
  inline const Item* GetItem_(const Key& key) const;
  inline Item* GetItem_(const Key& key);

 private:
  size_t size_array_;
  KeyEqualF key_equal_;
  HashF hash_;

  Item* items_;
  size_t num_elm_;
};

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator::Iterator() :
  master_(NULL),
  pos_(0) {}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator::Iterator(
    const Master& master,
    bool begin) :
  master_(&master) {
  if (!begin) {
    pos_ = master_->Capacity();
  } else {
    for (pos_=0; pos_ < master_->Capacity(); ++pos_) {
      if (master_->items_[pos_].first>0) {
        return;
      }
    }
  }
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator::Iterator(
    const Iterator& iter) :
  master_(iter.master_),
  pos_(iter.pos_) {}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
void CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator::Next() {
  if (master_->Capacity() == pos_) {
    return;
  }

  for (++pos_; pos_ < master_->Capacity(); ++pos_) {
    if (master_->items_[pos_].first>0) {
      return;
    }
  }
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
std::pair<Key, Val>* CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator::operator->() {
  return pos_ != master_->Capacity() ? 
    &(master_->items_[pos_].second) :
    NULL;
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
typename CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator& 
CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator::operator=(const Iterator& iter) {
  master_ = iter.master_;
  pos_ = iter.pos_;
  return *this;
}
template <typename Key, typename Val, typename KeyEqualF, typename HashF>
bool CloseHashmap<Key, Val, KeyEqualF, HashF>::Iterator::operator==(const Iterator& iter) const {
  return master_ == iter.master_ && pos_ == iter.pos_;
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
const size_t CloseHashmap<Key, Val, KeyEqualF, HashF>::kPrimes[kNumPrimes] = {
    53,         97,         193,       389,       769,
    1543,       3079,       6151,      12289,     24593,
    49157,      98317,      196613,    393241,    786433,
    1572869,    3145739,    6291469,   12582917,  25165843,
    50331653,   100663319,  201326611, 402653189, 805306457,
    1610612741, 3221225473, 4294967291,
};

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
bool CloseHashmap<Key, Val, KeyEqualF, HashF>::Init(
    size_t size_array,
    KeyEqualF key_equal,
    HashF hash) {
  for (size_t i=0; i<kNumPrimes; ++i) {
    if ( 0 == size_array % kPrimes[i] ) {
      return false;
    }
  }

  size_array_=size_array;
  key_equal_=key_equal;
  hash_=hash;
  num_elm_=0;
  XFC_NEW(items_, Item [size_array_])
  for (size_t i=0; i<size_array_; ++i) {
    items_[i].first = 0;
  }
  return true;
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
bool CloseHashmap<Key, Val, KeyEqualF, HashF>::Insert(const Key& key, const Val& val) {
  if (unlikely(size_array_==num_elm_)) {
    XFC_BUG(true)
    return false;
  }

  Item* item = GetItem_(key);
  if (NULL!=item && item->first<=0) {
    *item = std::make_pair(1, std::make_pair(key, val));
    ++num_elm_;
    return true;
  } else {
    return false;
  }
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
bool CloseHashmap<Key, Val, KeyEqualF, HashF>::Erase(const Key& key) {
  Item* item = GetItem_(key);
  if (NULL!=item && item->first>0 && key_equal_(key, item->second.first)) {
    item->first = -1;
    --num_elm_;
    return true;
  } else {
    return false;
  }
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
const Val* CloseHashmap<Key, Val, KeyEqualF, HashF>::Get(const Key& key) const {
  const Item* item = GetItem_(key);
  return NULL!=item && item->first > 0 ? &(item->second.second) : NULL;
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
Val* CloseHashmap<Key, Val, KeyEqualF, HashF>::Get(const Key& key) {
  return CCAST<Val*>(CCAST<const Self*>(this)->Get(key));
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
const typename CloseHashmap<Key, Val, KeyEqualF, HashF>::Item* 
CloseHashmap<Key, Val, KeyEqualF, HashF>::GetItem_(const Key& key) const {
  size_t pos = hash_(key) % size_array_;
  if (!items_[pos].first || key_equal_(key, items_[pos].second.first)) {
    return &(items_[pos]);
  }

  const size_t step = kPrimes[pos%kNumPrimes]; 
  for (size_t i=0; i<size_array_-1; ++i) {
    pos = (pos+step) % size_array_;
    if (items_[pos].first==0 || (items_[pos].first>0 && key_equal_(key, items_[pos].second.first))) {
      return &(items_[pos]);
    }
  }
  XFC_BUG(true)
  return NULL;
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
typename CloseHashmap<Key, Val, KeyEqualF, HashF>::Item* 
CloseHashmap<Key, Val, KeyEqualF, HashF>::GetItem_(const Key& key) {
  return CCAST<Item*>(CCAST<const Self*>(this)->GetItem_(key));
}

template <typename Key, typename Val, typename KeyEqualF, typename HashF>
CloseHashmap<Key, Val, KeyEqualF, HashF>::~CloseHashmap() {
  XFC_DELETE_ARRAY(items_)
}

}
