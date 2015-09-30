#pragma once

#include "../common.h"
#include "default_notifier.h"

namespace xforce {

template <typename MsgHeader>
struct MPSCFixedPipeMsg {
 public:
  static const uint32_t kMaskNotWriting = ((uint32_t)1 << 31);
 
 public: 
  MsgHeader msg_header;
  uint32_t size;
  char content[];
};

template <typename MsgHeader, typename Notifier=DefaultNotifier>
class MPSCFixedPipe {
 public:
  typedef MPSCFixedPipeMsg<MsgHeader> Msg;

 private:
  static const uint32_t kMsgOffset = offsetof(MPSCFixedPipeMsg<MsgHeader>, content);
  static const size_t kNumBitsVersion = 16;
  static const size_t kNumVersions = (1<<kNumBitsVersion);
  static const size_t kMaskPtr = (( (size_t)1 << (64-kNumBitsVersion) ) - 1);
  static const size_t kMaskVersion = ~kMaskPtr;

  static const size_t kTimesBusyQuery=2;
  static const time_t kTimesSleepUs[];
  static const size_t kDefaultThresholdSenderNotify=0;

 public:
  MPSCFixedPipe() {}

  bool Init(
      size_t size_pipe, 
      size_t max_size_content,
      size_t threshold_sender_notify=kDefaultThresholdSenderNotify,
      Notifier* notifier=NULL);

  /* 
   * @sender interfaces
   */
  bool SendMsg(
      const MsgHeader& msg_header, 
      const char* content=NULL,
      uint32_t size_content=0);

  /* 
   * @receiver interfaces 
   */
  inline Msg* ReceiveMsg(time_t timeo_us=0);
  inline void MsgConsumed(); 

  inline size_t SizeMsgReady() const;
  inline size_t SizeSpace() const;

  inline static time_t GetCurrentTimeUs();

  virtual ~MPSCFixedPipe();

 private:
  inline Msg* ReceiveMsgWithNotify_(time_t timeo_ms);
  inline Msg* ReceiveMsgWithoutNotify_(time_t timeo_ms);

  inline bool SendMsg_(const MsgHeader& msg_header, const char* content, uint32_t size_content);
  inline Msg* ReceiveMsg_();

  inline static time_t GetTimeToSleepUs_(size_t slot);

  Msg* ReserveSpaceForSend_(size_t size_content, char*& pos_before_rewind);
  void JustSendMsg_(Msg* msg, char* pos_before_rewind);
  inline bool HasTodoMPSCFixedPipeMsg_();

  static void SetReady_(Msg* msg) { msg->size |= Msg::kMaskNotWriting; }
  inline static bool IsReady_(const Msg* msg);
  inline static void SetBlank_(char* from, char* to);
  static bool IsBlank_(const Msg* msg) { return IsBlank_(msg->size); }
  static bool IsBlank_(size_t size) { return uint32_t(-1) == size; }
 
  //not writing means ready or blank
  static bool IsNotWriting_(const Msg* msg) { return IsNotWriting_(msg->size); }
  static bool IsNotWriting_(size_t size) { return size & Msg::kMaskNotWriting; }

  inline char* DecodeSenderPaceFromPtr_() const;
  inline char* DecodeSenderPaceToPtr_() const;
  inline char* DecodeSenderPacePtr_(size_t pace) const;
  inline size_t DecodeSenderPaceFromVersion_() const;
  inline size_t DecodeSenderPaceToVersion_() const;
  inline static size_t DecodeSenderPaceVersion_(size_t pace);

  inline void EncodeSenderPaceFrom_(char* ptr, size_t version);
  inline void EncodeSenderPaceTo_(char* ptr, size_t version);
  inline static size_t EncodeSenderPace_(char* ptr, size_t version);

  inline bool CASSenderPaceFrom_(size_t old_pace, char* new_pace, bool new_version);
  inline bool CASSenderPaceTo_(size_t old_pace, char* new_pace, bool new_version);

  inline static uint32_t GetMsgSize_(Msg* msg);

  inline static void SleepUs(time_t usecs);

 private:
  //const
  size_t size_pipe_;
  size_t max_size_content_;
  size_t threshold_sender_notify_;
  ///

  char* pipe_;
  char* receiver_pace_;
  volatile size_t sender_pace_from_;
  volatile size_t sender_pace_to_;
  char* end_of_pipe_;

  Notifier* notifier_;
};

template <typename MsgHeader, typename Notifier>
const time_t MPSCFixedPipe<MsgHeader, Notifier>::kTimesSleepUs[] = {1,10,100,1000};

template <typename MsgHeader, typename Notifier>
bool MPSCFixedPipe<MsgHeader, Notifier>::Init(
    size_t size_pipe, 
    size_t max_size_content,
    size_t threshold_sender_notify,
    Notifier* notifier) {
  if ( size_pipe <= max_size_content+kMsgOffset
      || ( 0!=threshold_sender_notify && NULL==notifier ) ) {
    return false;
  }

  size_pipe_=size_pipe;
  max_size_content_=max_size_content;
  threshold_sender_notify_=threshold_sender_notify;

  pipe_ = new (std::nothrow) char [size_pipe_];
  if (NULL==pipe_) return false;

  receiver_pace_=pipe_;
  EncodeSenderPaceFrom_(pipe_, 0);
  EncodeSenderPaceTo_(pipe_, 0);
  end_of_pipe_ = size_pipe_+pipe_;
  SetBlank_(pipe_, end_of_pipe_);

  notifier_=notifier;
  return true;
}

template <typename MsgHeader, typename Notifier>
bool MPSCFixedPipe<MsgHeader, Notifier>::SendMsg(
    const MsgHeader& msg_header, 
    const char* content, 
    uint32_t size_content) {
  bool notify = ( 0!=threshold_sender_notify_ 
      && (size_pipe_-SizeSpace() <= threshold_sender_notify_) );

  bool ret;
  for (size_t i=0; i<kTimesBusyQuery; ++i) {
    ret = SendMsg_(msg_header, content, size_content);
    if (likely(ret)) {
      if (unlikely(notify)) notifier_->Notify();
      return true;
    }
  }
  return false;
}

template <typename MsgHeader, typename Notifier>
typename MPSCFixedPipe<MsgHeader, Notifier>::Msg* 
MPSCFixedPipe<MsgHeader, Notifier>::ReceiveMsg(time_t timeo_ms) {
  return 0!=threshold_sender_notify_ 
    ? ReceiveMsgWithNotify_(timeo_ms) 
    : ReceiveMsgWithoutNotify_(timeo_ms);
}

template <typename MsgHeader, typename Notifier>
void MPSCFixedPipe<MsgHeader, Notifier>::MsgConsumed() { 
  char* orig_receiver_pace=receiver_pace_;
  receiver_pace_ += GetMsgSize_((RCAST<Msg*>(orig_receiver_pace)));
  SetBlank_(orig_receiver_pace, receiver_pace_);
}

template <typename MsgHeader, typename Notifier>
size_t MPSCFixedPipe<MsgHeader, Notifier>::SizeMsgReady() const {
  char* tmp_receiver_pace=receiver_pace_;
  char* tmp_sender_pace_from = DecodeSenderPaceFromPtr_();
  return tmp_receiver_pace<=tmp_sender_pace_from ?
    tmp_sender_pace_from-tmp_receiver_pace :
    tmp_sender_pace_from+size_pipe_-tmp_receiver_pace;
}

template <typename MsgHeader, typename Notifier>
size_t MPSCFixedPipe<MsgHeader, Notifier>::SizeSpace() const {
  char* tmp_receiver_pace=receiver_pace_;
  char* tmp_sender_pace_to = DecodeSenderPaceToPtr_();
  return tmp_sender_pace_to<tmp_receiver_pace ?
    tmp_receiver_pace-tmp_sender_pace_to :
    tmp_receiver_pace+size_pipe_-tmp_sender_pace_to;
}

template <typename MsgHeader, typename Notifier>
time_t MPSCFixedPipe<MsgHeader, Notifier>::GetCurrentTimeUs() {
  timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000000 + t.tv_usec;
}

template <typename MsgHeader, typename Notifier>
MPSCFixedPipe<MsgHeader, Notifier>::~MPSCFixedPipe() {
  XFC_DELETE_ARRAY(pipe_)
}

template <typename MsgHeader, typename Notifier>
typename MPSCFixedPipe<MsgHeader, Notifier>::Msg* 
MPSCFixedPipe<MsgHeader, Notifier>::ReceiveMsgWithNotify_(time_t timeo_ms) {
  Msg* msg;
  for (size_t i=0; i<kTimesBusyQuery; ++i) {
    msg = ReceiveMsg_();
    if (NULL!=msg) return msg;
  }

  if (0==timeo_ms) return NULL;

  notifier_->Wait(timeo_ms);
  return ReceiveMsg_();
}

template <typename MsgHeader, typename Notifier>
typename MPSCFixedPipe<MsgHeader, Notifier>::Msg* 
MPSCFixedPipe<MsgHeader, Notifier>::ReceiveMsgWithoutNotify_(time_t timeo_ms) {
  Msg* msg;
  for (size_t i=0; i<kTimesBusyQuery; ++i) {
    msg = ReceiveMsg_();
    if (NULL!=msg) return msg;
  }

  if (0==timeo_ms) return NULL;

  time_t start_time = GetCurrentTimeUs();
  size_t round=0;
  while (true) {
    time_t time_to_sleep_us = GetTimeToSleepUs_(round);
    SleepUs(time_to_sleep_us);

    msg = ReceiveMsg_();
    if (NULL!=msg) return msg;

    time_t current_time = GetCurrentTimeUs();
    if (start_time + timeo_ms*1000 < current_time) break;

    ++round;
  }
  return NULL;
}

template <typename MsgHeader, typename Notifier>
bool MPSCFixedPipe<MsgHeader, Notifier>::SendMsg_(
    const MsgHeader& msg_header,
    const char *content, 
    uint32_t size_content) {
  char* pos_before_rewind=NULL;
  Msg* msg = ReserveSpaceForSend_(size_content, pos_before_rewind);
  if (unlikely(NULL==msg)) return false;

  msg->msg_header = msg_header;
  msg->size = size_content;
  if (likely(0!=size_content)) {
    memcpy(msg->content, content, size_content);
  }
  JustSendMsg_(msg, pos_before_rewind);
  return true;
}

template <typename MsgHeader, typename Notifier>
MPSCFixedPipeMsg<MsgHeader>* MPSCFixedPipe<MsgHeader, Notifier>::ReceiveMsg_() { 
  return HasTodoMPSCFixedPipeMsg_() ? RCAST<Msg*>(receiver_pace_) : NULL; 
}

template <typename MsgHeader, typename Notifier>
time_t MPSCFixedPipe<MsgHeader, Notifier>::GetTimeToSleepUs_(size_t slot) {
  const size_t kSizeTimesSleepUs = sizeof(kTimesSleepUs)/sizeof(kTimesSleepUs[0]);
  return slot<kSizeTimesSleepUs ? kTimesSleepUs[slot] : kTimesSleepUs[kSizeTimesSleepUs - 1];
}

template <typename MsgHeader, typename Notifier>
typename MPSCFixedPipe<MsgHeader, Notifier>::Msg* 
MPSCFixedPipe<MsgHeader, Notifier>::ReserveSpaceForSend_(size_t size_content, char*& pos_before_rewind) {
  if (unlikely(size_content>max_size_content_)) return NULL;

  size_t tmp_sender_pace_to=sender_pace_to_;
  char* tmp_sender_pace_to_ptr = DecodeSenderPacePtr_(tmp_sender_pace_to);
  char* tmp_receiver_pace=receiver_pace_;
  size_t size_need = kMsgOffset+size_content;
  if (tmp_sender_pace_to_ptr>=tmp_receiver_pace) {
    if (tmp_sender_pace_to_ptr+size_need <= end_of_pipe_) {
      return CASSenderPaceTo_(tmp_sender_pace_to, tmp_sender_pace_to_ptr+size_need, false) ? 
        RCAST<Msg*>(tmp_sender_pace_to_ptr) : NULL;
    } else if (pipe_+size_need < tmp_receiver_pace) {
      bool ret = CASSenderPaceTo_(tmp_sender_pace_to, pipe_+size_need, true);
      if (unlikely(!ret)) return NULL;

      pos_before_rewind=tmp_sender_pace_to_ptr;
      return RCAST<Msg*>(pipe_);
    }
  } else if (tmp_receiver_pace>tmp_sender_pace_to_ptr
      && tmp_sender_pace_to_ptr+size_need < tmp_receiver_pace) {
    return CASSenderPaceTo_(tmp_sender_pace_to, tmp_sender_pace_to_ptr+size_need, false) ? 
      RCAST<Msg*>(tmp_sender_pace_to_ptr) : NULL;
  }
  return NULL;
}

template <typename MsgHeader, typename Notifier>
void MPSCFixedPipe<MsgHeader, Notifier>::JustSendMsg_(Msg* msg, char* pos_before_rewind) {
  SetReady_(msg);

  MB()

  size_t old_send_pace_from=sender_pace_from_;
  char* old_send_pace_from_ptr = DecodeSenderPacePtr_(old_send_pace_from);
  size_t current_version = DecodeSenderPaceVersion_(old_send_pace_from);
  if ( old_send_pace_from_ptr == RCAST<char*>(msg) ) {
  } else if ( RCAST<char*>(msg) == pipe_ ) {
    while (DecodeSenderPaceFromPtr_() != pos_before_rewind)
      ;

    current_version = (current_version+1) % kNumVersions;
    old_send_pace_from=EncodeSenderPace_(pipe_, current_version);
    old_send_pace_from_ptr=pipe_;
    EncodeSenderPaceFrom_(pipe_, current_version);
  } else {
    return;
  }

  size_t size_pace = GetMsgSize_((RCAST<Msg*>(old_send_pace_from_ptr)));
  if (unlikely(uint32_t(-1) == size_pace)) return;

  char* new_send_pace_from = old_send_pace_from_ptr+size_pace;
  bool ret = CASSenderPaceFrom_(old_send_pace_from, new_send_pace_from, false);
  if (unlikely(!ret)) return;

  while ( new_send_pace_from != DecodeSenderPaceToPtr_()
      && IsReady_(RCAST<Msg*>(new_send_pace_from)) ) {
    old_send_pace_from_ptr=new_send_pace_from;
    old_send_pace_from = EncodeSenderPace_(old_send_pace_from_ptr, current_version);
    size_pace = GetMsgSize_((RCAST<Msg*>(old_send_pace_from_ptr)));
    if (unlikely(uint32_t(-1) == size_pace)) break;

    new_send_pace_from = old_send_pace_from_ptr+size_pace;

    bool ret = CASSenderPaceFrom_(old_send_pace_from, new_send_pace_from, false);
    if (!ret) break;
  }
}

template <typename MsgHeader, typename Notifier>
bool MPSCFixedPipe<MsgHeader, Notifier>::HasTodoMPSCFixedPipeMsg_() {
  char* tmp_sender_pace_from = DecodeSenderPaceFromPtr_();
  if (receiver_pace_<tmp_sender_pace_from) { 
    return true;
  } else if (tmp_sender_pace_from==receiver_pace_) { 
    return false;
  } else if ( receiver_pace_+kMsgOffset > end_of_pipe_ 
      || IsBlank_(RCAST<Msg*>(receiver_pace_)) ) {
    receiver_pace_=pipe_;
    return tmp_sender_pace_from!=receiver_pace_; 
  } else { 
    return true; 
  }
}

template <typename MsgHeader, typename Notifier>
bool MPSCFixedPipe<MsgHeader, Notifier>::IsReady_(const Msg* msg) { 
  size_t size = msg->size;
  return !IsBlank_(size) && IsNotWriting_(size); 
}

template <typename MsgHeader, typename Notifier>
void MPSCFixedPipe<MsgHeader, Notifier>::SetBlank_(char* from, char* to) {
  memset(from, 255, to-from);
}

template <typename MsgHeader, typename Notifier>
char* MPSCFixedPipe<MsgHeader, Notifier>::DecodeSenderPaceFromPtr_() const {
  return DecodeSenderPacePtr_(sender_pace_from_);
}

template <typename MsgHeader, typename Notifier>
char* MPSCFixedPipe<MsgHeader, Notifier>::DecodeSenderPaceToPtr_() const {
  return DecodeSenderPacePtr_(sender_pace_to_);
}

template <typename MsgHeader, typename Notifier>
char* MPSCFixedPipe<MsgHeader, Notifier>::DecodeSenderPacePtr_(size_t pace) const {
  return RCAST<char*>(pace & kMaskPtr);
}

template <typename MsgHeader, typename Notifier>
size_t MPSCFixedPipe<MsgHeader, Notifier>::DecodeSenderPaceFromVersion_() const {
  return DecodeSenderPaceVersion_(sender_pace_from_);
}

template <typename MsgHeader, typename Notifier>
size_t MPSCFixedPipe<MsgHeader, Notifier>::DecodeSenderPaceToVersion_() const {
  return DecodeSenderPaceVersion_(sender_pace_to_);
}

template <typename MsgHeader, typename Notifier>
size_t MPSCFixedPipe<MsgHeader, Notifier>::DecodeSenderPaceVersion_(size_t pace) {
  return (pace&kMaskVersion) >> (64-kNumBitsVersion);
}

template <typename MsgHeader, typename Notifier>
void MPSCFixedPipe<MsgHeader, Notifier>::EncodeSenderPaceFrom_(char* ptr, size_t version) {
  sender_pace_from_ = EncodeSenderPace_(ptr, version);
}

template <typename MsgHeader, typename Notifier>
void MPSCFixedPipe<MsgHeader, Notifier>::EncodeSenderPaceTo_(char* ptr, size_t version) {
  sender_pace_to_ = EncodeSenderPace_(ptr, version);
}

template <typename MsgHeader, typename Notifier>
size_t MPSCFixedPipe<MsgHeader, Notifier>::EncodeSenderPace_(char* ptr, size_t version) {
  return RCAST<size_t>(ptr) | ( (size_t)version << (64-kNumBitsVersion) );
}

template <typename MsgHeader, typename Notifier>
bool MPSCFixedPipe<MsgHeader, Notifier>::CASSenderPaceFrom_(
    size_t old_pace, 
    char* new_pace, 
    bool new_version) {
  size_t version = DecodeSenderPaceVersion_(old_pace);
  return CAS_bool(
      &sender_pace_from_, 
      old_pace, 
      new_version ? 
        EncodeSenderPace_(new_pace, (version+1) % kNumVersions) :
        EncodeSenderPace_(new_pace, version) );
}

template <typename MsgHeader, typename Notifier>
bool MPSCFixedPipe<MsgHeader, Notifier>::CASSenderPaceTo_(
    size_t old_pace, 
    char* new_pace, 
    bool new_version) {
  size_t version = DecodeSenderPaceVersion_(old_pace);
  return CAS_bool(
      &sender_pace_to_, 
      old_pace, 
      new_version ? 
        EncodeSenderPace_(new_pace, (version+1) % kNumVersions) :
        EncodeSenderPace_(new_pace, version) );
}

template <typename MsgHeader, typename Notifier>
uint32_t MPSCFixedPipe<MsgHeader, Notifier>::GetMsgSize_(Msg* msg) { 
  size_t msg_size = msg->size;
  return uint32_t(-1) != msg_size ? kMsgOffset + (msg_size & ~Msg::kMaskNotWriting) : -1;
}

template <typename MsgHeader, typename Notifier>
void MPSCFixedPipe<MsgHeader, Notifier>::SleepUs(time_t usecs) {
  struct timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);
  time.tv_nsec += (usecs<<10);
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
}

}
