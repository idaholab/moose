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

#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

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
  params.addCoupledVar("wear_depth",
                       "The name of the mortar auxiliary variable that is used to modify the "
                       "weighted gap definition");
  params.addRequiredRangeCheckedParam<Real>(
      "newmark_beta", "newmark_beta > 0", "Beta parameter for the Newmark time integrator");
  params.addRequiredRangeCheckedParam<Real>(
      "newmark_gamma", "newmark_gamma >= 0.25", "Gamma parameter for the Newmark time integrator");

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
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _has_wear(isParamValid("wear_depth")),
    _wear_depth(_has_wear ? coupledValueLower("wear_depth") : _zero),
    _newmark_beta(getParam<Real>("newmark_beta")),
    _newmark_gamma(getParam<Real>("newmark_gamma"))
{
  mooseAssert(!_interpolate_normals,
              "Dynamic mortar mechanical contact constraints require the surface geometry to be "
              "attached to nodes");

  if (!useDual())
    mooseError("Dynamic mortar contact constraints requires the use of Lagrange multipliers dual "
               "interpolation");
}

void
ComputeDynamicWeightedGapLMMechanicalContact::computeQpProperties()
{
  // Trim interior node variable derivatives
  const auto & primary_ip_lowerd_map = amg().getPrimaryIpToLowerElementMap(
      *_lower_primary_elem, *_lower_primary_elem->interior_parent(), *_lower_secondary_elem);
  const auto & secondary_ip_lowerd_map =
      amg().getSecondaryIpToLowerElementMap(*_lower_secondary_elem);

  ADReal prim_x = _primary_disp_x[_qp];
  ADReal prim_y = _primary_disp_y[_qp];
  ADReal prim_z = _primary_disp_z ? (*_primary_disp_z)[_qp] : 0.0;

  ADReal sec_x = _secondary_disp_x[_qp];
  ADReal sec_y = _secondary_disp_y[_qp];
  ADReal sec_z = _secondary_disp_z ? (*_secondary_disp_z)[_qp] : 0.0;

#ifdef MOOSE_GLOBAL_AD_INDEXING
  std::array<MooseVariable *, 3> var_array{
      {getVar("disp_x", 0), getVar("disp_y", 0), _has_disp_z ? getVar("disp_z", 0) : nullptr}};
  trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, prim_x, prim_y, prim_z, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, sec_x, sec_y, sec_z, true);
#endif

  ADReal prim_x_dot = _primary_x_dot[_qp];
  ADReal prim_y_dot = _primary_y_dot[_qp];
  ADReal prim_z_dot = _primary_z_dot ? (*_primary_z_dot)[_qp] : 0.0;

  ADReal sec_x_dot = _secondary_x_dot[_qp];
  ADReal sec_y_dot = _secondary_y_dot[_qp];
  ADReal sec_z_dot = _secondary_z_dot ? (*_secondary_z_dot)[_qp] : 0.0;

#ifdef MOOSE_GLOBAL_AD_INDEXING
  trimInteriorNodeDerivatives(
      primary_ip_lowerd_map, var_array, prim_x_dot, prim_y_dot, prim_z_dot, false);
  trimInteriorNodeDerivatives(
      secondary_ip_lowerd_map, var_array, sec_x_dot, sec_y_dot, sec_z_dot, true);
#endif

  // Compute dynamic constraint-related quantities
  ADRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];

  ADRealVectorValue relative_velocity(prim_x_dot - sec_x_dot, prim_y_dot - sec_y_dot, 0.0);
  gap_vec(0).derivatives() = prim_x.derivatives() - sec_x.derivatives();
  gap_vec(1).derivatives() = prim_y.derivatives() - sec_y.derivatives();

  if (_has_disp_z)
  {
    relative_velocity(2) = prim_z_dot - sec_z_dot;
    gap_vec(2) = prim_z - sec_z;
  }

  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);
  _qp_velocity = relative_velocity * (_JxW_msm[_qp] * _coord[_qp]);

  // Current part of the gap velocity Newmark-beta time discretization
  _qp_gap_nodal_dynamics =
      (_newmark_gamma / _newmark_beta * gap_vec / _dt) * (_JxW_msm[_qp] * _coord[_qp]);

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

  // Regular normal contact constraint: Use before contact is established for contact detection
  _dof_to_weighted_gap[dof].first += _test[_i][_qp] * (_qp_gap_nodal * _normals[_i]);

  // Integrated part of the "persistency" constraint
  _dof_to_weighted_gap_dynamics[dof] += _test[_i][_qp] * _qp_gap_nodal_dynamics * _normals[_i];
  _dof_to_velocity[dof] += _test[_i][_qp] * _qp_velocity * _normals[_i];

  _dof_to_nodal_wear_depth[dof] += _test[_i][_qp] * _wear_depth[_qp] * _JxW_msm[_qp] * _coord[_qp];

  if (_normalize_c)
    _dof_to_weighted_gap[dof].second += _test[_i][_qp] * _qp_factor;
}

void
ComputeDynamicWeightedGapLMMechanicalContact::timestepSetup()
{
  _dof_to_old_weighted_gap.clear();
  _dof_to_old_velocity.clear();
  _dof_to_nodal_old_wear_depth.clear();

  for (auto & map_pr : _dof_to_weighted_gap)
    _dof_to_old_weighted_gap.emplace(map_pr.first, std::move(map_pr.second.first));

  for (auto & map_pr : _dof_to_velocity)
    _dof_to_old_velocity.emplace(map_pr);

  for (auto & map_pr : _dof_to_nodal_wear_depth)
    _dof_to_nodal_old_wear_depth.emplace(map_pr);
}

void
ComputeDynamicWeightedGapLMMechanicalContact::residualSetup()
{
  ComputeWeightedGapLMMechanicalContact::residualSetup();
  _dof_to_weighted_gap_dynamics.clear();
  _dof_to_velocity.clear();

  // Wear
  _dof_to_nodal_wear_depth.clear();
}

void
ComputeDynamicWeightedGapLMMechanicalContact::post()
{
  communicateGaps();

  if (_has_wear)
    communicateWear();

  // There is a need for the dynamic constraint to uncouple the computation of the weighted gap from
  // the computation of the constraint itself since we are switching from gap constraint to
  // persistency constraint.
  for (const auto & pr : _dof_to_weighted_gap)
  {
    if (pr.first->processor_id() != this->processor_id())
      continue;

    //
    _dof_to_weighted_gap[pr.first].first += _dof_to_nodal_wear_depth[pr.first];
    _dof_to_weighted_gap_dynamics[pr.first] +=
        _newmark_gamma / _newmark_beta * _dof_to_nodal_wear_depth[pr.first] / _dt;
    //

    const auto is_dof_on_map = _dof_to_old_weighted_gap.find(pr.first);

    // If is_dof_on_map isn't on map, it means it's an initial step
    if (is_dof_on_map == _dof_to_old_weighted_gap.end() ||
        _dof_to_old_weighted_gap[pr.first] > _capture_tolerance)
      _weighted_gap_ptr = &pr.second.first;
    else
    {
      ADReal term = -_newmark_gamma / _newmark_beta / _dt * _dof_to_old_weighted_gap[pr.first];
      term += _dof_to_old_velocity[pr.first];
      _dof_to_weighted_gap_dynamics[pr.first] += term;

      _weighted_gap_ptr = &_dof_to_weighted_gap_dynamics[pr.first];
    }

    _normalization_ptr = &pr.second.second;

    ComputeWeightedGapLMMechanicalContact::enforceConstraintOnDof(pr.first);
  }
}

void
ComputeDynamicWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  communicateGaps();

  if (_has_wear)
    communicateWear();

  for (const auto & pr : _dof_to_weighted_gap)
  {
    if ((inactive_lm_nodes.find(static_cast<const Node *>(pr.first)) != inactive_lm_nodes.end()) ||
        (pr.first->processor_id() != this->processor_id()))
      continue;

    //
    _dof_to_weighted_gap[pr.first].first += _dof_to_nodal_wear_depth[pr.first];
    _dof_to_weighted_gap_dynamics[pr.first] +=
        _newmark_gamma / _newmark_beta * _dof_to_nodal_wear_depth[pr.first] / _dt;
    //
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

void
ComputeDynamicWeightedGapLMMechanicalContact::communicateWear()
{
#ifdef MOOSE_SPARSE_AD
  // We may have wear depth information that should go to other processes that own the dofs
  using Datum = std::pair<dof_id_type, ADReal>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (auto & pr : _dof_to_nodal_wear_depth)
  {
    const auto * const dof_object = pr.first;
    const auto proc_id = dof_object->processor_id();
    if (proc_id == this->processor_id())
      continue;

    push_data[proc_id].push_back(std::make_pair(dof_object->id(), std::move(pr.second)));
  }

  const auto & lm_mesh = _mesh.getMesh();

  auto action_functor = [this, &lm_mesh](const processor_id_type libmesh_dbg_var(pid),
                                         const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != this->processor_id(), "We do not send messages to ourself here");
    for (auto & pr : sent_data)
    {
      const auto dof_id = pr.first;
      const auto * const dof_object =
          _nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                 : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This should be non-null");
      _dof_to_nodal_wear_depth[dof_object] += std::move(pr.second);
    }
  };

  TIMPI::push_parallel_vector_data(_communicator, push_data, action_functor);
#endif
}
