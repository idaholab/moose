//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialPropertyBlockAux.h"

#include "Assembly.h"

registerMooseObject("MooseTestApp", MaterialPropertyBlockAux);

InputParameters
MaterialPropertyBlockAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addParam<MaterialPropertyName>("mat_prop_name",
                                        "The material property to check where it is defined");
  return params;
}

MaterialPropertyBlockAux::MaterialPropertyBlockAux(const InputParameters & params)
  : AuxKernel(params),
    _mat_prop_name(getParam<MaterialPropertyName>("mat_prop_name")),
    _has_prop(false)
{
  if (isNodal())
    mooseError("This AuxKernel only supports Elemental fields");
}

void
MaterialPropertyBlockAux::subdomainSetup()
{
  _has_prop = _subproblem.hasBlockMaterialProperty(_assembly.currentSubdomainID(), _mat_prop_name);
}

Real
MaterialPropertyBlockAux::computeValue()
{
  if (_has_prop)
    return 1;
  else
    return 0;
}
