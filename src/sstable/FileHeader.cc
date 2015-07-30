/**
 * This file is part of the "libsstable" project
 *   Copyright (c) 2015 Paul Asmuth, FnordCorp B.V.
 *
 * FnordMetric is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License v3.0. You should have received a
 * copy of the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <stx/fnv.h>
#include <sstable/FileHeader.h>
#include <sstable/binaryformat.h>

namespace stx {
namespace sstable {

FileHeader FileHeader::createMetaPage(
    const void* userdata,
    size_t userdata_size) {

  FNV<uint32_t> fnv;
  FileHeader hdr;

  hdr.version_ = BinaryFormat::kVersion;
  hdr.userdata_size_ = userdata_size;
  hdr.userdata_checksum_ = fnv.hash(userdata, userdata_size);
  return hdr;
}

FileHeader FileHeader::readMetaPage(InputStream* is) {
  FileHeader hdr;

  auto magic_bytes = is->readUInt32();
  if (magic_bytes != BinaryFormat::kMagicBytes) {
    RAISE(kIllegalStateError, "not a valid sstable");
  }

  hdr.version_ = is->readUInt16();
  hdr.userdata_offset_ = 6;

  switch (hdr.version_) {

    case 0x1:
      hdr.flags_ = 0;
      break;

    case 0x2:
      hdr.flags_ = is->readUInt64();
      hdr.userdata_offset_ += 8;
      break;

    default:
      RAISE(kIllegalStateError, "unsupported sstable version");

  }

  hdr.body_size_ = is->readUInt64();
  hdr.userdata_checksum_ = is->readUInt32();
  hdr.userdata_size_ = is->readUInt32();
  hdr.userdata_offset_ += 16;

  /* pre version 0x02 body_size > 0 implied that the table is finalized */
  if (hdr.version_ == 0x01 && hdr.body_size_ > 0) {
    hdr.flags_ |= (uint64_t) FileHeaderFlags::FINALIZED;
  }

  return hdr;
}

FileHeader::FileHeader() :
  version_(0),
  flags_(0),
  body_size_(0),
  userdata_checksum_(0),
  userdata_size_(0),
  userdata_offset_(0) {}

size_t FileHeader::headerSize() const {
  return bodyOffset();
}

size_t FileHeader::bodySize() const {
  return body_size_;
}

size_t FileHeader::bodyOffset() const {
  return userdata_offset_ + userdata_size_;
}

bool FileHeader::isFinalized() const {
  return (flags_ & (uint64_t) FileHeaderFlags::FINALIZED) > 0;
}

size_t FileHeader::userdataSize() const {
  return userdata_size_;
}

size_t FileHeader::userdataOffset() const {
  return userdata_offset_;
}

uint32_t FileHeader::userdataChecksum() const {
  return userdata_checksum_;
}

}
}
