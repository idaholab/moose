#include "Postprocessor.h"

// libMesh includes
#include "parallel.h"

template<>
InputParameters validParams<Postprocessor>()
{
  InputParameters params = validParams<MooseObject>();
  return params;
}


Postprocessor::Postprocessor(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :MooseObject(name, moose_system, parameters)
{}

void
Postprocessor::gatherSum(Real value)
{
  // TODO: Gather threaded values as well
  Parallel::sum(value);
}




