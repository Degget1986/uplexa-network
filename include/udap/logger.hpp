#ifndef UDAP_LOGGER_HPP
#define UDAP_LOGGER_HPP

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace udap
{
  enum LogLevel
  {
    eLogDebug,
    eLogInfo,
    eLogWarn,
    eLogError
  };

  struct Logger
  {
    LogLevel minlevel = eLogInfo;
    std::ostream& out = std::cout;
  };

  extern Logger _glog;

  void
  SetLogLevel(LogLevel lvl);

  /** internal */
  template < typename TArg >
  void
  LogAppend(std::stringstream& ss, TArg&& arg) noexcept
  {
    ss << std::forward< TArg >(arg);
  }
  /** internal */
  template < typename TArg, typename... TArgs >
  void
  LogAppend(std::stringstream& ss, TArg&& arg, TArgs&&... args) noexcept
  {
    LogAppend(ss, std::forward< TArg >(arg));
    LogAppend(ss, std::forward< TArgs >(args)...);
  }

  /** internal */
  template < typename... TArgs >
  void
  _Log(LogLevel lvl, const char* fname, TArgs&&... args) noexcept
  {
    if(_glog.minlevel > lvl)
      return;

    std::stringstream ss;
    switch(lvl)
    {
      case eLogDebug:
        ss << (char)27 << "[0m";
        ss << "[DBG] ";
        break;
      case eLogInfo:
        ss << (char)27 << "[1m";
        ss << "[NFO] ";
        break;
      case eLogWarn:
        ss << (char)27 << "[1;33m";
        ss << "[WRN] ";
        break;
      case eLogError:
        ss << (char)27 << "[1;31m";
        ss << "[ERR] ";
        break;
    }
    std::time_t t;
    std::time(&t);
    std::string tag = fname;
    auto pos        = tag.rfind('/');
    if(pos != std::string::npos)
      tag = tag.substr(pos + 1);
    ss << std::put_time(std::localtime(&t), "%F %T") << " " << tag;
    auto sz = tag.size() % 8;
    while(sz--)
      ss << " ";
    ss << "\t";
    LogAppend(ss, std::forward< TArgs >(args)...);
    ss << (char)27 << "[0;0m";
    _glog.out << ss.str() << std::endl;
#ifdef SHADOW_TESTNET
    _glog.out << "\n" << std::flush;
#endif
  }
}  // namespace udap

#define Debug(x, ...) _Log(udap::eLogDebug, __FILE__, x, ##__VA_ARGS__)
#define Info(x, ...) _Log(udap::eLogInfo, __FILE__, x, ##__VA_ARGS__)
#define Warn(x, ...) _Log(udap::eLogWarn, __FILE__, x, ##__VA_ARGS__)
#define Error(x, ...) _Log(udap::eLogError, __FILE__, x, ##__VA_ARGS__)

#endif
