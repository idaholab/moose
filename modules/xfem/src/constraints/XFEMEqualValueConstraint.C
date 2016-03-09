/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "XFEMEqualValueConstraint.h"
#include "FEProblem.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<XFEMEqualValueConstraint>()
{
  InputParameters params = validParams<ElemElemConstraint>();
  params.addParam<Real>("alpha", 100, "Stablization parameter in Nitsche's formulation.");
  params.addParam<Real>("jump", 0, "Jump at the interface.");
  params.addParam<Real>("jump_flux", 0, "Flux jump at the interface.");
  return params;
}

XFEMEqualValueConstraint::XFEMEqualValueConstraint(const InputParameters & parameters) :
    ElemElemConstraint(parameters),
    _alpha(getParam<Real>("alpha")),
    _jump(getParam<Real>("jump")),
    _jump_flux(getParam<Real>("jump_flux"))
{
}

XFEMEqualValueConstraint::~XFEMEqualValueConstraint()
{
}

void
XFEMEqualValueConstraint::setqRuleNormal(ElementPairInfo & element_pair_info)
{ 
  _interface_q_point.resize(element_pair_info._q_point.size());
  _interface_JxW.resize(element_pair_info._JxW.size());
  std::copy(element_pair_info._q_point.begin(), element_pair_info._q_point.end(), _interface_q_point.begin());
  std::copy(element_pair_info._JxW.begin(), element_pair_info._JxW.end(), _interface_JxW.begin());
  _interface_normal = element_pair_info._normal;
} 

Real 
XFEMEqualValueConstraint::computeQpResidual(Moose::DGResidualType type)
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
XFEMEqualValueConstraint::computeQpJacobian(Moose::DGJacobianType type)
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
