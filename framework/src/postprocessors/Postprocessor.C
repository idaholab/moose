#include "Postprocessor.h"

#include "MooseSystem.h"

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
{
  // Initialize the postprocessor data for this PP
//  moose_system._postprocessor_data[_tid]._values[name] = 0.0;
  moose_system._postprocessor_data[_tid].init(name);
}

void
Postprocessor::gatherSum(Real & value)
{
  // TODO: Gather threaded values as well
  Parallel::sum(value);
}

void
Postprocessor::gatherSum(int & value)
{
  // TODO: Gather threaded values as well
  Parallel::sum(value);
}




