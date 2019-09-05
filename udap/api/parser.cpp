#include <functional>
#include <udap/api/parser.hpp>
#include <map>

namespace udap
{
  namespace api
  {
    std::map< std::string, std::function< IMessage*() > > funcmap = {
        {"CreateSession", []() { return new CreateSessionMessage; }},
    };

    MessageParser::MessageParser()
    {
      r.user   = this;
      r.on_key = &OnKey;
    }

    bool
    MessageParser::OnKey(dict_reader* r, udap_buffer_t* key)
    {
      MessageParser* self = static_cast< MessageParser* >(r->user);
      if(self->msg == nullptr && key == nullptr)  // empty message
        return false;
      if(self->msg == nullptr && key)
      {
        // first message, function name
        if(!udap_buffer_eq(*key, "f"))
          return false;
        udap_buffer_t strbuf;
        if(!bencode_read_string(r->buffer, &strbuf))
          return false;
        std::string funcname((char*)strbuf.cur, strbuf.sz);
        auto itr = funcmap.find(funcname);
        if(itr == funcmap.end())
          return false;
        self->msg = itr->second();
        return true;
      }
      else if(self->msg && key)
      {
        return self->msg->DecodeKey(*key, r->buffer);
      }
      else if(self->msg && key == nullptr)
      {
        return true;
      }
      return false;
    }

    IMessage*
    MessageParser::ParseMessage(udap_buffer_t buf)
    {
      if(bencode_read_dict(&buf, &r))
        return msg;
      return nullptr;
    }

  }  // namespace api
}  // namespace udap