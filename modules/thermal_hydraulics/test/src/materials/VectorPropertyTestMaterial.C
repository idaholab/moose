//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPropertyTestMaterial.h"

registerMooseObject("ThermalHydraulicsTestApp", VectorPropertyTestMaterial);

InputParameters
VectorPropertyTestMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Test material with vector properties");
  return params;
}

VectorPropertyTestMaterial::VectorPropertyTestMaterial(const InputParameters & parameters)
  : Material(parameters), _vec(declareProperty<std::vector<Real>>("test_property"))
{
}

void
VectorPropertyTestMaterial::computeQpProperties()
{
  _vec[_qp].resize(5);
  for (std::size_t i = 0; i < _vec[_qp].size(); i++)
    _vec[_qp][i] = i * 2.;
}
