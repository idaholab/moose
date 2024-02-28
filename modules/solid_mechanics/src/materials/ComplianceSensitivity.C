//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComplianceSensitivity.h"

registerMooseObject("SolidMechanicsApp", ComplianceSensitivity);

InputParameters
ComplianceSensitivity::validParams()
{
  InputParameters params = StrainEnergyDensity::validParams();
  params.addClassDescription("Computes compliance sensitivity needed for SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredParam<MaterialPropertyName>("youngs_modulus",
                                                "DerivativeParsedMaterial for Youngs modulus.");

  return params;
}

ComplianceSensitivity::ComplianceSensitivity(const InputParameters & parameters)
  : StrainEnergyDensity(parameters),
    _sensitivity(declareProperty<Real>(_base_name + "sensitivity")),
    _design_density(coupledValue("design_density")),
    _design_density_name(coupledName("design_density", 0)),
    _dEdp(getMaterialPropertyDerivativeByName<Real>(
        getParam<MaterialPropertyName>("youngs_modulus"), _design_density_name)),
    _youngs_modulus(getMaterialProperty<Real>(getParam<MaterialPropertyName>("youngs_modulus")))
{
}

void
ComplianceSensitivity::computeQpProperties()
{
  // Call the parent class's method to compute the strain energy density
  StrainEnergyDensity::computeQpProperties();
  _sensitivity[_qp] = -_dEdp[_qp] * _strain_energy_density[_qp] / _youngs_modulus[_qp];
}
