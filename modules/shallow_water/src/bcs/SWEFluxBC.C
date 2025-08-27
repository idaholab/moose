//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEFluxBC.h"

registerMooseObject("ShallowWaterApp", SWEFluxBC);

InputParameters
SWEFluxBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Boundary flux BC for SWE using BoundaryFluxBase userobjects.");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredCoupledVar("hu", "Conserved variable: h*u");
  params.addRequiredCoupledVar("hv", "Conserved variable: h*v");
  params.addRequiredParam<UserObjectName>("boundary_flux", "Name of boundary flux user object");
  params.addCoupledVar("b_var",
                       "Cell-constant bathymetry variable (MONOMIAL/CONSTANT) to pass to the "
                       "boundary flux UO (optional)");
  return params;
}

SWEFluxBC::SWEFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _h1(getMaterialProperty<Real>("h")),
    _hu1(getMaterialProperty<Real>("hu")),
    _hv1(getMaterialProperty<Real>("hv")),
    _h(coupledValue("h")),
    _hu(coupledValue("hu")),
    _hv(coupledValue("hv")),
    _h_var(coupled("h")),
    _hu_var(coupled("hu")),
    _hv_var(coupled("hv")),
    _jmap({{_h_var, 0}, {_hu_var, 1}, {_hv_var, 2}}),
    _equation_index(_jmap.at(_var.number())),
    _flux(getUserObject<BoundaryFluxBase>("boundary_flux")),
    _has_b(isCoupled("b_var")),
    _b_var_val(_has_b ? &coupledValue("b_var") : nullptr)
{
}

Real
SWEFluxBC::computeQpResidual()
{
  std::vector<Real> U = {_h1[_qp], _hu1[_qp], _hv1[_qp]};
  if (_has_b)
    U.push_back((*_b_var_val)[_qp]);
  const auto & F = _flux.getFlux(_current_side, _current_elem->id(), U, _normals[_qp]);
  return F[_equation_index] * _test[_i][_qp];
}

Real
SWEFluxBC::computeQpJacobian()
{
  return computeQpOffDiagJacobian(_var.number());
}

Real
SWEFluxBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  std::vector<Real> U = {_h[_qp], _hu[_qp], _hv[_qp]};
  if (_has_b)
    U.push_back((*_b_var_val)[_qp]);
  const auto & J = _flux.getJacobian(_current_side, _current_elem->id(), U, _normals[_qp]);
  return J(_equation_index, _jmap.at(jvar)) * _phi[_j][_qp] * _test[_i][_qp];
}
