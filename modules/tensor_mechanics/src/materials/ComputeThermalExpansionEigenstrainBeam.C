//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeThermalExpansionEigenstrainBeam.h"

registerMooseObject("TensorMechanicsApp", ComputeThermalExpansionEigenstrainBeam);

InputParameters
ComputeThermalExpansionEigenstrainBeam::validParams()
{
  InputParameters params = ComputeThermalExpansionEigenstrainBeamBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion "
                             "with a constant coefficient");
  params.addParam<Real>("thermal_expansion_coeff", "Thermal expansion coefficient");

  return params;
}

ComputeThermalExpansionEigenstrainBeam::ComputeThermalExpansionEigenstrainBeam(
    const InputParameters & parameters)
  : ComputeThermalExpansionEigenstrainBeamBase(parameters),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff"))
{
}

Real
ComputeThermalExpansionEigenstrainBeam::computeThermalStrain()
{
  return _thermal_expansion_coeff * (_temperature[_qp] - _stress_free_temperature[_qp]);
}
