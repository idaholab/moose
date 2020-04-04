//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMEqualValueAtInterface.h"
#include "FEProblem.h"
#include "GeometricCutUserObject.h"
#include "XFEM.h"

registerMooseObject("XFEMApp", XFEMEqualValueAtInterface);

InputParameters
XFEMEqualValueAtInterface::validParams()
{
  InputParameters params = ElemElemConstraint::validParams();
  params.addRequiredParam<Real>("alpha", "Penalty parameter in penalty formulation.");
  params.addRequiredParam<Real>("value", "Prescribed value at the interface.");
  params.addParam<UserObjectName>(
      "geometric_cut_userobject",
      "Name of GeometricCutUserObject associated with this constraint.");
  params.addClassDescription("Enforce that the solution have the same value on opposing sides of "
                             "an XFEM interface.");
  return params;
}

XFEMEqualValueAtInterface::XFEMEqualValueAtInterface(const InputParameters & parameters)
  : ElemElemConstraint(parameters), _alpha(getParam<Real>("alpha")), _value(getParam<Real>("value"))
{
  _xfem = std::dynamic_pointer_cast<XFEM>(_fe_problem.getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in XFEMEqualValueAtInterface");

  const UserObject * uo =
      &(_fe_problem.getUserObjectBase(getParam<UserObjectName>("geometric_cut_userobject")));

  if (dynamic_cast<const GeometricCutUserObject *>(uo) == nullptr)
    mooseError("UserObject casting to GeometricCutUserObject in XFEMEqualValueAtInterface");

  _interface_id = _xfem->getGeometricCutID(dynamic_cast<const GeometricCutUserObject *>(uo));
}

XFEMEqualValueAtInterface::~XFEMEqualValueAtInterface() {}

void
XFEMEqualValueAtInterface::reinitConstraintQuadrature(const ElementPairInfo & element_pair_info)
{
  ElemElemConstraint::reinitConstraintQuadrature(element_pair_info);
}

Real
XFEMEqualValueAtInterface::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::Element:
      r += _alpha * (_u[_qp] - _value) * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      r += _alpha * (_u_neighbor[_qp] - _value) * _test_neighbor[_i][_qp];
      break;
  }
  return r;
}

Real
XFEMEqualValueAtInterface::computeQpJacobian(Moose::DGJacobianType type)
{
  Real r = 0;

  switch (type)
  {
    case Moose::ElementElement:
      r += _alpha * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      r += _alpha * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    default:
      break;
  }

  return r;
}
