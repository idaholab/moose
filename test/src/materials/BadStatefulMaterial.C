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
  InputParameters params = validParams<Material>();
  params.addParam<bool>("get_older", false, "true to retrieve older property instead of old");
  return params;
}

BadStatefulMaterial::BadStatefulMaterial(const InputParameters & parameters) : Material(parameters)
{
  if (getParam<bool>("get_older"))
    getMaterialPropertyOlder<Real>("nonexistingpropertyname");
  else
    getMaterialPropertyOld<Real>("nonexistingpropertyname");
}

void
BadStatefulMaterial::initQpStatefulProperties()
{
}

void
BadStatefulMaterial::computeQpProperties()
{
}
