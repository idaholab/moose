#include "ElementPostprocessor.h"

template<>
InputParameters validParams<ElementPostprocessor>()
{
  InputParameters params = validParams<Kernel>();
  params += validParams<Postprocessor>();
  return params;
}

ElementPostprocessor::ElementPostprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   Postprocessor(name, moose_system, parameters)
{}
