//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBEvolution.h"

template <>
InputParameters
validParams<GBEvolution>()
{
  InputParameters params = validParams<GBEvolutionBase>();
  params.addRequiredParam<Real>("GBenergy", "Grain boundary energy in J/m^2");
  return params;
}

GBEvolution::GBEvolution(const InputParameters & parameters)
  : GBEvolutionBase(parameters), _GBEnergy(getParam<Real>("GBenergy"))
{
}

void
GBEvolution::computeQpProperties()
{
  // eV/nm^2
  _sigma[_qp] = _GBEnergy * _JtoeV * (_length_scale * _length_scale);

  GBEvolutionBase::computeQpProperties();
}
