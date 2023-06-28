//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVElectrochemicalPotential2.h"

registerMooseObject("NavierStokesTestApp", INSFVElectrochemicalPotential2);

InputParameters
INSFVElectrochemicalPotential2::validParams()
{
  auto params = FVElementalKernel::validParams();
  params.addClassDescription("Computes residual for diffusion operator for finite volume method.");
  params.addRequiredParam<std::vector<MooseFunctorName>>("c", "concentration");
  params.addRequiredParam<std::vector<int>>("z", "the valence of the cation");
  params.addRequiredParam<MooseFunctorName>("epsilonr", "relative permittivity");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVElectrochemicalPotential2::INSFVElectrochemicalPotential2(const InputParameters & params)
  : FVElementalKernel(params),
  _z(getParam<std::vector<int>>("z")),
    _epsilonr(getFunctor<ADReal>("epsilonr"))
{
  const auto & cs = getParam<std::vector<MooseFunctorName>>("c");
    for (const auto & c_name : cs)
    _c.push_back(&getFunctor<ADReal>(c_name));

  if (_c.size() != _z.size())
    paramError("c", "The lengths of 'c' and 'z' must be the same");
}

ADReal
INSFVElectrochemicalPotential2::computeQpResidual()
{
  const auto state = determineState();
  const auto elem = makeElemArg(_current_elem);
  ADReal epsilonr;
  ADReal sum = 0;
  for (const auto i : index_range(_c))
    sum += _z[i] * (*_c[i])(elem, state);
  
  return (96485.3 / (epsilonr * 8.8542e-12)) * sum;
}
