//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEHydrostaticCorrectionBC.h"

registerMooseObject("ShallowWaterApp", SWEHydrostaticCorrectionBC);

InputParameters
SWEHydrostaticCorrectionBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Boundary hydrostatic correction for SWE momentum (cancels wall pressure contribution).");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addParam<Real>("gravity", 9.81, "Gravitational acceleration g");
  return params;
}

SWEHydrostaticCorrectionBC::SWEHydrostaticCorrectionBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _h(getMaterialProperty<Real>("h")),
    _h_var(coupled("h")),
    _g(getParam<Real>("gravity"))
{
}

Real
SWEHydrostaticCorrectionBC::computeQpResidual()
{
  // Only apply to momentum components; for h equation return zero
  // Determine normal component based on variable: assumes variables named 'hu' or 'hv'
  const Real nx = _normals[_qp](0);
  const Real ny = _normals[_qp](1);
  const bool is_hu = (_var.name().find("hu") != std::string::npos);
  const bool is_hv = (_var.name().find("hv") != std::string::npos);
  if (!is_hu && !is_hv)
    return 0.0;

  const Real ncomp = is_hu ? nx : ny;
  const Real h = std::max(_h[_qp], 0.0);
  // Residual contribution: -0.5 * g * h^2 * n_component
  return (-0.5 * _g * h * h * ncomp) * _test[_i][_qp];
}

Real
SWEHydrostaticCorrectionBC::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
SWEHydrostaticCorrectionBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  const Real nx = _normals[_qp](0);
  const Real ny = _normals[_qp](1);
  const bool is_hu = (_var.name().find("hu") != std::string::npos);
  const bool is_hv = (_var.name().find("hv") != std::string::npos);
  if (!is_hu && !is_hv)
    return 0.0;

  if (jvar == _h_var)
  {
    const Real ncomp = is_hu ? nx : ny;
    const Real h = std::max(_h[_qp], 0.0);
    // d/dh (-0.5 * g * h^2) = -g * h
    return (-_g * h * ncomp) * _phi[_j][_qp] * _test[_i][_qp];
  }
  return 0.0;
}
