/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVBC.h"

template <>
InputParameters
validParams<CNSFVBC>()
{
  InputParameters params = validParams<IntegratedBC>();

  params.addClassDescription("A boundary condition object for the CNS equations.");

  MooseEnum component("mass x-momentum y-momentum z-momentum total-energy");

  params.addParam<MooseEnum>("component", component, "Choose one of the conservation equations");

  params.addRequiredCoupledVar("rho", "Conserved variable: rho");

  params.addRequiredCoupledVar("rhou", "Conserved variable: rhou");

  params.addCoupledVar("rhov", "Conserved variable: rhov");

  params.addCoupledVar("rhow", "Conserved variable: rhow");

  params.addRequiredCoupledVar("rhoe", "Conserved variable: rhoe");

  params.addRequiredParam<UserObjectName>("flux", "The name of boundary flux object to use");

  return params;
}

CNSFVBC::CNSFVBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _component(getParam<MooseEnum>("component")),
    _rhoc1(coupledValue("rho")),
    _rhouc1(coupledValue("rhou")),
    _rhovc1(isCoupled("rhov") ? coupledValue("rhov") : _zero),
    _rhowc1(isCoupled("rhow") ? coupledValue("rhow") : _zero),
    _rhoec1(coupledValue("rhoe")),
    _rho1(getMaterialProperty<Real>("rho")),
    _rhou1(getMaterialProperty<Real>("rhou")),
    _rhov1(getMaterialProperty<Real>("rhov")),
    _rhow1(getMaterialProperty<Real>("rhow")),
    _rhoe1(getMaterialProperty<Real>("rhoe")),
    _flux(getUserObject<BoundaryFluxBase>("flux")),
    _rho_var(coupled("rho")),
    _rhou_var(coupled("rhou")),
    _rhov_var(isCoupled("rhov") ? coupled("rhov") : zero),
    _rhow_var(isCoupled("rhow") ? coupled("rhow") : zero),
    _rhoe_var(coupled("rhoe"))
{
  _jmap.insert(std::pair<unsigned int, unsigned int>(_rho_var, 0));
  _jmap.insert(std::pair<unsigned int, unsigned int>(_rhou_var, 1));
  _jmap.insert(std::pair<unsigned int, unsigned int>(_rhov_var, 2));
  _jmap.insert(std::pair<unsigned int, unsigned int>(_rhow_var, 3));
  _jmap.insert(std::pair<unsigned int, unsigned int>(_rhoe_var, 4));
}

Real
CNSFVBC::computeQpResidual()
{
  /// the size of flux vector is five
  /// 0 for mass
  /// 1 for x-momentum
  /// 2 for y-momentum
  /// 3 for z-momentum
  /// 4 for total-energy

  std::vector<Real> uvec1 = {_rho1[_qp], _rhou1[_qp], _rhov1[_qp], _rhow1[_qp], _rhoe1[_qp]};

  /// input variables are the reconstructed linear monomial extrapolated at side center from "left" and "ghost" cells

  const std::vector<Real> & flux =
      _flux.getFlux(_current_side, _current_elem->id(), uvec1, _normals[_qp], _tid);

  return flux[_component] * _test[_i][_qp];
}

Real
CNSFVBC::computeQpJacobian()
{
  std::vector<Real> uvec1 = {_rhoc1[_qp], _rhouc1[_qp], _rhovc1[_qp], _rhowc1[_qp], _rhoec1[_qp]};

  /// input variables are the original constant monomial from the "left" and "right" cells

  const DenseMatrix<Real> & fjac1 =
      _flux.getJacobian(_current_side, _current_elem->id(), uvec1, _normals[_qp], _tid);

  return fjac1(_component, _component) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
CNSFVBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  std::vector<Real> uvec1 = {_rhoc1[_qp], _rhouc1[_qp], _rhovc1[_qp], _rhowc1[_qp], _rhoec1[_qp]};

  /// input variables are the original constant monomial from the "left" and "right" cells

  const DenseMatrix<Real> & fjac1 =
      _flux.getJacobian(_current_side, _current_elem->id(), uvec1, _normals[_qp], _tid);

  return fjac1(_component, _jmap[jvar]) * _phi[_j][_qp] * _test[_i][_qp];
}
