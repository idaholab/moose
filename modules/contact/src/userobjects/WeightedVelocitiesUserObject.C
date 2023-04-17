//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeightedVelocitiesUserObject.h"
#include "MooseVariableField.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "MortarContactUtils.h"
#include "libmesh/quadrature.h"

InputParameters
WeightedVelocitiesUserObject::validParams()
{
  InputParameters params = WeightedGapUserObject::validParams();
  params.addRequiredParam<VariableName>("secondary_variable",
                                        "Primal variable on secondary surface.");
  params.addParam<VariableName>(
      "primary_variable",
      "Primal variable on primary surface. If this parameter is not provided then the primary "
      "variable will be initialized to the secondary variable");
  return params;
}

WeightedVelocitiesUserObject::WeightedVelocitiesUserObject(const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _secondary_var(
        isParamValid("secondary_variable")
            ? _sys.getActualFieldVariable<Real>(_tid, parameters.getMooseType("secondary_variable"))
            : _sys.getActualFieldVariable<Real>(_tid, parameters.getMooseType("primary_variable"))),
    _primary_var(
        isParamValid("primary_variable")
            ? _sys.getActualFieldVariable<Real>(_tid, parameters.getMooseType("primary_variable"))
            : _secondary_var),
    _secondary_x_dot(_secondary_var.adUDot()),
    _primary_x_dot(_primary_var.adUDotNeighbor()),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _3d(_has_disp_z)
{
  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must be true for the WeightedVelocitiesUserObject object");
}

void
WeightedVelocitiesUserObject::initialSetup()
{
  WeightedGapUserObject::initialSetup();
}

void
WeightedVelocitiesUserObject::computeQpProperties()
{
  // Compute the value of _qp_gap
  WeightedGapUserObject::computeQpProperties();

  // Trim derivatives
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_disp_dot{
      {_primary_x_dot[_qp], _primary_y_dot[_qp], _has_disp_z ? (*_primary_z_dot)[_qp] : 0}};
  std::array<ADReal, 3> secondary_disp_dot{
      {_secondary_x_dot[_qp], _secondary_y_dot[_qp], _has_disp_z ? (*_secondary_z_dot)[_qp] : 0}};

  trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, primary_disp_dot, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, secondary_disp_dot, true);

  const ADReal & prim_x_dot = primary_disp_dot[0];
  const ADReal & prim_y_dot = primary_disp_dot[1];
  const ADReal * prim_z_dot = nullptr;
  if (_has_disp_z)
    prim_z_dot = &primary_disp_dot[2];

  const ADReal & sec_x_dot = secondary_disp_dot[0];
  const ADReal & sec_y_dot = secondary_disp_dot[1];
  const ADReal * sec_z_dot = nullptr;
  if (_has_disp_z)
    sec_z_dot = &secondary_disp_dot[2];

  // Build relative velocity vector
  ADRealVectorValue relative_velocity;

  if (_3d)
    relative_velocity = {sec_x_dot - prim_x_dot, sec_y_dot - prim_y_dot, *sec_z_dot - *prim_z_dot};
  else
    relative_velocity = {sec_x_dot - prim_x_dot, sec_y_dot - prim_y_dot, 0.0};

  // Geometry is averaged and used at the nodes for constraint enforcement.
  _qp_real_tangential_velocity_nodal = relative_velocity;
  _qp_tangential_velocity_nodal = relative_velocity * (_JxW_msm[_qp] * _coord[_qp]);
}

void
WeightedVelocitiesUserObject::computeQpIProperties()
{
  WeightedGapUserObject::computeQpIProperties();

  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);
  // Get the _dof_to_weighted_tangential_velocity map
  const DofObject * const dof =
      _is_weighted_gap_nodal ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
                             : static_cast<const DofObject *>(_lower_secondary_elem);

  _dof_to_weighted_tangential_velocity[dof][0] +=
      (*_test)[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[0][_i];
  _dof_to_real_tangential_velocity[dof][0] =
      _qp_real_tangential_velocity_nodal * nodal_tangents[0][_i];

  // Get the _dof_to_weighted_tangential_velocity map for a second direction
  if (_3d)
  {
    _dof_to_weighted_tangential_velocity[dof][1] +=
        (*_test)[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[1][_i];

    _dof_to_real_tangential_velocity[dof][1] =
        _qp_real_tangential_velocity_nodal * nodal_tangents[1][_i];
  }
}

void
WeightedVelocitiesUserObject::initialize()
{
  // Clear weighted gaps
  WeightedGapUserObject::initialize();

  _dof_to_weighted_tangential_velocity.clear();
  _dof_to_real_tangential_velocity.clear();
}

void
WeightedVelocitiesUserObject::finalize()
{
  WeightedGapUserObject::finalize();

  // If the constraint is performed by the owner, then we don't need any data sent back; the owner
  // will take care of it. But if the constraint is not performed by the owner and we might have to
  // do some of the constraining ourselves, then we need data sent back to us
  const bool send_data_back = !constrainedByOwner();

  Moose::Mortar::Contact::communicateVelocities(_dof_to_weighted_tangential_velocity,
                                                _subproblem.mesh(),
                                                _nodal,
                                                _communicator,
                                                send_data_back);
  Moose::Mortar::Contact::communicateVelocities(
      _dof_to_real_tangential_velocity, _subproblem.mesh(), _nodal, _communicator, send_data_back);
}

void
WeightedVelocitiesUserObject::execute()
{
  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test->size(); ++_i)
      computeQpIProperties();
  }
}
