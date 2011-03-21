#ifndef CONVERSION_H
#define CONVERSION_H

#include "Moose.h"

namespace Moose {

  template<typename T>
  T stringToEnum(const std::string & s);

  template<>
  TimeSteppingScheme stringToEnum<TimeSteppingScheme>(const std::string & s);

}

#endif //CONVERSION_H
