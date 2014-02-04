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

template<>
InputParameters validParams<BadStatefulMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<bool>("declare_only_older", false, "Whether or not to declare the old or older property");
  return params;
}

BadStatefulMaterial::BadStatefulMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _prop_old(getParam<bool>("declare_only_older") ? declarePropertyOlder<Real>("property") : declarePropertyOld<Real>("property"))
{}

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
