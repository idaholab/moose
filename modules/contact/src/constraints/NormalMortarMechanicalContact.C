//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalMortarMechanicalContact.h"
#include "WeightedGapUserObject.h"

registerMooseObject("ContactApp", NormalMortarMechanicalContact);

InputParameters
NormalMortarMechanicalContact::validParams()
{
  InputParameters params = ADMortarLagrangeConstraint::validParams();

  MooseEnum component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "component", component, "The force component constraint that this object is supplying");
  params.addClassDescription(
      "This class is used to apply normal contact forces using lagrange multipliers");
  params.set<bool>("compute_lm_residual") = false;
  params.set<bool>("interpolate_normals") = false;
  params.addRequiredParam<UserObjectName>("weighted_gap_uo", "The weighted gap user object.");
  return params;
}

NormalMortarMechanicalContact::NormalMortarMechanicalContact(const InputParameters & parameters)
  : ADMortarLagrangeConstraint(parameters),
    _component(getParam<MooseEnum>("component")),
    _weighted_gap_uo(const_cast<WeightedGapUserObject &>(
        getUserObject<WeightedGapUserObject>("weighted_gap_uo")))
{
}

ADReal
NormalMortarMechanicalContact::computeQpResidual(Moose::MortarType type)
{
  switch (type)
  {
    case Moose::MortarType::Secondary:
      // If normals is positive, then this residual is positive, indicating that we have an outflow
      // of momentum, which in turn indicates that the momentum will tend to decrease at this
      // location with time, which is what we want because the force vector is in the negative
      // direction (always opposite of the normals). Conversely, if the normals is negative, then
      // this residual is negative, indicating that we have an inflow of momentum, which in turn
      // indicates the momentum will tend to increase at this location with time, which is what we
      // want because the force vector is in the positive direction (always opposite of the
      // normals).
      // Get the _dof_to_weighted_gap map
      {
        const auto normal_index = libmesh_map_find(_secondary_ip_lowerd_map, _i);
        return _test_secondary[_i][_qp] * _weighted_gap_uo.contactPressure()[_qp] *
               _normals[normal_index](_component);
      }

    case Moose::MortarType::Primary:
      // The normal vector is signed according to the secondary face, so we need to introduce a
      // negative sign here
      {
        const auto normal_index = libmesh_map_find(_primary_ip_lowerd_map, _i);
        return -_test_primary[_i][_qp] * _weighted_gap_uo.contactPressure()[_qp] *
               _normals[normal_index](_component);
      }
    default:
      return 0;
  }
}
