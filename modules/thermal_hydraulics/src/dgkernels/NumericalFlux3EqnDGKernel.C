//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFlux3EqnDGKernel.h"
#include "NumericalFlux3EqnBase.h"
#include "MooseVariable.h"
#include "THMIndicesVACE.h"

registerMooseObject("ThermalHydraulicsApp", NumericalFlux3EqnDGKernel);

InputParameters
NumericalFlux3EqnDGKernel::validParams()
{
  InputParameters params = DGKernel::validParams();

  params.addClassDescription(
      "Adds side fluxes for the 1-D, 1-phase, variable-area Euler equations");

  params.addRequiredCoupledVar("A_linear", "Cross-sectional area, linear");
  params.addRequiredCoupledVar("rhoA", "Conserved variable: rho*A");
  params.addRequiredCoupledVar("rhouA", "Conserved variable: rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "Conserved variable: rho*E*A");

  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");

  return params;
}

NumericalFlux3EqnDGKernel::NumericalFlux3EqnDGKernel(const InputParameters & parameters)
  : DGKernel(parameters),

    _A_elem(coupledValue("A_linear")),
    _A_neig(coupledNeighborValue("A_linear")),
    _rhoA1(getMaterialProperty<Real>("rhoA")),
    _rhouA1(getMaterialProperty<Real>("rhouA")),
    _rhoEA1(getMaterialProperty<Real>("rhoEA")),
    _p1(getMaterialProperty<Real>("p")),
    _rhoA2(getNeighborMaterialProperty<Real>("rhoA")),
    _rhouA2(getNeighborMaterialProperty<Real>("rhouA")),
    _rhoEA2(getNeighborMaterialProperty<Real>("rhoEA")),
    _p2(getNeighborMaterialProperty<Real>("p")),
    _numerical_flux(getUserObject<NumericalFlux3EqnBase>("numerical_flux")),
    _rhoA_var(coupled("rhoA")),
    _rhouA_var(coupled("rhouA")),
    _rhoEA_var(coupled("rhoEA")),
    _jmap(getIndexMapping()),
    _equation_index(_jmap.at(_var.number()))
{
}

Real
NumericalFlux3EqnDGKernel::computeQpResidual(Moose::DGResidualType type)
{
  // construct the left and right solution vectors from the reconstructed solution
  std::vector<Real> U1 = {_rhoA1[_qp], _rhouA1[_qp], _rhoEA1[_qp], _A_elem[_qp]};
  std::vector<Real> U2 = {_rhoA2[_qp], _rhouA2[_qp], _rhoEA2[_qp], _A_neig[_qp]};

  const Real nLR_dot_d = _current_side * 2 - 1.0;

  const std::vector<Real> & flux_elem =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), true, U1, U2, nLR_dot_d);
  const std::vector<Real> & flux_neig =
      _numerical_flux.getFlux(_current_side, _current_elem->id(), false, U1, U2, nLR_dot_d);

  Real re = 0.0;
  switch (type)
  {
    case Moose::Element:
      re = flux_elem[_equation_index] * _test[_i][_qp];
      break;
    case Moose::Neighbor:
      re = -flux_neig[_equation_index] * _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

Real
NumericalFlux3EqnDGKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  return computeQpOffDiagJacobian(type, _var.number());
}

Real
NumericalFlux3EqnDGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  // construct the left and right solution vectors from the cell-average solution
  std::vector<Real> U1 = {_rhoA1[_qp], _rhouA1[_qp], _rhoEA1[_qp], _A_elem[_qp]};
  std::vector<Real> U2 = {_rhoA2[_qp], _rhouA2[_qp], _rhoEA2[_qp], _A_neig[_qp]};

  // Temporary hack to allow libMesh fix. After the fix, this should be changed to:
  // const Real nLR_dot_d = _normals[_qp] * _direction[_qp];
  const Real nLR_dot_d = _current_side * 2 - 1.0;

  const DenseMatrix<Real> & dF1_dU1 = _numerical_flux.getJacobian(
      true, true, _current_side, _current_elem->id(), U1, U2, nLR_dot_d);
  const DenseMatrix<Real> & dF1_dU2 = _numerical_flux.getJacobian(
      true, false, _current_side, _current_elem->id(), U1, U2, nLR_dot_d);
  const DenseMatrix<Real> & dF2_dU1 = _numerical_flux.getJacobian(
      false, true, _current_side, _current_elem->id(), U1, U2, nLR_dot_d);
  const DenseMatrix<Real> & dF2_dU2 = _numerical_flux.getJacobian(
      false, false, _current_side, _current_elem->id(), U1, U2, nLR_dot_d);

  Real re = 0.0;
  switch (type)
  {
    case Moose::ElementElement:
      re = dF1_dU1(_equation_index, _jmap.at(jvar)) * _phi[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::ElementNeighbor:
      re = dF1_dU2(_equation_index, _jmap.at(jvar)) * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;
    case Moose::NeighborElement:
      re = -dF2_dU1(_equation_index, _jmap.at(jvar)) * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;
    case Moose::NeighborNeighbor:
      re = -dF2_dU2(_equation_index, _jmap.at(jvar)) * _phi_neighbor[_j][_qp] *
           _test_neighbor[_i][_qp];
      break;
  }
  return re;
}

std::map<unsigned int, unsigned int>
NumericalFlux3EqnDGKernel::getIndexMapping() const
{
  std::map<unsigned int, unsigned int> jmap;
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoA_var, THMVACE1D::MASS));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhouA_var, THMVACE1D::MOMENTUM));
  jmap.insert(std::pair<unsigned int, unsigned int>(_rhoEA_var, THMVACE1D::ENERGY));

  return jmap;
}
