#include <udap/api/messages.hpp>

namespace udap
{
  namespace api
  {
    bool
    IMessage::BEncode(udap_buffer_t* buf) const
    {
      if(!bencode_start_dict(buf))
        return false;
      if(!BEncodeWriteDictString("F", FunctionName(), buf))
        return false;
      if(!BEncodeWriteDictInt(buf, "I", sessionID))
        return false;
      if(!BEncodeWriteDictInt(buf, "M", msgID))
        return false;
      if(!BEncodeWriteDictBEncodeList("P", GetParams(), buf))
        return false;
      if(!BEncodeWriteDictEntry("Z", hash, buf))
        return false;
      return bencode_end(buf);
    }

    bool
    IMessage::DecodeKey(udap_buffer_t key, udap_buffer_t* val)
    {
      if(udap_buffer_eq(key, "P"))
      {
        return DecodeParams(val);
      }
      bool read = false;
      if(!BEncodeMaybeReadDictInt("I", sessionID, read, key, val))
        return false;
      if(!BEncodeMaybeReadDictInt("M", msgID, read, key, val))
        return false;
      if(!BEncodeMaybeReadDictEntry("Z", hash, read, key, val))
        return false;
      return read;
    }

    bool
    IMessage::IsWellFormed(udap_crypto* crypto, const std::string& password)
    {
      // hash password
      udap::ShortHash secret;
      udap_buffer_t passbuf;
      passbuf.base = (byte_t*)password.c_str();
      passbuf.cur  = passbuf.base;
      passbuf.sz   = password.size();
      crypto->shorthash(secret, passbuf);

      udap::ShortHash digest, tmpHash;
      // save hash
      tmpHash = hash;
      // zero hash
      hash.Zero();

      // bencode
      byte_t tmp[1500];
      udap_buffer_t buf;
      buf.base = tmp;
      buf.cur  = buf.base;
      buf.sz   = sizeof(tmp);
      if(!BEncode(&buf))
        return false;

      // rewind buffer
      buf.sz  = buf.cur - buf.base;
      buf.cur = buf.base;
      // calculate message auth
      crypto->hmac(digest, buf, secret);
      // restore hash
      hash = tmpHash;
      return tmpHash == digest;
    }

    void
    IMessage::CalculateHash(udap_crypto* crypto, const std::string& password)
    {
      // hash password
      udap::ShortHash secret;
      udap_buffer_t passbuf;
      passbuf.base = (byte_t*)password.c_str();
      passbuf.cur  = passbuf.base;
      passbuf.sz   = password.size();
      crypto->shorthash(secret, passbuf);

      udap::ShortHash digest;
      // zero hash
      hash.Zero();

      // bencode
      byte_t tmp[1500];
      udap_buffer_t buf;
      buf.base = tmp;
      buf.cur  = buf.base;
      buf.sz   = sizeof(tmp);
      if(BEncode(&buf))
      {
        // rewind buffer
        buf.sz  = buf.cur - buf.base;
        buf.cur = buf.base;
        // calculate message auth
        crypto->hmac(hash, buf, secret);
      }
    }
  }  // namespace api
}  // namespace udap