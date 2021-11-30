//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialMortarMechanicalContact.h"

registerMooseObject("ContactApp", TangentialMortarMechanicalContact);

InputParameters
TangentialMortarMechanicalContact::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");

  MooseEnum direction("direction_1 direction_2", "direction_1");
  params.addParam<MooseEnum>("direction",
                             direction,
                             "Tangent direction to compute the residual due to frictional contact");

  params.addClassDescription(
      "Used to apply tangential stresses from frictional contact using lagrange multipliers");
  params.set<bool>("compute_lm_residual") = false;
  return params;
}

TangentialMortarMechanicalContact::TangentialMortarMechanicalContact(
    const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _direction(getParam<MooseEnum>("direction"))
{
}

ADReal
TangentialMortarMechanicalContact::computeQpResidual(Moose::MortarType type)
{
  switch (type)
  {
    case Moose::MortarType::Secondary:
      // We have taken the convention the lagrange multiplier must have the same sign as the
      // relative slip velocity of the secondary face. So positive lambda indicates that force is
      // being applied in the negative direction, so we want to decrease the momentum in the system,
      // which means we want an outflow of momentum, which means we want the residual to be positive
      // in that case. Negative lambda means force is being applied in the positive direction, so we
      // want to increase momentum in the system, which means we want an inflow of momentum, which
      // means we want the residual to be negative in that case. So the sign of this residual should
      // be the same as the sign of lambda

      if (!_interpolate_normals && !_secondary_ip_lowerd_map.count(_i))
        return 0.0;

      if (_interpolate_normals)
        return _test_secondary[_i][_qp] * _lambda[_qp] * _tangents[_qp][_direction](_component) /
               _tangents[_qp][_direction].norm();
      else
      {
        if (!_interpolate_normals)
        {
          unsigned int tangent_index = 0;
          tangent_index = _secondary_ip_lowerd_map.at(_i);
          return _test_secondary[tangent_index][_qp] * _lambda[_qp] *
                 _tangents_libmesh_3d[tangent_index][_direction](_component) /
                 _tangents_libmesh_3d[tangent_index][_direction].norm();
        }
      }

    case Moose::MortarType::Primary:
      // Equal and opposite reactions so we put a negative sign here
      if (!_interpolate_normals && !_primary_ip_lowerd_map.count(_i))
        return 0.0;

      if (_interpolate_normals)
        return -_test_primary[_i][_qp] * _lambda[_qp] * _tangents[_qp][_direction](_component) /
               _tangents[_qp][_direction].norm();
      else
      {
        unsigned int tangent_index = 0;
        tangent_index = _primary_ip_lowerd_map.at(_i);
        return -_test_primary[tangent_index][_qp] * _lambda[_qp] *
               _tangents_libmesh_3d[tangent_index][_direction](_component) /
               _tangents_libmesh_3d[tangent_index][_direction].norm();
      }

    default:
      return 0;
  }
}
