#ifndef _MEMT_Strategy_Legacy_Config_h
#define _MEMT_Strategy_Legacy_Config_h

#include "MEMT/Strategy/Horizon/Config.hh"
#include "MEMT/Strategy/Phrase/Type.hh"
#include "MEMT/Feature/Scorer/Config.hh"

namespace strategy {
namespace legacy {

struct LegacyOnlyConfig {
  bool continue_recent;
  bool extend_aligned;
};

struct Config {
  phrase::Type phrase;
  horizon::Config horizon;
  feature::scorer::Config scorer;
  LegacyOnlyConfig legacy;
};

} // namespace legacy
} // namespace strategy

#endif // _MEMT_Strategy_Legacy_Config_h
