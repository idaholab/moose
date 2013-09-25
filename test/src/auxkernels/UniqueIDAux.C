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

#include "UniqueIDAux.h"

template<>
InputParameters validParams<UniqueIDAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

UniqueIDAux::UniqueIDAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters)
{
}

UniqueIDAux::~UniqueIDAux()
{
}

Real
UniqueIDAux::computeValue()
{
#ifdef LIBMESH_ENABLE_UNIQUE_ID
  if (isNodal())
    return _current_node->unique_id();
  else
    return _current_elem->unique_id();
#endif
  return 0;
}

