//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDynamicWeightedGapLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

registerMooseObject("ContactApp", ComputeDynamicWeightedGapLMMechanicalContact);

namespace
{
}

InputParameters
ComputeDynamicWeightedGapLMMechanicalContact::validParams()
{
  InputParameters params = ComputeWeightedGapLMMechanicalContact::validParams();
  params.addClassDescription(
      "Computes the normal contact mortar constraints for dynamic simulations");
  params.addRangeCheckedParam<Real>("capture_tolerance",
                                    1.0e-5,
                                    "capture_tolerance>=0",
                                    "Parameter describing a gap threshold for the application of "
                                    "the persistency constraint in dynamic simulations.");

  return params;
}

ComputeDynamicWeightedGapLMMechanicalContact::ComputeDynamicWeightedGapLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeWeightedGapLMMechanicalContact(parameters),
    _capture_tolerance(getParam<Real>("capture_tolerance")),
    _secondary_x_dot(adCoupledDot("disp_x")),
    _primary_x_dot(adCoupledNeighborValueDot("disp_x")),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr)
{
  mooseAssert(!_interpolate_normals,
              "Dynamic mortar mechanical contact constraints require the surface geometry be "
              "attached to nodes");
}

void
ComputeDynamicWeightedGapLMMechanicalContact::computeQpProperties()
{
  ADRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

  ADRealVectorValue relative_velocity(_primary_x_dot[_qp] - _secondary_x_dot[_qp],
                                      _primary_y_dot[_qp] - _secondary_y_dot[_qp],
                                      0.0);

  gap_vec(0).derivatives() =
      _primary_disp_x[_qp].derivatives() - _secondary_disp_x[_qp].derivatives();
  gap_vec(1).derivatives() =
      _primary_disp_y[_qp].derivatives() - _secondary_disp_y[_qp].derivatives();

  relative_velocity(0).derivatives() =
      _primary_x_dot[_qp].derivatives() - _secondary_x_dot[_qp].derivatives();
  relative_velocity(1).derivatives() =
      _primary_y_dot[_qp].derivatives() - _secondary_y_dot[_qp].derivatives();

  if (_has_disp_z)
  {
    relative_velocity(2) = (*_primary_z_dot)[_qp] - (*_secondary_z_dot)[_qp];
    relative_velocity(2).derivatives() =
        (*_primary_z_dot)[_qp].derivatives() - (*_secondary_z_dot)[_qp].derivatives();
    gap_vec(2).derivatives() =
        (*_primary_disp_z)[_qp].derivatives() - (*_secondary_disp_z)[_qp].derivatives();
  }

  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);
  _qp_gap_nodal_dynamics = (relative_velocity * _dt) * (_JxW_msm[_qp] * _coord[_qp]);
  //    (2.0 * gap_vec / _dt / _dt - 2.0 * relative_velocity / _dt - relative_acc) *

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_qp] * _coord[_qp];
}

void
ComputeDynamicWeightedGapLMMechanicalContact::computeQpIProperties()
{
  mooseAssert(_normals.size() == _lower_secondary_elem->n_nodes(),
              "Making sure that _normals is the expected size");

  // Get the _dof_to_weighted_gap map
  const DofObject * dof = _var->isNodal()
                              ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
                              : static_cast<const DofObject *>(_lower_secondary_elem);

  _dof_to_weighted_gap[dof].first += _test[_i][_qp] * _qp_gap_nodal * _normals[_i];
  _dof_to_weighted_gap_dynamics[dof] += _test[_i][_qp] * _qp_gap_nodal_dynamics * _normals[_i];

  if (_normalize_c)
    _dof_to_weighted_gap[dof].second += _test[_i][_qp] * _qp_factor;
}

void
ComputeDynamicWeightedGapLMMechanicalContact::timestepSetup()
{
  _dof_to_old_weighted_gap.clear();
  for (auto & map_pr : _dof_to_weighted_gap)
    _dof_to_old_weighted_gap.emplace(map_pr.first, std::move(map_pr.second.first));
}

void
ComputeDynamicWeightedGapLMMechanicalContact::residualSetup()
{
  ComputeWeightedGapLMMechanicalContact::residualSetup();
  _dof_to_weighted_gap_dynamics.clear();
}

void
ComputeDynamicWeightedGapLMMechanicalContact::post()
{
  communicateGaps();

  // There is a need for the dynamic constraint to uncouple the computation of the weighted gap from
  // the computation of the constraint itself since we are switching from gap constraint to
  // persistency constraint.
  for (const auto & pr : _dof_to_weighted_gap)
  {
    if (pr.first->processor_id() != this->processor_id())
      continue;

    const auto is_dof_on_map = _dof_to_old_weighted_gap.find(pr.first);

    // If is_dof_on_map isn't on map, it means it's an initial step
    if (is_dof_on_map == _dof_to_old_weighted_gap.end() ||
        _dof_to_old_weighted_gap[pr.first] > _capture_tolerance)
      _weighted_gap_ptr = &pr.second.first;
    else
      _weighted_gap_ptr = &_dof_to_weighted_gap_dynamics[pr.first];

    _normalization_ptr = &pr.second.second;

    ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(pr.first);
  }
}

void
ComputeDynamicWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  communicateGaps();

  for (const auto & pr : _dof_to_weighted_gap)
  {
    if ((inactive_lm_nodes.find(static_cast<const Node *>(pr.first)) != inactive_lm_nodes.end()) ||
        (pr.first->processor_id() != this->processor_id()))
      continue;

    const auto is_dof_on_map = _dof_to_old_weighted_gap.find(pr.first);

    // If is_dof_on_map isn't on map, it means it's an initial step
    if (is_dof_on_map == _dof_to_old_weighted_gap.end() ||
        _dof_to_old_weighted_gap[pr.first] > _capture_tolerance)
    {
      // If this is the first step or the previous step gap is not identified as in contact, apply
      // regular conditions
      _weighted_gap_ptr = &pr.second.first;
    }
    else
    {
      // Enable the application of persistency condition
      _weighted_gap_ptr = &_dof_to_weighted_gap_dynamics[pr.first];
    }

    _normalization_ptr = &pr.second.second;

    ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(pr.first);
  }
}
