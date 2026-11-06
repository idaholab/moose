//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConstantArrayDiffusivity.h"

registerKokkosMaterial("MooseTestApp", KokkosConstantArrayDiffusivity);

InputParameters
KokkosConstantArrayDiffusivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "The name of the diffusivity");
  params.addRequiredParam<unsigned int>("components", "The number of array variable components");
  params.addParam<bool>("matrix",
                        false,
                        "Whether to declare a two-dimensional (full matrix) diffusivity instead "
                        "of a one-dimensional (diagonal) diffusivity");
  params.addRequiredParam<std::vector<Real>>(
      "values",
      "The diffusivity values: one per component for a one-dimensional diffusivity, or the matrix "
      "entries in row-major order for a two-dimensional diffusivity");
  params.addClassDescription(
      "Declares a constant one-dimensional or two-dimensional diffusivity of an array variable.");
  return params;
}

KokkosConstantArrayDiffusivity::KokkosConstantArrayDiffusivity(const InputParameters & parameters)
  : Material(parameters),
    _n(getParam<unsigned int>("components")),
    _matrix(getParam<bool>("matrix"))
{
  const auto & values = getParam<std::vector<Real>>("values");
  const auto expected = _matrix ? _n * _n : _n;

  if (values.size() != expected)
    paramError("values", "Expected ", expected, " values but got ", values.size(), ".");

  _values = values;

  if (_matrix)
    _diffusivity_2d = declareKokkosProperty<Real, 2>("prop_name", {_n, _n});
  else
    _diffusivity_1d = declareKokkosProperty<Real, 1>("prop_name", {_n});
}
