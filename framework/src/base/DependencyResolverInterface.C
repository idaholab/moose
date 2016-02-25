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

#include "DependencyResolverInterface.h"

template<>
InputParameters validParams<DependencyResolverInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<bool>("allow_cyclic_dependency", false, "Allows for cyclic dependencies to exists for this object (object is also not sorted).");
  return params;
}


DependencyResolverInterface::DependencyResolverInterface(const InputParameters & parameters) :
    _allow_cyclic(parameters.get<bool>("allow_cyclic_dependency"))
{
}
