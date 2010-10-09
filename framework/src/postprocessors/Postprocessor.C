/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

Postprocessor::Postprocessor(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :_local_name(name),
   _local_tid(parameters.get<THREAD_ID>("_tid"))
{
  // Initialize the postprocessor data for this PP
  moose_system._postprocessor_data[_local_tid].init(name);
}

const std::string &
Postprocessor::name()
{
  return _local_name;
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
