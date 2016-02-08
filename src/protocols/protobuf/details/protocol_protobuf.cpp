#include "../protocol_protobuf.h"

namespace xforce { namespace magneto {

bool ProtocolWriteProtobuf::Encode() {
  bool ret = msg_->SerializeToString(&pb_data_);
  if (!ret) {
    return false;
  }

  uint32_t len_name = strlen(msg_name_);
  uint32_t len_name_n = htonl(len_name);
  uint32_t len_all_n = htonl(4 + len_name + 1 + pb_data_.size() + 4);
  out_.append(RCAST<char*>(&len_all_n), sizeof(len_all_n));
  out_.append(RCAST<char*>(&len_name_n), sizeof(len_name_n));
  out_.append(msg_name_, len_name+1);
  out_.append(pb_data_.data(), pb_data_.size());
  uint32_t checksum = static_cast<uint32_t>(
      ::adler32(1,
          reinterpret_cast<const Bytef*>(out_.data() + sizeof(len_all_n)),
          static_cast<int>(out_.size() - sizeof(len_all_n))));
  uint32_t checksum_n = htonl(checksum);
  out_.append(RCAST<char*>(&checksum_n), sizeof(checksum_n));
  return true;
}

int ProtocolWriteProtobuf::Write(int fd) {
  int ret = IOHelper::WriteNonBlock(fd, out_.data() + out_pos_, out_.size() - out_pos_);
  if (ret>0) {
    out_pos_+=ret;
    return out_.size() - out_pos_;
  } else {
    return 0==ret ? kEnd : ret;
  }
}

bool ProtocolReadProtobuf::Decode() {
  uint32_t checksum = static_cast<uint32_t>(
      ::adler32(1,
          reinterpret_cast<const Bytef*>(Body()),
          static_cast<int>(SizeBody() - sizeof(checksum))));
  if (checksum != ntohl(*(RCAST<const uint32_t*>(Body() + SizeBody() - sizeof(checksum))))) {
    return false;
  }

  uint32_t len_name = htonl(*(RCAST<const uint32_t*>(Body())));
  if ('\0' != *(Body() + 4 + len_name)) {
    return false;
  }

  const char* msg_name = Body() + 4;
  const char* pb_data = Body() + 5 + len_name;
  const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(msg_name);
  if (NULL!=descriptor) {
    const google::protobuf::Message* prototype =
    google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
    if (prototype) {
      msg_ = prototype->New();
    } else {
      return false;
    }
  } else {
    return false;
  }
  return msg_->ParseFromArray(pb_data, SizeBody() - 9 - strlen(msg_name));
}

ProtocolReadProtobuf::~ProtocolReadProtobuf() {
  XFC_DELETE(msg_)
}

}}
