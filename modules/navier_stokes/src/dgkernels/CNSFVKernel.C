/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVKernel.h"

template <>
InputParameters
validParams<CNSFVKernel>()
{
  InputParameters params = validParams<DGKernel>();

  params.addClassDescription("A DGKernel for the CNS equations.");

  MooseEnum component("mass x-momentum y-momentum z-momentum total-energy");

  params.addParam<MooseEnum>("component", component, "Choose one of the conservation equations");

  params.addRequiredCoupledVar("rho", "Conserved variable: rho");

  params.addRequiredCoupledVar("rhou", "Conserved variable: rhou");

  params.addCoupledVar("rhov", "Conserved variable: rhov");

  params.addCoupledVar("rhow", "Conserved variable: rhow");

  params.addRequiredCoupledVar("rhoe", "Conserved variable: rhoe");

  params.addRequiredParam<UserObjectName>("flux", "The name of internal side flux object to use");

  return params;
}

CNSFVKernel::CNSFVKernel(const InputParameters & parameters)
  : DGKernel(parameters),
    _component(getParam<MooseEnum>("component")),
    _rhoc1(coupledValue("rho")),
    _rhouc1(coupledValue("rhou")),
    _rhovc1(isCoupled("rhov") ? coupledValue("rhov") : _zero),
    _rhowc1(isCoupled("rhow") ? coupledValue("rhow") : _zero),
    _rhoec1(coupledValue("rhoe")),
    _rhoc2(coupledNeighborValue("rho")),
    _rhouc2(coupledNeighborValue("rhou")),
    _rhovc2(isCoupled("rhov") ? coupledNeighborValue("rhov") : _zero),
    _rhowc2(isCoupled("rhow") ? coupledNeighborValue("rhow") : _zero),
    _rhoec2(coupledNeighborValue("rhoe")),
    _rho1(getMaterialProperty<Real>("rho")),
    _rhou1(getMaterialProperty<Real>("rhou")),
    _rhov1(getMaterialProperty<Real>("rhov")),
    _rhow1(getMaterialProperty<Real>("rhow")),
    _rhoe1(getMaterialProperty<Real>("rhoe")),
    _rho2(getNeighborMaterialProperty<Real>("rho")),
    _rhou2(getNeighborMaterialProperty<Real>("rhou")),
    _rhov2(getNeighborMaterialProperty<Real>("rhov")),
    _rhow2(getNeighborMaterialProperty<Real>("rhow")),
    _rhoe2(getNeighborMaterialProperty<Real>("rhoe")),
    _flux(getUserObject<InternalSideFluxBase>("flux")),
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

CNSFVKernel::~CNSFVKernel() {}

Real
CNSFVKernel::computeQpResidual(Moose::DGResidualType type)
{
  /// the size of flux vector is five
  /// 0 for mass
  /// 1 for x-momentum
  /// 2 for y-momentum
  /// 3 for z-momentum
  /// 4 for total-energy

  std::vector<Real> uvec1 = {_rho1[_qp], _rhou1[_qp], _rhov1[_qp], _rhow1[_qp], _rhoe1[_qp]};

  std::vector<Real> uvec2 = {_rho2[_qp], _rhou2[_qp], _rhov2[_qp], _rhow2[_qp], _rhoe2[_qp]};

  /// input variables are the reconstructed linear monomial extrapolated at side center from "left" and "right" cells
  const std::vector<Real> & flux = _flux.getFlux(
      _current_side, _current_elem->id(), _neighbor_elem->id(), uvec1, uvec2, _normals[_qp], _tid);

  Real re = 0.;
  switch (type)
  {
    case Moose::Element:
      re = flux[_component] * _test[_i][_qp];
      break;
    case Moose::Neighbor:
      re = -flux[_component] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

Real
CNSFVKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  /// input variables are the original constant monomial from the "left" and "right" cells

  std::vector<Real> uvec1 = {_rhoc1[_qp], _rhouc1[_qp], _rhovc1[_qp], _rhowc1[_qp], _rhoec1[_qp]};

  std::vector<Real> uvec2 = {_rhoc2[_qp], _rhouc2[_qp], _rhovc2[_qp], _rhowc2[_qp], _rhoec2[_qp]};

  const DenseMatrix<Real> & fjac1 = _flux.getJacobian(Moose::Element,
                                                      _current_side,
                                                      _current_elem->id(),
                                                      _neighbor_elem->id(),
                                                      uvec1,
                                                      uvec2,
                                                      _normals[_qp],
                                                      _tid);

  const DenseMatrix<Real> & fjac2 = _flux.getJacobian(Moose::Neighbor,
                                                      _current_side,
                                                      _current_elem->id(),
                                                      _neighbor_elem->id(),
                                                      uvec1,
                                                      uvec2,
                                                      _normals[_qp],
                                                      _tid);

  Real re = 0.;
  switch (type)
  {
    case Moose::ElementElement:
      re = fjac1(_component, _component) * _phi[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::ElementNeighbor:
      re = fjac2(_component, _component) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::NeighborElement:
      re = -fjac1(_component, _component) * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;
    case Moose::NeighborNeighbor:
      re = -fjac2(_component, _component) * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

Real
CNSFVKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  /// input variables are the original constant monomial from the "left" and "right" cells

  std::vector<Real> uvec1 = {_rhoc1[_qp], _rhouc1[_qp], _rhovc1[_qp], _rhowc1[_qp], _rhoec1[_qp]};

  std::vector<Real> uvec2 = {_rhoc2[_qp], _rhouc2[_qp], _rhovc2[_qp], _rhowc2[_qp], _rhoec2[_qp]};

  const DenseMatrix<Real> & fjac1 = _flux.getJacobian(Moose::Element,
                                                      _current_side,
                                                      _current_elem->id(),
                                                      _neighbor_elem->id(),
                                                      uvec1,
                                                      uvec2,
                                                      _normals[_qp],
                                                      _tid);

  const DenseMatrix<Real> & fjac2 = _flux.getJacobian(Moose::Neighbor,
                                                      _current_side,
                                                      _current_elem->id(),
                                                      _neighbor_elem->id(),
                                                      uvec1,
                                                      uvec2,
                                                      _normals[_qp],
                                                      _tid);

  Real re = 0.;
  switch (type)
  {
    case Moose::ElementElement:
      re = fjac1(_component, _jmap[jvar]) * _phi[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::ElementNeighbor:
      re = fjac2(_component, _jmap[jvar]) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::NeighborElement:
      re = -fjac1(_component, _jmap[jvar]) * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;
    case Moose::NeighborNeighbor:
      re = -fjac2(_component, _jmap[jvar]) * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}
