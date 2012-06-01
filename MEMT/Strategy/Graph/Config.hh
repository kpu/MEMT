#ifndef _MEMT_Strategy_Graph_Config_h
#define _MEMT_Strategy_Graph_Config_h

#include "MEMT/Strategy/Scorer/Config.hh"
#include "MEMT/Strategy/Graph/Coverage/Config.hh"

namespace strategy {
namespace graph {

struct Config {
  coverage::Config coverage;
  scorer::Config scorer;
};

} // namespace graph
} // namespace strategy

#endif // _MEMT_Strategy_Graph_Config_h
