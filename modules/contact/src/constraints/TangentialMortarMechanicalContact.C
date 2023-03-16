//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TangentialMortarMechanicalContact.h"
#include "WeightedVelocitiesUserObject.h"

registerMooseObject("ContactApp", TangentialMortarMechanicalContact);

InputParameters
TangentialMortarMechanicalContact::validParams()
{
  InputParameters params = ADMortarLagrangeConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");

  // This enum is used to pick the proper 'tangent' vector (i.e. tangent or binormal)
  MooseEnum direction("direction_1 direction_2", "direction_1");
  params.addParam<MooseEnum>("direction",
                             direction,
                             "Tangent direction to compute the residual due to frictional contact");
  params.addClassDescription(
      "Used to apply tangential stresses from frictional contact using lagrange multipliers");
  params.addRequiredParam<UserObjectName>("weighted_velocities_uo",
                                          "The weighted velocities user object.");
  params.set<bool>("interpolate_normals") = false;
  params.set<bool>("compute_lm_residual") = false;
  return params;
}

TangentialMortarMechanicalContact::TangentialMortarMechanicalContact(
    const InputParameters & parameters)
  : ADMortarLagrangeConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _direction(getParam<MooseEnum>("direction")),
    _weighted_velocities_uo(const_cast<WeightedVelocitiesUserObject &>(
        getUserObject<WeightedVelocitiesUserObject>("weighted_velocities_uo")))
{
}

ADReal
TangentialMortarMechanicalContact::computeQpResidual(Moose::MortarType type)
{
  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);
  MooseEnum direction("direction_1 direction_2", "direction_1");

  const auto tangential_pressure =
      _direction.compareCurrent(direction)
          ? _weighted_velocities_uo.contactTangentialPressureDirOne()[_qp]
          : _weighted_velocities_uo.contactTangentialPressureDirTwo()[_qp];

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
      {
        const unsigned int tangent_index = libmesh_map_find(_secondary_ip_lowerd_map, _i);
        return _test_secondary[_i][_qp] * tangential_pressure *
               nodal_tangents[_direction][tangent_index](_component) /
               nodal_tangents[_direction][tangent_index].norm();
      }
    case Moose::MortarType::Primary:
    {
      const unsigned int tangent_index = libmesh_map_find(_primary_ip_lowerd_map, _i);
      return -_test_primary[_i][_qp] * tangential_pressure *
             nodal_tangents[_direction][tangent_index](_component) /
             nodal_tangents[_direction][tangent_index].norm();
    }
    default:
      return 0;
  }
}
