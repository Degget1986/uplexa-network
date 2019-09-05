#define NO_JEMALLOC
#include <udap/mem.h>
#include <cstdlib>

namespace udap
{
  void
  Zero(void *ptr, size_t sz)
  {
    uint8_t *p = (uint8_t *)ptr;
    while(sz--)
    {
      *p = 0;
      ++p;
    }
  }
}

extern "C" {

void
udap_mem_slab(struct udap_alloc *mem, uint32_t *buf, size_t sz)
{
  // not implemented
  abort();
}

bool
udap_eq(const void *a, const void *b, size_t sz)
{
  bool result          = true;
  const uint8_t *a_ptr = (const uint8_t *)a;
  const uint8_t *b_ptr = (const uint8_t *)b;
  while(sz--)
  {
    result &= a_ptr[sz] == b_ptr[sz];
  }
  return result;
}
}
