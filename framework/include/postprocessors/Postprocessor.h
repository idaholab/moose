#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

#include "Kernel.h"

//Forward Declarations
class Postprocessor;

template<>
InputParameters validParams<Postprocessor>();

class Postprocessor
{
  virtual void execute() = 0;  
};
 
#endif
