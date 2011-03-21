#include "ProcessorIDAux.h"

template<>
InputParameters validParams<ProcessorIDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

ProcessorIDAux::ProcessorIDAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters)
{
  if (isNodal())
    mooseError("ProcessorIDAux can only be used with an elemental Aux variable!");
}


Real
ProcessorIDAux::computeValue()
{
//  return _moose_system._element_weights[_current_elem->id()];
  
  return _current_elem->processor_id();
}
