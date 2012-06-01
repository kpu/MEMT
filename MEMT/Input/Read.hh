#ifndef _MEMT_Input_Read_h
#define _MEMT_Input_Read_h

#include <istream>
#include <string>

namespace input {

class Engine;

void ReadEngine(const Config &config, const std::string &line, unsigned int num_engines, Engine &engine);

void ReadAllEngines(const Config &config, std::istream &in, Input &input);

void AddSelfAlignments(Input &input);
void AddBoundaryAlignments(Input &input);

} // namespace input

#endif // _MEMT_Input_Read_h
