#ifndef UDAP_CRYPTO_HPP
#define UDAP_CRYPTO_HPP

#include <udap/crypto.h>
#include <udap/mem.h>
#include <udap/threadpool.h>
#include <udap/aligned.hpp>

namespace udap
{
  const byte_t*
  seckey_topublic(const byte_t* secret);

  byte_t*
  seckey_topublic(byte_t* secret);

  typedef AlignedBuffer< 32 > SharedSecret;
  typedef AlignedBuffer< 32 > KeyExchangeNonce;

  typedef AlignedBuffer< PUBKEYSIZE > PubKey;

  struct PubKeyHash
  {
    std::size_t
    operator()(PubKey const& a) const noexcept
    {
      size_t sz = 0;
      memcpy(&sz, a.data(), sizeof(size_t));
      return sz;
    }
  };

  typedef AlignedBuffer< SECKEYSIZE > SecretKey;

  typedef AlignedBuffer< SHORTHASHSIZE > ShortHash;

  typedef AlignedBuffer< SIGSIZE > Signature;

  typedef AlignedBuffer< TUNNONCESIZE > TunnelNonce;

  typedef AlignedBuffer< 24 > SymmNonce;

  typedef AlignedBuffer< 32 > SymmKey;
}

#endif
