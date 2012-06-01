#ifndef _MEMT_Output_Config_h
#define _MEMT_Output_Config_h

namespace output {

struct Config {
  // Size of n-best list.
  unsigned int nbest;

  // Lowercase all output?  
  bool lowercase;

  // Capitialize initial word in sentence?  Only effective if capitalize_everything is false. 
  bool initial_cap;

  // Include scores?
  bool scores;

  // Include alignment back to original system and offset?
  bool alignment;

  bool flush_nbest;
};

} // namespace output
#endif // _MEMT_Output_Config_h
