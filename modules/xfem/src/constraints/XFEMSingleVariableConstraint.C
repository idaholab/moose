/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMSingleVariableConstraint.h"
#include "FEProblem.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<XFEMSingleVariableConstraint>()
{
  InputParameters params = validParams<ElemElemConstraint>();
  params.addParam<Real>("alpha", 100, "Stablization parameter in Nitsche's formulation.");
  params.addParam<Real>("jump", 0, "Jump at the interface.");
  params.addParam<Real>("jump_flux", 0, "Flux jump at the interface.");
  return params;
}

XFEMSingleVariableConstraint::XFEMSingleVariableConstraint(const InputParameters & parameters) :
    ElemElemConstraint(parameters),
    _alpha(getParam<Real>("alpha")),
    _jump(getParam<Real>("jump")),
    _jump_flux(getParam<Real>("jump_flux"))
{
}

XFEMSingleVariableConstraint::~XFEMSingleVariableConstraint()
{
}

void
XFEMSingleVariableConstraint::reinitConstraintQuadrature(const ElementPairInfo & element_pair_info)
{
  _interface_normal = element_pair_info._normal;
  ElemElemConstraint::reinitConstraintQuadrature(element_pair_info);
}

Real
XFEMSingleVariableConstraint::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      r -= (0.5 * _grad_u[_qp] * _interface_normal + 0.5 * _grad_u_neighbor[_qp] * _interface_normal) * _test[_i][_qp];
      r -= (_u[_qp] - _u_neighbor[_qp]) * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r += 0.5 * _grad_test[_i][_qp] * _interface_normal * _jump + 0.5 * _test[_i][_qp] * _jump_flux;
      r += _alpha * (_u[_qp] - _u_neighbor[_qp] - _jump) * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      r += (0.5 * _grad_u[_qp] * _interface_normal + 0.5 * _grad_u_neighbor[_qp] * _interface_normal) * _test_neighbor[_i][_qp];
      r -= (_u[_qp] - _u_neighbor[_qp]) * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r += 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal * _jump + 0.5 * _test_neighbor[_i][_qp] * _jump_flux;
      r -= _alpha * (_u[_qp] - _u_neighbor[_qp] - _jump) * _test_neighbor[_i][_qp];
      break;
  }
  return r;
}

Real
XFEMSingleVariableConstraint::computeQpJacobian(Moose::DGJacobianType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::ElementElement:
      r += -0.5 * _grad_phi[_j][_qp] * _interface_normal * _test[_i][_qp] - _phi[_j][_qp] * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r += _alpha * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      r += -0.5 * _grad_phi_neighbor[_j][_qp] * _interface_normal * _test[_i][_qp] + _phi_neighbor[_j][_qp] * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r -= _alpha * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborElement:
      r += 0.5 * _grad_phi[_j][_qp] * _interface_normal * _test_neighbor[_i][_qp] - _phi[_j][_qp] * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r -= _alpha * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      r += 0.5 * _grad_phi_neighbor[_j][_qp] * _interface_normal * _test_neighbor[_i][_qp] + _phi_neighbor[_j][_qp] * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r += _alpha * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }

  return r;
}
