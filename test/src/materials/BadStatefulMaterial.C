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

#include "BadStatefulMaterial.h"

template <>
InputParameters
validParams<BadStatefulMaterial>()
{
  return validParams<Material>();
}

BadStatefulMaterial::BadStatefulMaterial(const InputParameters & parameters) : Material(parameters)
{
}

void
BadStatefulMaterial::initQpStatefulProperties()
{
  // Bad Material
}

void
BadStatefulMaterial::computeQpProperties()
{
  // Bad Material
}
