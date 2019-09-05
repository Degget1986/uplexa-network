#ifndef UDAP_BENCODE_HPP
#define UDAP_BENCODE_HPP

#include <udap/bencode.h>
#include <udap/logger.hpp>

namespace udap
{
  inline bool
  BEncodeWriteDictMsgType(udap_buffer_t* buf, const char* k, const char* t)
  {
    return bencode_write_bytestring(buf, k, 1)
        && bencode_write_bytestring(buf, t, 1);
  }
  template < typename Obj_t >
  bool
  BEncodeWriteDictString(const char* k, const Obj_t& str, udap_buffer_t* buf)
  {
    return bencode_write_bytestring(buf, k, 1)
        && bencode_write_bytestring(buf, str.c_str(), str.size());
  }

  template < typename Obj_t >
  bool
  BEncodeWriteDictEntry(const char* k, const Obj_t& o, udap_buffer_t* buf)
  {
    return bencode_write_bytestring(buf, k, 1) && o.BEncode(buf);
  }

  template < typename Int_t >
  bool
  BEncodeWriteDictInt(udap_buffer_t* buf, const char* k, const Int_t& i)
  {
    return bencode_write_bytestring(buf, k, 1) && bencode_write_uint64(buf, i);
  }

  template < typename Item_t >
  bool
  BEncodeRead(Item_t& item, udap_buffer_t* buf);

  template < typename Item_t >
  bool
  BEncodeMaybeReadDictEntry(const char* k, Item_t& item, bool& read,
                            udap_buffer_t key, udap_buffer_t* buf)
  {
    if(udap_buffer_eq(key, k))
    {
      if(!item.BDecode(buf))
      {
        udap::Warn("failed to decode key ", k);
        return false;
      }
      read = true;
    }
    return true;
  }

  template < typename Int_t >
  bool
  BEncodeMaybeReadDictInt(const char* k, Int_t& i, bool& read,
                          udap_buffer_t key, udap_buffer_t* buf)
  {
    if(udap_buffer_eq(key, k))
    {
      if(!bencode_read_integer(buf, &i))
      {
        udap::Warn("failed to decode key ", k);
        return false;
      }
      read = true;
    }
    return true;
  }

  template < typename Item_t >
  bool
  BEncodeMaybeReadVersion(const char* k, Item_t& item, uint64_t expect,
                          bool& read, udap_buffer_t key, udap_buffer_t* buf)
  {
    if(udap_buffer_eq(key, k))
    {
      if(!bencode_read_integer(buf, &item))
        return false;
      read = item == expect;
    }
    return true;
  }

  template < typename List_t >
  bool
  BEncodeWriteDictBEncodeList(const char* k, const List_t& l,
                              udap_buffer_t* buf)
  {
    if(!bencode_write_bytestring(buf, k, 1))
      return false;
    if(!bencode_start_list(buf))
      return false;

    for(const auto& item : l)
      if(!item->BEncode(buf))
        return false;
    return bencode_end(buf);
  }

  template < typename Iter >
  bool
  BEncodeWriteList(Iter itr, Iter end, udap_buffer_t* buf)
  {
    if(!bencode_start_list(buf))
      return false;
    while(itr != end)
      if(!itr->BEncode(buf))
        return false;
      else
        ++itr;
    return bencode_end(buf);
  }

  template < typename List_t >
  bool
  BEncodeReadList(List_t& result, udap_buffer_t* buf)
  {
    if(*buf->cur != 'l')  // ensure is a list
      return false;

    buf->cur++;
    while(udap_buffer_size_left(*buf) && *buf->cur != 'e')
    {
      if(!result.emplace(result.end())->BDecode(buf))
        return false;
    }
    if(*buf->cur != 'e')  // make sure we're at a list end
      return false;
    buf->cur++;
    return true;
  }

  template < typename List_t >
  bool
  BEncodeWriteDictList(const char* k, List_t& list, udap_buffer_t* buf)
  {
    return bencode_write_bytestring(buf, k, 1)
        && BEncodeWriteList(list.begin(), list.end(), buf);
  }

  /// bencode serializable message
  struct IBEncodeMessage
  {
    virtual ~IBEncodeMessage(){};

    virtual bool
    DecodeKey(udap_buffer_t key, udap_buffer_t* val) = 0;

    virtual bool
    BEncode(udap_buffer_t* buf) const = 0;
  };

}  // namespace udap

#endif