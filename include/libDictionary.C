#include <vector>
#include <stdint.h>

#ifdef __MAKECINT__
#pragma link C++ class std::vector<float>+;
#pragma link C++ class std::vector<uint8_t>+;
#pragma link C++ class std::vector<uint16_t>+;
#pragma link C++ class std::vector<char>+;
#pragma link C++ class std::vector<std::vector<float> >+;
#pragma link C++ class std::vector<std::vector<double> >+;
#pragma link C++ class std::vector<std::vector<int> >+;
#endif

void libDictionary(){}
