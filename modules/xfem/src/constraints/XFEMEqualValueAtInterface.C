//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
registerMooseObject("XFEMApp", ADXFEMEqualValueAtInterface);

template <bool is_ad>
InputParameters
XFEMEqualValueAtInterfaceTempl<is_ad>::validParams()
{
  InputParameters params = GenericElemElemConstraint<is_ad>::validParams();
  params.addRequiredParam<Real>("alpha", "Penalty parameter in penalty formulation.");
  params.addRequiredParam<Real>("value", "Prescribed value at the interface.");
  params.addParam<UserObjectName>(
      "geometric_cut_userobject",
      "Name of GeometricCutUserObject associated with this constraint.");
  params.addClassDescription("Enforce that the solution have the same value on opposing sides of "
                             "an XFEM interface.");
  return params;
}

template <bool is_ad>
XFEMEqualValueAtInterfaceTempl<is_ad>::XFEMEqualValueAtInterfaceTempl(
    const InputParameters & parameters)
  : GenericElemElemConstraint<is_ad>(parameters),
    _alpha(this->template getParam<Real>("alpha")),
    _value(this->template getParam<Real>("value"))
{
  _xfem = std::dynamic_pointer_cast<XFEM>(_fe_problem.getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in XFEMEqualValueAtInterface");

  const UserObject * uo = &(_fe_problem.getUserObjectBase(
      this->template getParam<UserObjectName>("geometric_cut_userobject")));

  if (dynamic_cast<const GeometricCutUserObject *>(uo) == nullptr)
    mooseError("UserObject casting to GeometricCutUserObject in XFEMEqualValueAtInterface");

  _interface_id = _xfem->getGeometricCutID(dynamic_cast<const GeometricCutUserObject *>(uo));
}

template <bool is_ad>
XFEMEqualValueAtInterfaceTempl<is_ad>::~XFEMEqualValueAtInterfaceTempl()
{
}

template <bool is_ad>
void
XFEMEqualValueAtInterfaceTempl<is_ad>::reinitConstraintQuadrature(
    const ElementPairInfo & element_pair_info)
{
  GenericElemElemConstraint<is_ad>::reinitConstraintQuadrature(element_pair_info);
}

template <bool is_ad>
GenericReal<is_ad>
XFEMEqualValueAtInterfaceTempl<is_ad>::computeQpResidual(Moose::DGResidualType type)
{
  GenericReal<is_ad> r = 0;

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

template <bool is_ad>
Real
XFEMEqualValueAtInterfaceTempl<is_ad>::computeQpJacobian(Moose::DGJacobianType type)
{
  if (is_ad)
    return 0;

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

template class XFEMEqualValueAtInterfaceTempl<false>;
template class XFEMEqualValueAtInterfaceTempl<true>;
