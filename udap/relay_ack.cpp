#include <udap/messages/relay_ack.hpp>

#include "router.hpp"

namespace udap
{
  bool
  LR_AckRecord::BEncode(udap_buffer_t* buf) const
  {
    return false;
  }

  bool
  LR_AckRecord::BDecode(udap_buffer_t* buf)
  {
    return false;
  }

  LR_AckMessage::LR_AckMessage(const RouterID& from) : ILinkMessage(from)
  {
  }
  LR_AckMessage::~LR_AckMessage()
  {
  }

  bool
  LR_AckMessage::BEncode(udap_buffer_t* buf) const
  {
    return false;
  }

  bool
  LR_AckMessage::DecodeKey(udap_buffer_t key, udap_buffer_t* buf)
  {
    return false;
  }

  struct LRAM_Decrypt
  {
    typedef AsyncFrameDecrypter< LRAM_Decrypt > Decrypter;

    udap_router* router;
    Decrypter* decrypt;
    std::vector< EncryptedFrame > frames;
    LR_AckRecord record;

    LRAM_Decrypt(udap_router* r, byte_t* seckey,
                 const std::vector< EncryptedFrame >& f)
        : router(r), frames(f)
    {
      decrypt = new Decrypter(&r->crypto, seckey, &Decrypted);
    }

    ~LRAM_Decrypt()
    {
      delete decrypt;
    }

    static void
    Decrypted(udap_buffer_t* buf, LRAM_Decrypt* self)
    {
      if(!buf)
      {
        udap::Error("Failed to decrypt LRAM frame");
        delete self;
        return;
      }
      if(!self->record.BDecode(buf))
      {
        udap::Error("LRAR invalid format");
        delete self;
        return;
      }
    }
  };

  bool
  LR_AckMessage::HandleMessage(udap_router* router) const
  {
    // TODO: use different private key for different path contexts as client
    LRAM_Decrypt* lram = new LRAM_Decrypt(router, router->encryption, replies);
    lram->decrypt->AsyncDecrypt(router->tp, &lram->frames[0], lram);
    return true;
  }
}