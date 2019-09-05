#ifndef UDAP_MESSAGES_DISCARD_HPP
#define UDAP_MESSAGES_DISCARD_HPP
#include <udap/link_message.hpp>

namespace udap
{
  const std::size_t MAX_DISCARD_SIZE = 10000;

  /// a dummy link message that is discarded
  struct DiscardMessage : public ILinkMessage
  {
    byte_t pad[MAX_DISCARD_SIZE];
    size_t sz = 0;

    DiscardMessage(const RouterID& id) : ILinkMessage(id)
    {
    }

    DiscardMessage(std::size_t padding) : ILinkMessage()
    {
      sz = padding;
      memset(pad, 'z', sz);
    }

    virtual bool
    DecodeKey(udap_buffer_t key, udap_buffer_t* buf)
    {
      udap_buffer_t strbuf;
      if(udap_buffer_eq(key, "v"))
      {
        if(!bencode_read_integer(buf, &version))
          return false;
        return version == UDAP_PROTO_VERSION;
      }
      if(udap_buffer_eq(key, "z"))
      {
        if(!bencode_read_string(buf, &strbuf))
          return false;
        if(strbuf.sz > MAX_DISCARD_SIZE)
          return false;
        sz = strbuf.sz;
        memcpy(pad, strbuf.base, sz);
        return true;
      }
      return false;
    }

    virtual bool
    BEncode(udap_buffer_t* buf) const
    {
      if(!bencode_start_dict(buf))
        return false;

      if(!bencode_write_bytestring(buf, "a", 1))
        return false;
      if(!bencode_write_bytestring(buf, "z", 1))
        return false;

      if(!bencode_write_version_entry(buf))
        return false;

      if(!bencode_write_bytestring(buf, "z", 1))
        return false;
      if(!bencode_write_bytestring(buf, pad, sz))
        return false;

      return bencode_end(buf);
    }

    virtual bool
    HandleMessage(udap_router* router) const
    {
      (void)router;
      udap::Info("got discard message of size ", sz, " bytes");
      return true;
    }
  };
}

#endif
