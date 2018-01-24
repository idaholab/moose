/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
