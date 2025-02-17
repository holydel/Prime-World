/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*

IMPLEMENTATION DETAILS

TDenseProtocol was designed to have a smaller serialized form than
TBinaryProtocol.  This is accomplished using two techniques.  The first is
variable-length integer encoding.  We use the same technique that the Standard
MIDI File format uses for "variable-length quantities"
(http://en.wikipedia.org/wiki/Variable-length_quantity).
All integers (including i16, but not byte) are first cast to uint64_t,
then written out as variable-length quantities.  This has the unfortunate side
effect that all negative numbers require 10 bytes, but negative numbers tend
to be far less common than positive ones.

The second technique eliminating the field ids used by TBinaryProtocol.  This
decision required support from the Thrift compiler and also sacrifices some of
the backward and forward compatibility of TBinaryProtocol.

We considered implementing this technique by generating separate readers and
writers for the dense protocol (this is how Pillar, Thrift's predecessor,
worked), but this idea had a few problems:
- Our abstractions go out the window.
- We would have to maintain a second code generator.
- Preserving compatibility with old versions of the structures would be a
  nightmare.

Therefore, we chose an alternate implementation that stored the description of
the data neither in the data itself (like TBinaryProtocol) nor in the
serialization code (like Pillar), but instead in a separate data structure,
called a TypeSpec.  TypeSpecs are generated by the Thrift compiler
(specifically in the t_cpp_generator), and their structure should be
documented there (TODO(dreiss): s/should be/is/).

We maintain a stack of TypeSpecs within the protocol so it knows where the
generated code is in the reading/writing process.  For example, if we are
writing an i32 contained in a struct bar, contained in a struct foo, then the
stack would look like: TOP , i32 , struct bar , struct foo , BOTTOM.
The following invariant: whenever we are about to read/write an object
(structBegin, containerBegin, or a scalar), the TypeSpec on the top of the
stack must match the type being read/written.  The main reasons that this
invariant must be maintained is that if we ever start reading a structure, we
must have its exact TypeSpec in order to pass the right tags to the
deserializer.

We use the following strategies for maintaining this invariant:

- For structures, we have a separate stack of indexes, one for each structure
  on the TypeSpec stack.  These are indexes into the list of fields in the
  structure's TypeSpec.  When we {read,write}FieldBegin, we push on the
  TypeSpec for the field.
- When we begin writing a list or set, we push on the TypeSpec for the
  element type.
- For maps, we have a separate stack of booleans, one for each map on the
  TypeSpec stack.  The boolean is true if we are writing the key for that
  map, and false if we are writing the value.  Maps are the trickiest case
  because the generated code does not call any protocol method between
  the key and the value.  As a result, we potentially have to switch
  between map key state and map value state after reading/writing any object.
- This job is handled by the stateTransition method.  It is called after
  reading/writing every object.  It pops the current TypeSpec off the stack,
  then optionally pushes a new one on, depending on what the next TypeSpec is.
  If it is a struct, the job is left to the next writeFieldBegin.  If it is a
  set or list, the just-popped typespec is pushed back on.  If it is a map,
  the top of the key/value stack is toggled, and the appropriate TypeSpec
  is pushed.

Optional fields are a little tricky also.  We write a zero byte if they are
absent and prefix them with an 0x01 byte if they are present
*/

#define __STDC_LIMIT_MACROS
#include <thrift/windows/config.h>
#include <thrift/protocol/TDenseProtocol.h>
#include <thrift/TReflectionLocal.h>

// Leaving this on for now.  Disabling it will turn off asserts, which should
// give a performance boost.  When we have *really* thorough test cases,
// we should drop this.
#define DEBUG_TDENSEPROTOCOL

// NOTE: Assertions should *only* be used to detect bugs in code,
//       either in TDenseProtocol itself, or in code using it.
//       (For example, using the wrong TypeSpec.)
//       Invalid data should NEVER cause an assertion failure,
//       no matter how grossly corrupted, nor how ingeniously crafted.
#ifdef DEBUG_TDENSEPROTOCOL
#undef NDEBUG
#else
#define NDEBUG
#endif
#include <cassert>

using std::string;

#ifdef __GNUC__
#define UNLIKELY(val) (__builtin_expect((val), 0))
#else
#define UNLIKELY(val) (val)
#endif

namespace apache { namespace thrift { namespace protocol {

const int TDenseProtocol::FP_PREFIX_LEN =
  apache::thrift::reflection::local::FP_PREFIX_LEN;

// Top TypeSpec.  TypeSpec of the structure being encoded.
#define TTS  (ts_stack_.back())  // type = TypeSpec*
// InDeX.  Index into TTS of the current/next field to encode.
#define IDX (idx_stack_.back())  // type = int
// Field TypeSpec.  TypeSpec of the current/next field to encode.
#define FTS (TTS->tstruct.specs[IDX])  // type = TypeSpec*
// Field MeTa.  Metadata of the current/next field to encode.
#define FMT (TTS->tstruct.metas[IDX])  // type = FieldMeta
// SubType 1/2.  TypeSpec of the first/second subtype of this container.
#define ST1 (TTS->tcontainer.subtype1)
#define ST2 (TTS->tcontainer.subtype2)


/**
 * Checks that @c ttype is indeed the ttype that we should be writing,
 * according to our typespec.  Aborts if the test fails and debugging in on.
 */
inline void TDenseProtocol::checkTType(const TType ttype) {
  assert(!ts_stack_.empty());
  assert(TTS->ttype == ttype);
}

/**
 * Makes sure that the TypeSpec stack is correct for the next object.
 * See top-of-file comments.
 */
inline void TDenseProtocol::stateTransition() {
  TypeSpec* old_tts = ts_stack_.back();
  ts_stack_.pop_back();

  // If this is the end of the top-level write, we should have just popped
  // the TypeSpec passed to the constructor.
  if (ts_stack_.empty()) {
    assert(old_tts = type_spec_);
    return;
  }

  switch (TTS->ttype) {

    case T_STRUCT:
      assert(old_tts == FTS);
      break;

    case T_LIST:
    case T_SET:
      assert(old_tts == ST1);
      ts_stack_.push_back(old_tts);
      break;

    case T_MAP:
      assert(old_tts == (mkv_stack_.back() ? ST1 : ST2));
      mkv_stack_.back() = !mkv_stack_.back();
      ts_stack_.push_back(mkv_stack_.back() ? ST1 : ST2);
      break;

    default:
      assert(!"Invalid TType in stateTransition.");
      break;

  }
}


/*
 * Variable-length quantity functions.
 */

inline uint32_t TDenseProtocol::vlqRead(uint64_t& vlq) {
  uint32_t used = 0;
  uint64_t val = 0;
  uint8_t buf[10];  // 64 bits / (7 bits/byte) = 10 bytes.
  uint32_t buf_size = sizeof(buf);
  const uint8_t* borrowed = trans_->borrow(buf, &buf_size);

  // Fast path.  TODO(dreiss): Make it faster.
  if (borrowed != NULL) {
    while (true) {
      uint8_t byte = borrowed[used];
      used++;
      val = (val << 7) | (byte & 0x7f);
      if (!(byte & 0x80)) {
        vlq = val;
        trans_->consume(used);
        return used;
      }
      // Have to check for invalid data so we don't crash.
      if (UNLIKELY(used == sizeof(buf))) {
        resetState();
        throw TProtocolException(TProtocolException::INVALID_DATA, "Variable-length int over 10 bytes.");
      }
    }
  }

  // Slow path.
  else {
    while (true) {
      uint8_t byte;
      used += trans_->readAll(&byte, 1);
      val = (val << 7) | (byte & 0x7f);
      if (!(byte & 0x80)) {
        vlq = val;
        return used;
      }
      // Might as well check for invalid data on the slow path too.
      if (UNLIKELY(used >= sizeof(buf))) {
        resetState();
        throw TProtocolException(TProtocolException::INVALID_DATA, "Variable-length int over 10 bytes.");
      }
    }
  }
}

inline uint32_t TDenseProtocol::vlqWrite(uint64_t vlq) {
  uint8_t buf[10];  // 64 bits / (7 bits/byte) = 10 bytes.
  int32_t pos = sizeof(buf) - 1;

  // Write the thing from back to front.
  buf[pos] = vlq & 0x7f;
  vlq >>= 7;
  pos--;

  while (vlq > 0) {
    assert(pos >= 0);
    buf[pos] = static_cast<uint8_t>(vlq | 0x80);
    vlq >>= 7;
    pos--;
  }

  // Back up one step before writing.
  pos++;

  trans_->write(buf+pos, static_cast<uint32_t>(sizeof(buf) - pos));
  return static_cast<uint32_t>(sizeof(buf) - pos);
}



/*
 * Writing functions.
 */

uint32_t TDenseProtocol::writeMessageBegin(const std::string& name,
                                           const TMessageType messageType,
                                           const int32_t seqid) {
  throw TException("TDenseProtocol doesn't work with messages (yet).");

  int32_t version = (VERSION_2) | ((int32_t)messageType);
  uint32_t wsize = 0;
  wsize += subWriteI32(version);
  wsize += subWriteString(name);
  wsize += subWriteI32(seqid);
  return wsize;
}

uint32_t TDenseProtocol::writeMessageEnd() {
  return 0;
}

uint32_t TDenseProtocol::writeStructBegin(const char* name) {
  (void) name;
  uint32_t xfer = 0;

  // The TypeSpec stack should be empty if this is the top-level read/write.
  // If it is, we push the TypeSpec passed to the constructor.
  if (ts_stack_.empty()) {
    assert(standalone_);

    if (type_spec_ == NULL) {
      resetState();
      throw TException("TDenseProtocol: No type specified.");
    } else {
      assert(type_spec_->ttype == T_STRUCT);
      ts_stack_.push_back(type_spec_);
      // Write out a prefix of the structure fingerprint.
      trans_->write(type_spec_->fp_prefix, FP_PREFIX_LEN);
      xfer += FP_PREFIX_LEN;
    }
  }

  // We need a new field index for this structure.
  idx_stack_.push_back(0);
  return 0;
}

uint32_t TDenseProtocol::writeStructEnd() {
  idx_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::writeFieldBegin(const char* name,
                                         const TType fieldType,
                                         const int16_t fieldId) {
  (void) name;
  uint32_t xfer = 0;

  // Skip over optional fields.
  while (FMT.tag != fieldId) {
    // TODO(dreiss): Old meta here.
    assert(FTS->ttype != T_STOP);
    assert(FMT.is_optional);
    // Write a zero byte so the reader can skip it.
    xfer += subWriteBool(false);
    // And advance to the next field.
    IDX++;
  }

  // TODO(dreiss): give a better exception.
  assert(FTS->ttype == fieldType);

  if (FMT.is_optional) {
    subWriteBool(true);
    xfer += 1;
  }

  // writeFieldStop shares all lot of logic up to this point.
  // Instead of replicating it all, we just call this method from that one
  // and use a gross special case here.
  if (UNLIKELY(FTS->ttype != T_STOP)) {
    // For normal fields, push the TypeSpec that we're about to use.
    ts_stack_.push_back(FTS);
  }
  return xfer;
}

uint32_t TDenseProtocol::writeFieldEnd() {
  // Just move on to the next field.
  IDX++;
  return 0;
}

uint32_t TDenseProtocol::writeFieldStop() {
  return TDenseProtocol::writeFieldBegin("", T_STOP, 0);
}

uint32_t TDenseProtocol::writeMapBegin(const TType keyType,
                                       const TType valType,
                                       const uint32_t size) {
  checkTType(T_MAP);

  assert(keyType == ST1->ttype);
  assert(valType == ST2->ttype);

  ts_stack_.push_back(ST1);
  mkv_stack_.push_back(true);

  return subWriteI32((int32_t)size);
}

uint32_t TDenseProtocol::writeMapEnd() {
  // Pop off the value type, as well as our entry in the map key/value stack.
  // stateTransition takes care of popping off our TypeSpec.
  ts_stack_.pop_back();
  mkv_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::writeListBegin(const TType elemType,
                                        const uint32_t size) {
  checkTType(T_LIST);

  assert(elemType == ST1->ttype);
  ts_stack_.push_back(ST1);
  return subWriteI32((int32_t)size);
}

uint32_t TDenseProtocol::writeListEnd() {
  // Pop off the element type.  stateTransition takes care of popping off ours.
  ts_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::writeSetBegin(const TType elemType,
                                       const uint32_t size) {
  checkTType(T_SET);

  assert(elemType == ST1->ttype);
  ts_stack_.push_back(ST1);
  return subWriteI32((int32_t)size);
}

uint32_t TDenseProtocol::writeSetEnd() {
  // Pop off the element type.  stateTransition takes care of popping off ours.
  ts_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::writeBool(const bool value) {
  checkTType(T_BOOL);
  stateTransition();
  return TBinaryProtocol::writeBool(value);
}

uint32_t TDenseProtocol::writeByte(const int8_t byte) {
  checkTType(T_BYTE);
  stateTransition();
  return TBinaryProtocol::writeByte(byte);
}

uint32_t TDenseProtocol::writeI16(const int16_t i16) {
  checkTType(T_I16);
  stateTransition();
  return vlqWrite(i16);
}

uint32_t TDenseProtocol::writeI32(const int32_t i32) {
  checkTType(T_I32);
  stateTransition();
  return vlqWrite(i32);
}

uint32_t TDenseProtocol::writeI64(const int64_t i64) {
  checkTType(T_I64);
  stateTransition();
  return vlqWrite(i64);
}

uint32_t TDenseProtocol::writeDouble(const double dub) {
  checkTType(T_DOUBLE);
  stateTransition();
  return TBinaryProtocol::writeDouble(dub);
}

uint32_t TDenseProtocol::writeString(const std::string& str) {
  checkTType(T_STRING);
  stateTransition();
  return subWriteString(str);
}

uint32_t TDenseProtocol::writeBinary(const std::string& str) {
  return TDenseProtocol::writeString(str);
}

inline uint32_t TDenseProtocol::subWriteI32(const int32_t i32) {
  return vlqWrite(i32);
}

uint32_t TDenseProtocol::subWriteString(const std::string& str) {
  if(str.size() > static_cast<size_t>((std::numeric_limits<int32_t>::max)()))
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  uint32_t size = static_cast<uint32_t>(str.size());
  uint32_t xfer = subWriteI32((int32_t)size);
  if (size > 0) {
    trans_->write((uint8_t*)str.data(), size);
  }
  return xfer + size;
}



/*
 * Reading functions
 *
 * These have a lot of the same logic as the writing functions, so if
 * something is confusing, look for comments in the corresponding writer.
 */

uint32_t TDenseProtocol::readMessageBegin(std::string& name,
                                          TMessageType& messageType,
                                          int32_t& seqid) {
  throw TException("TDenseProtocol doesn't work with messages (yet).");

  uint32_t xfer = 0;
  int32_t sz;
  xfer += subReadI32(sz);

  if (sz < 0) {
    // Check for correct version number
    int32_t version = sz & VERSION_MASK;
    if (version != VERSION_2) {
      throw TProtocolException(TProtocolException::BAD_VERSION, "Bad version identifier");
    }
    messageType = (TMessageType)(sz & 0x000000ff);
    xfer += subReadString(name);
    xfer += subReadI32(seqid);
  } else {
    throw TProtocolException(TProtocolException::BAD_VERSION, "No version identifier... old protocol client in strict mode?");
  }
  return xfer;
}

uint32_t TDenseProtocol::readMessageEnd() {
  return 0;
}

uint32_t TDenseProtocol::readStructBegin(string& name) {
  (void) name;
  uint32_t xfer = 0;

  if (ts_stack_.empty()) {
    assert(standalone_);

    if (type_spec_ == NULL) {
      resetState();
      throw TException("TDenseProtocol: No type specified.");
    } else {
      assert(type_spec_->ttype == T_STRUCT);
      ts_stack_.push_back(type_spec_);

      // Check the fingerprint prefix.
      uint8_t buf[FP_PREFIX_LEN];
      xfer += trans_->read(buf, FP_PREFIX_LEN);
      if (std::memcmp(buf, type_spec_->fp_prefix, FP_PREFIX_LEN) != 0) {
        resetState();
        throw TProtocolException(TProtocolException::INVALID_DATA,
            "Fingerprint in data does not match type_spec.");
      }
    }
  }

  // We need a new field index for this structure.
  idx_stack_.push_back(0);
  return 0;
}

uint32_t TDenseProtocol::readStructEnd() {
  idx_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::readFieldBegin(string& name,
                                        TType& fieldType,
                                        int16_t& fieldId) {
  (void) name;
  uint32_t xfer = 0;

  // For optional fields, check to see if they are there.
  while (FMT.is_optional) {
    bool is_present;
    xfer += subReadBool(is_present);
    if (is_present) {
      break;
    }
    IDX++;
  }

  // Once we hit a mandatory field, or an optional field that is present,
  // we know that FMT and FTS point to the appropriate field.

  fieldId   = FMT.tag;
  fieldType = FTS->ttype;

  // Normally, we push the TypeSpec that we are about to read,
  // but no reading is done for T_STOP.
  if (FTS->ttype != T_STOP) {
    ts_stack_.push_back(FTS);
  }
  return xfer;
}

uint32_t TDenseProtocol::readFieldEnd() {
  IDX++;
  return 0;
}

uint32_t TDenseProtocol::readMapBegin(TType& keyType,
                                      TType& valType,
                                      uint32_t& size) {
  checkTType(T_MAP);

  uint32_t xfer = 0;
  int32_t sizei;
  xfer += subReadI32(sizei);
  if (sizei < 0) {
    resetState();
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  } else if (container_limit_ && sizei > container_limit_) {
    resetState();
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }
  size = (uint32_t)sizei;

  keyType = ST1->ttype;
  valType = ST2->ttype;

  ts_stack_.push_back(ST1);
  mkv_stack_.push_back(true);

  return xfer;
}

uint32_t TDenseProtocol::readMapEnd() {
  ts_stack_.pop_back();
  mkv_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::readListBegin(TType& elemType,
                                       uint32_t& size) {
  checkTType(T_LIST);

  uint32_t xfer = 0;
  int32_t sizei;
  xfer += subReadI32(sizei);
  if (sizei < 0) {
    resetState();
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  } else if (container_limit_ && sizei > container_limit_) {
    resetState();
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }
  size = (uint32_t)sizei;

  elemType = ST1->ttype;

  ts_stack_.push_back(ST1);

  return xfer;
}

uint32_t TDenseProtocol::readListEnd() {
  ts_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::readSetBegin(TType& elemType,
                                      uint32_t& size) {
  checkTType(T_SET);

  uint32_t xfer = 0;
  int32_t sizei;
  xfer += subReadI32(sizei);
  if (sizei < 0) {
    resetState();
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  } else if (container_limit_ && sizei > container_limit_) {
    resetState();
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }
  size = (uint32_t)sizei;

  elemType = ST1->ttype;

  ts_stack_.push_back(ST1);

  return xfer;
}

uint32_t TDenseProtocol::readSetEnd() {
  ts_stack_.pop_back();
  stateTransition();
  return 0;
}

uint32_t TDenseProtocol::readBool(bool& value) {
  checkTType(T_BOOL);
  stateTransition();
  return TBinaryProtocol::readBool(value);
}

uint32_t TDenseProtocol::readByte(int8_t& byte) {
  checkTType(T_BYTE);
  stateTransition();
  return TBinaryProtocol::readByte(byte);
}

uint32_t TDenseProtocol::readI16(int16_t& i16) {
  checkTType(T_I16);
  stateTransition();
  uint64_t u64;
  uint32_t rv = vlqRead(u64);
  int64_t val = (int64_t)u64;
  if (UNLIKELY(val > INT16_MAX || val < INT16_MIN)) {
    resetState();
    throw TProtocolException(TProtocolException::INVALID_DATA,
                             "i16 out of range.");
  }
  i16 = (int16_t)val;
  return rv;
}

uint32_t TDenseProtocol::readI32(int32_t& i32) {
  checkTType(T_I32);
  stateTransition();
  uint64_t u64;
  uint32_t rv = vlqRead(u64);
  int64_t val = (int64_t)u64;
  if (UNLIKELY(val > INT32_MAX || val < INT32_MIN)) {
    resetState();
    throw TProtocolException(TProtocolException::INVALID_DATA,
                             "i32 out of range.");
  }
  i32 = (int32_t)val;
  return rv;
}

uint32_t TDenseProtocol::readI64(int64_t& i64) {
  checkTType(T_I64);
  stateTransition();
  uint64_t u64;
  uint32_t rv = vlqRead(u64);
  int64_t val = (int64_t)u64;
  if (UNLIKELY(val > INT64_MAX || val < INT64_MIN)) {
    resetState();
    throw TProtocolException(TProtocolException::INVALID_DATA,
                             "i64 out of range.");
  }
  i64 = (int64_t)val;
  return rv;
}

uint32_t TDenseProtocol::readDouble(double& dub) {
  checkTType(T_DOUBLE);
  stateTransition();
  return TBinaryProtocol::readDouble(dub);
}

uint32_t TDenseProtocol::readString(std::string& str) {
  checkTType(T_STRING);
  stateTransition();
  return subReadString(str);
}

uint32_t TDenseProtocol::readBinary(std::string& str) {
  return TDenseProtocol::readString(str);
}

uint32_t TDenseProtocol::subReadI32(int32_t& i32) {
  uint64_t u64;
  uint32_t rv = vlqRead(u64);
  int64_t val = (int64_t)u64;
  if (UNLIKELY(val > INT32_MAX || val < INT32_MIN)) {
    resetState();
    throw TProtocolException(TProtocolException::INVALID_DATA,
                             "i32 out of range.");
  }
  i32 = (int32_t)val;
  return rv;
}

uint32_t TDenseProtocol::subReadString(std::string& str) {
  uint32_t xfer;
  int32_t size;
  xfer = subReadI32(size);
  return xfer + readStringBody(str, size);
}

}}} // apache::thrift::protocol
