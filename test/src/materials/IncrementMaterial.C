//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncrementMaterial.h"

registerMooseObject("MooseTestApp", IncrementMaterial);

InputParameters
IncrementMaterial::validParams()
{
  InputParameters params = GenericConstantMaterial::validParams();
  params.addClassDescription(
      "Material that tracks the number of times computeQpProperties has been called.");
  params.set<MooseEnum>("constant_on") = "NONE";
  return params;
}

IncrementMaterial::IncrementMaterial(const InputParameters & parameters)
  : GenericConstantMaterial(parameters), _inc(0), _mat_prop(declareProperty<Real>("mat_prop"))
{
}

void
IncrementMaterial::computeQpProperties()
{
  GenericConstantMaterial::computeQpProperties();
  _inc++;
  _mat_prop[_qp] = _inc;
}
