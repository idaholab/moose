//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMSingleVariableConstraint.h"

// MOOSE includes
#include "Assembly.h"
#include "ElementPairInfo.h"
#include "FEProblem.h"
#include "GeometricCutUserObject.h"
#include "XFEM.h"
#include "Function.h"

#include "libmesh/quadrature.h"

registerMooseObject("XFEMApp", XFEMSingleVariableConstraint);

InputParameters
XFEMSingleVariableConstraint::validParams()
{
  InputParameters params = ElemElemConstraint::validParams();
  params.addParam<Real>("alpha",
                        100,
                        "Stabilization parameter in Nitsche's formulation and penalty factor "
                        "in the Penalty Method. In Nitsche's formulation this should be as "
                        "small as possible while the method is still stable; while in the "
                        "Penalty Method you want this to be quite large (e.g. 1e6).");
  params.addParam<FunctionName>("jump", 0, "Jump at the interface. Can be a Real or FunctionName.");
  params.addParam<FunctionName>(
      "jump_flux", 0, "Flux jump at the interface. Can be a Real or FunctionName.");
  params.addParam<UserObjectName>(
      "geometric_cut_userobject",
      "Name of GeometricCutUserObject associated with this constraint.");
  params.addParam<bool>(
      "use_penalty",
      false,
      "Use the Penalty instead of Nitsche (Nitsche only works for simple diffusion problems).");
  params.addClassDescription("Enforce constraints on the value or flux associated with a variable "
                             "at an XFEM interface.");
  return params;
}

XFEMSingleVariableConstraint::XFEMSingleVariableConstraint(const InputParameters & parameters)
  : ElemElemConstraint(parameters),
    _alpha(getParam<Real>("alpha")),
    _jump(getFunction("jump")),
    _jump_flux(getFunction("jump_flux")),
    _use_penalty(getParam<bool>("use_penalty"))
{
  _xfem = std::dynamic_pointer_cast<XFEM>(_fe_problem.getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in XFEMSingleVariableConstraint");

  const UserObject * uo =
      &(_fe_problem.getUserObjectBase(getParam<UserObjectName>("geometric_cut_userobject")));

  if (dynamic_cast<const GeometricCutUserObject *>(uo) == nullptr)
    mooseError("UserObject casting to GeometricCutUserObject in XFEMSingleVariableConstraint");

  _interface_id = _xfem->getGeometricCutID(dynamic_cast<const GeometricCutUserObject *>(uo));
}

XFEMSingleVariableConstraint::~XFEMSingleVariableConstraint() {}

void
XFEMSingleVariableConstraint::reinitConstraintQuadrature(const ElementPairInfo & element_pair_info)
{
  _interface_normal = element_pair_info._elem1_normal;
  ElemElemConstraint::reinitConstraintQuadrature(element_pair_info);
}

Real
XFEMSingleVariableConstraint::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      if (!_use_penalty)
      {
        r -= (0.5 * _grad_u[_qp] * _interface_normal +
              0.5 * _grad_u_neighbor[_qp] * _interface_normal) *
             _test[_i][_qp];
        r -= (_u[_qp] - _u_neighbor[_qp]) * 0.5 * _grad_test[_i][_qp] * _interface_normal;
        r += 0.5 * _grad_test[_i][_qp] * _interface_normal * _jump.value(_t, _u[_qp]);
      }
      r += 0.5 * _test[_i][_qp] * _jump_flux.value(_t, _u[_qp]);
      r += _alpha * (_u[_qp] - _u_neighbor[_qp] - _jump.value(_t, _u[_qp])) * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      if (!_use_penalty)
      {
        r += (0.5 * _grad_u[_qp] * _interface_normal +
              0.5 * _grad_u_neighbor[_qp] * _interface_normal) *
             _test_neighbor[_i][_qp];
        r -= (_u[_qp] - _u_neighbor[_qp]) * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
        r += 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal *
             _jump.value(_t, _u_neighbor[_qp]);
      }
      r += 0.5 * _test_neighbor[_i][_qp] * _jump_flux.value(_t, _u_neighbor[_qp]);
      r -= _alpha * (_u[_qp] - _u_neighbor[_qp] - _jump.value(_t, _u_neighbor[_qp])) *
           _test_neighbor[_i][_qp];
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
      if (!_use_penalty)
        r += -0.5 * _grad_phi[_j][_qp] * _interface_normal * _test[_i][_qp] -
             _phi[_j][_qp] * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r += _alpha * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      if (!_use_penalty)
        r += -0.5 * _grad_phi_neighbor[_j][_qp] * _interface_normal * _test[_i][_qp] +
             _phi_neighbor[_j][_qp] * 0.5 * _grad_test[_i][_qp] * _interface_normal;
      r -= _alpha * _phi_neighbor[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborElement:
      if (!_use_penalty)
        r += 0.5 * _grad_phi[_j][_qp] * _interface_normal * _test_neighbor[_i][_qp] -
             _phi[_j][_qp] * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r -= _alpha * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      if (!_use_penalty)
        r += 0.5 * _grad_phi_neighbor[_j][_qp] * _interface_normal * _test_neighbor[_i][_qp] +
             _phi_neighbor[_j][_qp] * 0.5 * _grad_test_neighbor[_i][_qp] * _interface_normal;
      r += _alpha * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;
  }

  return r;
}
