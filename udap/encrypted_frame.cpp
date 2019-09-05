#include <udap/crypto.hpp>
#include <udap/encrypted_frame.hpp>
#include "logger.hpp"
#include "mem.hpp"

namespace udap
{
  Encrypted::Encrypted()
  {
    UpdateBuffer();
  }

  Encrypted::Encrypted(const byte_t* buf, size_t sz) : _sz(sz)
  {
    _data = new byte_t[sz];
    if(buf)
      memcpy(data(), buf, sz);
    else
      udap::Zero(data(), sz);
    UpdateBuffer();
  }
  Encrypted::~Encrypted()
  {
    if(_data)
      delete[] _data;
  }

  Encrypted::Encrypted(size_t sz) : Encrypted(nullptr, sz)
  {
  }

  bool
  EncryptedFrame::EncryptInPlace(byte_t* ourSecretKey, byte_t* otherPubkey,
                                 udap_crypto* crypto)
  {
    // format of frame is
    // <32 bytes keyed hash of following data>
    // <32 bytes nonce>
    // <32 bytes pubkey>
    // <N bytes encrypted payload>
    //
    byte_t* hash   = data();
    byte_t* nonce  = hash + SHORTHASHSIZE;
    byte_t* pubkey = nonce + TUNNONCESIZE;
    byte_t* body   = pubkey + PUBKEYSIZE;

    SharedSecret shared;

    auto DH      = crypto->dh_client;
    auto Encrypt = crypto->xchacha20;
    auto MDS     = crypto->hmac;

    udap_buffer_t buf;
    buf.base = body;
    buf.cur  = buf.base;
    buf.sz   = size() - EncryptedFrame::OverheadSize;

    // set our pubkey
    memcpy(pubkey, udap::seckey_topublic(ourSecretKey), PUBKEYSIZE);
    // randomize nonce
    crypto->randbytes(nonce, TUNNONCESIZE);

    // derive shared key
    if(!DH(shared, otherPubkey, ourSecretKey, nonce))
    {
      udap::Error("DH failed");
      return false;
    }

    // encrypt body
    if(!Encrypt(buf, shared, nonce))
    {
      udap::Error("encrypt failed");
      return false;
    }

    // generate message auth
    buf.base = nonce;
    buf.cur  = buf.base;
    buf.sz   = size() - SHORTHASHSIZE;

    if(!MDS(hash, buf, shared))
    {
      udap::Error("Failed to generate messgae auth");
      return false;
    }
    return true;
  }

  bool
  EncryptedFrame::DecryptInPlace(byte_t* ourSecretKey, udap_crypto* crypto)
  {
    if(size() <= size_t(EncryptedFrame::OverheadSize))
    {
      udap::Warn("encrypted frame too small, ", size(),
                  " <= ", size_t(EncryptedFrame::OverheadSize));
      return false;
    }
    // format of frame is
    // <32 bytes keyed hash of following data>
    // <32 bytes nonce>
    // <32 bytes pubkey>
    // <N bytes encrypted payload>
    //
    byte_t* hash        = data();
    byte_t* nonce       = hash + SHORTHASHSIZE;
    byte_t* otherPubkey = nonce + TUNNONCESIZE;
    byte_t* body        = otherPubkey + PUBKEYSIZE;

    // use dh_server becuase we are not the creator of this message
    auto DH      = crypto->dh_server;
    auto Decrypt = crypto->xchacha20;
    auto MDS     = crypto->hmac;

    udap_buffer_t buf;
    buf.base = nonce;
    buf.cur  = buf.base;
    buf.sz   = size() - SHORTHASHSIZE;

    SharedSecret shared;
    ShortHash digest;

    if(!DH(shared, otherPubkey, ourSecretKey, nonce))
    {
      udap::Error("DH failed");
      return false;
    }

    if(!MDS(digest, buf, shared))
    {
      udap::Error("Digest failed");
      return false;
    }

    if(memcmp(digest, hash, digest.size()))
    {
      udap::Error("message authentication failed");
      return false;
    }

    buf.base = body;
    buf.cur  = body;
    buf.sz   = size() - EncryptedFrame::OverheadSize;

    if(!Decrypt(buf, shared, nonce))
    {
      udap::Error("decrypt failed");
      return false;
    }
    return true;
  }
}  // namespace udap
