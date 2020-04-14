//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBEvolution.h"

registerMooseObject("PhaseFieldApp", GBEvolution);
registerMooseObject("PhaseFieldApp", ADGBEvolution);

template <bool is_ad>
InputParameters
GBEvolutionTempl<is_ad>::validParams()
{
  InputParameters params = GBEvolutionBaseTempl<is_ad>::validParams();
  params.addRequiredParam<Real>("GBenergy", "Grain boundary energy in J/m^2");
  return params;
}

template <bool is_ad>
GBEvolutionTempl<is_ad>::GBEvolutionTempl(const InputParameters & parameters)
  : GBEvolutionBaseTempl<is_ad>(parameters), _GBEnergy(this->template getParam<Real>("GBenergy"))
{
}

template <bool is_ad>
void
GBEvolutionTempl<is_ad>::computeQpProperties()
{
  // eV/nm^2
  this->_sigma[this->_qp] = _GBEnergy * this->_JtoeV * (this->_length_scale * this->_length_scale);

  GBEvolutionBaseTempl<is_ad>::computeQpProperties();
}

template class GBEvolutionTempl<false>;
template class GBEvolutionTempl<true>;
