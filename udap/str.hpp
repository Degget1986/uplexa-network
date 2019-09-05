#ifndef LIBUDAP_STR_HPP
#define LIBUDAP_STR_HPP
#include <cstring>

namespace udap
{
  static bool
  StrEq(const char *s1, const char *s2)
  {
    size_t sz1 = strlen(s1);
    size_t sz2 = strlen(s2);
    if(sz1 == sz2)
    {
      return strncmp(s1, s2, sz1) == 0;
    }
    else
      return false;
  }

}  // namespace udap

#endif
