#include "Postprocessor.h"

template<>
InputParameters validParams<Postprocessor>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

