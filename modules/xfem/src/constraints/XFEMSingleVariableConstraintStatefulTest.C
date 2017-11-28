/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMSingleVariableConstraintStatefulTest.h"

// MOOSE includes
#include "Assembly.h"
#include "ElementPairInfo.h"
#include "FEProblem.h"

#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<XFEMSingleVariableConstraintStatefulTest>()
{
  InputParameters params = validParams<XFEMMaterialManagerConstraint>();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same block");
  params.addParam<Real>("alpha", 100, "Stablization parameter in Nitsche's formulation.");
  params.addParam<Real>("jump", 0, "Jump at the interface.");
  params.addParam<Real>("jump_flux", 0, "Flux jump at the interface.");
  return params;
}

XFEMSingleVariableConstraintStatefulTest::XFEMSingleVariableConstraintStatefulTest(
    const InputParameters & parameters)
  : XFEMMaterialManagerConstraint(parameters),
    _alpha(getParam<Real>("alpha")),
    _jump(getParam<Real>("jump")),
    _jump_flux(getParam<Real>("jump_flux")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "")
{
}

XFEMSingleVariableConstraintStatefulTest::~XFEMSingleVariableConstraintStatefulTest() {}

void
XFEMSingleVariableConstraintStatefulTest::initialSetup()
{
  _prop_jump = getMaterialProperty<Real>(_base_name + "jump");
  _prop_jump_old = getMaterialPropertyOld<Real>(_base_name + "jump");
}

void
XFEMSingleVariableConstraintStatefulTest::reinitConstraintQuadrature(
    const ElementPairInfo & element_pair_info)
{
  _interface_normal = element_pair_info._elem1_normal;
  XFEMMaterialManagerConstraint::reinitConstraintQuadrature(element_pair_info);
}

Real
XFEMSingleVariableConstraintStatefulTest::computeQpResidual(Moose::DGResidualType type)
{
  // std::cout << "elem id = " << _current_elem->id() << ", neighbor id = " << _neighbor_elem->id()
  //           << " jump = " << (*_prop_jump)[_qp] << ", jump_old = " << (*_prop_jump_old)[_qp]
  //           << std::endl;

  std::cout << "elem id = " << _current_elem->id() << ", point = " << (_current_elem->point(0))(1)
            << ", jump = " << (*_prop_jump)[_qp] << ", jump_old = " << (*_prop_jump_old)[_qp]
            << std::endl;

  Real r = 0;

  // switch (type)
  // {
  //   case Moose::Element:
  //     r -= (0.5 * _grad_u[_qp] * _interface_normal +
  //           0.5 * _grad_u_neighbor[_qp] * _interface_normal) *
  //          _test[_i][_qp];
  //     r -= ((*_prop_jump)[_qp]) * 0.5 * _grad_test[_i][_qp] * _interface_normal;
  //     r +=
  //         0.5 * _grad_test[_i][_qp] * _interface_normal * _jump + 0.5 * _test[_i][_qp] *
  //         _jump_flux;
  //     r += _alpha * ((*_prop_jump)[_qp] - _jump) * _test[_i][_qp];
  //     break;
  //
  //   case Moose::Neighbor:
  //     r += (0.5 * _grad_u[_qp] * _interface_normal +
  //           0.5 * _grad_u_neighbor[_qp] * _interface_normal) *
  //          _test_neighbor[_i][_qp];
  //     r -= ((*_prop_jump)[_qp]) * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
  //     r += 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal * _jump +
  //          0.5 * _test_neighbor[_i][_qp] * _jump_flux;
  //     r -= _alpha * ((*_prop_jump)[_qp] - _jump) * _test_neighbor[_i][_qp];
  //     break;
  // }
  switch (type)
  {
    case Moose::Element:
      r -= (0.5 * _grad_u[_qp] * _interface_normal +
            0.5 * _grad_u_neighbor[_qp] * _interface_normal) *
           _test[_i][_qp];
      r -= (_u[_qp] - _u_neighbor[_qp]) * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r += 0.5 * _grad_test[_i][_qp] * _interface_normal * (*_prop_jump_old)[_qp] +
           0.5 * _test[_i][_qp] * _jump_flux;
      r += _alpha * (_u[_qp] - _u_neighbor[_qp] - (*_prop_jump_old)[_qp]) * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      r += (0.5 * _grad_u[_qp] * _interface_normal +
            0.5 * _grad_u_neighbor[_qp] * _interface_normal) *
           _test_neighbor[_i][_qp];
      r -= (_u[_qp] - _u_neighbor[_qp]) * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r += 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal * (*_prop_jump_old)[_qp] +
           0.5 * _test_neighbor[_i][_qp] * _jump_flux;
      r -= _alpha * (_u[_qp] - _u_neighbor[_qp] - (*_prop_jump_old)[_qp]) * _test_neighbor[_i][_qp];
      break;
  }
  return r;
}

Real
XFEMSingleVariableConstraintStatefulTest::computeQpJacobian(Moose::DGJacobianType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::ElementElement:
      r += -0.5 * _grad_phi[_j][_qp] * _interface_normal * _test[_i][_qp] -
           _phi[_j][_qp] * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r += _alpha * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      r += -0.5 * _grad_phi_neighbor[_j][_qp] * _interface_normal * _test[_i][_qp] +
           _phi_neighbor[_j][_qp] * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r -= _alpha * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborElement:
      r += 0.5 * _grad_phi[_j][_qp] * _interface_normal * _test_neighbor[_i][_qp] -
           _phi[_j][_qp] * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r -= _alpha * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      r += 0.5 * _grad_phi_neighbor[_j][_qp] * _interface_normal * _test_neighbor[_i][_qp] +
           _phi_neighbor[_j][_qp] * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r += _alpha * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }

  return r;
}
