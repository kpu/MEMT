#ifndef _MEMT_Controller_Sentence_Config_h
#define _MEMT_Controller_Sentence_Config_h

#include "MEMT/Input/Config.hh"
#include "MEMT/Decoder/Config.hh"
#include "MEMT/Strategy/Legacy/Config.hh"
#include "MEMT/Output/Config.hh"

namespace controller {
namespace sentence {

struct Config {
  size_t num_systems;
  input::Config input;
  decoder::Config decoder;
  strategy::legacy::Config legacy;
  output::Config output;
};

} // namespace sentence
} // namespace controller

#endif // _MEMT_Controller_Sentence_Config_h
