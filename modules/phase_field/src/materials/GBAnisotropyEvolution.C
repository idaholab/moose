//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GBAnisotropyEvolution.h"

registerMooseObject("PhaseFieldApp", GBAnisotropyEvolution);

InputParameters
GBAnisotropyEvolution::validParams()
{
  InputParameters params = GBAnisotropyEvolutionBase::validParams();
  params.addRequiredParam<Real>("GBenergy", "Grain boundary energy in J/m^2");
  return params;
}


GBAnisotropyEvolution::GBAnisotropyEvolution(const InputParameters & parameters)
  : GBAnisotropyEvolutionBase(parameters), _GBEnergy(getParam<Real>("GBenergy"))
{
}

void
GBAnisotropyEvolution::computeQpProperties()
{
  // eV/nm^2
  this->_sigma[this->_qp] = _GBEnergy * this->_JtoeV * (this->_length_scale * this->_length_scale);

  GBAnisotropyEvolutionBase::computeQpProperties();
}
