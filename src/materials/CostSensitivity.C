//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CostSensitivity.h"

registerMooseObject("troutApp", CostSensitivity);

InputParameters
CostSensitivity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes cost sensitivity needed for multimaterial SIMP method.");
  params.addRequiredCoupledVar("design_density", "Design density variable name.");
  params.addRequiredParam<MaterialPropertyName>("cost",
                                                "DerivativeParsedMaterial for cost of materials.");
  return params;
}

CostSensitivity::CostSensitivity(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _sensitivity(declareProperty<Real>(_base_name + "cost_sensitivity")),
    _design_density(coupledValue("design_density")),
    _design_density_name(coupledName("design_density", 0)),
    _dcostdrho(getMaterialPropertyDerivativeByName<Real>(getParam<MaterialPropertyName>("cost"),
                                                         _design_density_name)),
    _cost(getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("cost")))
{
}

void
CostSensitivity::computeQpProperties()
{
  _sensitivity[_qp] = _current_elem->volume() * _cost[_qp] +
                      _current_elem->volume() * _design_density[_qp] * _dcostdrho[_qp];

  // C_e (rho_e) = A_c rho^{1/p} + B_c; A_c = (C_i - C_{i+1})/(rho_i^{1/p} - rho_{i+1}^{1/p}); B_c =
  // C_i - A_c rho_i^{1/p}
}
