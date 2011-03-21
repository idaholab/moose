#include "TimeKernel.h"

template<>
InputParameters validParams<TimeKernel>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

TimeKernel::TimeKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters)
{
}
