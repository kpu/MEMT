#ifndef _MEMT_Output_StderrBeamDumper_h
#define _MEMT_Output_StderrBeamDumper_h

#include <iostream>

namespace output {

class StderrBeamDumper {
  public:
    template <class Beam> void DumpBeam(unsigned int length, const Beam &beam) {
      std::cerr << "Length " << length << '\n';
      std::vector<typename Beam::Value*> values;
      for (typename Beam::unordered_iterator i = beam.unordered_begin(); i != beam.unordered_end(); ++i) {
        values.push_back(&*i);
      }
      std::sort(values.begin(), values.end(), boost::indirect_fun<typename Beam::Value::LessByOverall>());
      for (typename std::vector<typename Beam::Value*>::const_iterator i = values.begin(); i != values.end(); ++i) {
        std::cerr << (*i)->History()->Entry().score;
        for (const decoder::HypHistory *hist = (*i)->History().get(); hist; hist = hist->BestPrevious()) {
          std::cerr << ' ' << hist->Entry().engine << ' ' << hist->Entry().offset;
        }
        std::cerr << '\n';
      }
      std::cerr << '\n';
    }
};
} // namespace output

#endif // _MEMT_Output_StderrBeamDumper_h
