//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDynamicFrictionalForceLMMechanicalContact.h"
#include "DisplacedProblem.h"
#include "Assembly.h"
#include "Function.h"

#include "DualRealOps.h"

#include "metaphysicl/dualsemidynamicsparsenumberarray.h"
#include "metaphysicl/parallel_dualnumber.h"
#include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
#include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
#include "timpi/parallel_sync.h"

#include <limits>

registerMooseObject("ContactApp", ComputeDynamicFrictionalForceLMMechanicalContact);

InputParameters
ComputeDynamicFrictionalForceLMMechanicalContact::validParams()
{
  InputParameters params = ComputeDynamicWeightedGapLMMechanicalContact::validParams();
  params.addClassDescription("Computes the tangential frictional forces for dynamic simulations");
  params.addRequiredCoupledVar("friction_lm", "The frictional Lagrange's multiplier");
  params.addCoupledVar("friction_lm_dir",
                       "The frictional Lagrange's multiplier for an addtional direction.");
  params.addParam<FunctionName>(
      "function_friction",
      "Coupled function to evaluate friction with values from contact pressure and relative "
      "tangential velocities (from the previous step).");
  params.addParam<Real>("c_t", 1e0, "Numerical parameter for tangential constraints");
  params.addParam<Real>(
      "epsilon",
      1.0e-7,
      "Minimum value of contact pressure that will trigger frictional enforcement");
  params.addParam<Real>("mu", "The friction coefficient for the Coulomb friction law");
  return params;
}

ComputeDynamicFrictionalForceLMMechanicalContact::ComputeDynamicFrictionalForceLMMechanicalContact(
    const InputParameters & parameters)
  : ComputeDynamicWeightedGapLMMechanicalContact(parameters),
    _c_t(getParam<Real>("c_t")),
    _secondary_x_dot(_secondary_var.adUDot()),
    _primary_x_dot(_primary_var.adUDotNeighbor()),
    _secondary_y_dot(adCoupledDot("disp_y")),
    _primary_y_dot(adCoupledNeighborValueDot("disp_y")),
    _secondary_z_dot(_has_disp_z ? &adCoupledDot("disp_z") : nullptr),
    _primary_z_dot(_has_disp_z ? &adCoupledNeighborValueDot("disp_z") : nullptr),
    _epsilon(getParam<Real>("epsilon")),
    _mu(isParamValid("mu") ? getParam<Real>("mu") : std::numeric_limits<double>::quiet_NaN()),
    _function_friction(isParamValid("function_friction") ? &getFunction("function_friction")
                                                         : nullptr),
    _has_friction_function(isParamValid("function_friction")),
    _3d(_has_disp_z)
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError(
      "ComputeFrictionalForceLMMechanicalContact relies on use of the global indexing container "
      "in order to make its implementation feasible");
#endif

  if (!_has_friction_function && !isParamValid("mu"))
    mooseError(
        "A coefficient of friction needs to be provided as a constant value of via a function.");

  if (_has_friction_function && isParamValid("mu"))
    paramError("mu",
               "Either provide a constant coefficient of friction or a function defining the "
               "coefficient of friction. Both inputs cannot be provided simultaneously.");

  if (!getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "'use_displaced_mesh' must be true for the "
               "ComputeFrictionalForceLMMechanicalContact object");

  if (_3d && !isParamValid("friction_lm_dir"))
    paramError("friction_lm_dir",
               "Three-dimensional mortar frictional contact simulations require an additional "
               "frictional Lagrange's multiplier to enforce a second tangential pressure");

  mooseAssert(!_interpolate_normals,
              "Dynamic mortar mechanical contact constraints require the surface geometry to be "
              "attached to nodes");

  _friction_vars.push_back(getVar("friction_lm", 0));

  if (_3d)
    _friction_vars.push_back(getVar("friction_lm_dir", 0));

  if (!_friction_vars[0]->isNodal())
    if (_friction_vars[0]->feType().order != static_cast<Order>(0))
      paramError(
          "friction_lm",
          "Frictional contact constraints only support elemental variables of CONSTANT order");

  // Request the old solution state in unison
  _sys.solutionOld();
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::computeQpProperties()
{
  // Compute the value of _qp_gap
  ComputeDynamicWeightedGapLMMechanicalContact::computeQpProperties();

  // It appears that the relative velocity between weighted gap and this class have a sign
  // difference
  _qp_tangential_velocity_nodal = -_relative_velocity * (_JxW_msm[_qp] * _coord[_qp]);
  _qp_real_tangential_velocity_nodal = -_relative_velocity;
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::computeQpIProperties()
{
  // Get the _dof_to_weighted_gap map
  ComputeDynamicWeightedGapLMMechanicalContact::computeQpIProperties();

  const auto & nodal_tangents = amg().getNodalTangents(*_lower_secondary_elem);

  // Get the _dof_to_weighted_tangential_velocity map
  const DofObject * const dof =
      _friction_vars[0]->isNodal()
          ? static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i))
          : static_cast<const DofObject *>(_lower_secondary_elem);

  _dof_to_weighted_tangential_velocity[dof][0] +=
      _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[0][_i];

  _dof_to_real_tangential_velocity[dof][0] +=
      _test[_i][_qp] * MetaPhysicL::raw_value(_qp_real_tangential_velocity_nodal) *
      nodal_tangents[0][_i];

  // Get the _dof_to_weighted_tangential_velocity map for a second direction
  if (_3d)
  {
    _dof_to_weighted_tangential_velocity[dof][1] +=
        _test[_i][_qp] * _qp_tangential_velocity_nodal * nodal_tangents[1][_i];

    _dof_to_real_tangential_velocity[dof][1] +=
        _test[_i][_qp] * MetaPhysicL::raw_value(_qp_real_tangential_velocity_nodal) *
        nodal_tangents[1][_i];
  }
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::residualSetup()
{
  // Clear both maps
  ComputeDynamicWeightedGapLMMechanicalContact::residualSetup();
  _dof_to_weighted_tangential_velocity.clear();
  _dof_to_real_tangential_velocity.clear();
  _dof_to_normal_pressure.clear();
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::timestepSetup()
{

  ComputeDynamicWeightedGapLMMechanicalContact::timestepSetup();

  _dof_to_old_real_tangential_velocity.clear();
  _dof_to_old_normal_pressure.clear();

  for (auto & map_pr : _dof_to_real_tangential_velocity)
    _dof_to_old_real_tangential_velocity.emplace(map_pr);

  for (auto & map_pr : _dof_to_normal_pressure)
    _dof_to_old_normal_pressure.emplace(map_pr);
}

#ifdef MOOSE_SPARSE_AD
template <typename T>
void
ComputeDynamicFrictionalForceLMMechanicalContact::communicateVelocities(
    std::unordered_map<const DofObject *, std::array<T, 2>> & dof_map)
{

  // We may have weighted gap information that should go to other processes that own the dofs
  using Datum = std::pair<dof_id_type, std::array<T, 2>>;
  std::unordered_map<processor_id_type, std::vector<Datum>> push_data;

  for (auto & pr : dof_map)
  {
    const auto * const dof_object = pr.first;
    const auto proc_id = dof_object->processor_id();
    if (proc_id == this->processor_id())
      continue;

    push_data[proc_id].push_back(std::make_pair(dof_object->id(), std::move(pr.second)));
  }

  const auto & lm_mesh = _mesh.getMesh();

  auto action_functor = [this, &lm_mesh, &dof_map](const processor_id_type libmesh_dbg_var(pid),
                                                   const std::vector<Datum> & sent_data)
  {
    mooseAssert(pid != this->processor_id(), "We do not send messages to ourself here");
    for (auto & pr : sent_data)
    {
      const auto dof_id = pr.first;
      const auto * const dof_object =
          _nodal ? static_cast<const DofObject *>(lm_mesh.node_ptr(dof_id))
                 : static_cast<const DofObject *>(lm_mesh.elem_ptr(dof_id));
      mooseAssert(dof_object, "This dof object should be non-null");
      dof_map[dof_object][0] += std::move(pr.second[0]);
      dof_map[dof_object][1] += std::move(pr.second[1]);
    }
  };

  TIMPI::push_parallel_vector_data(_communicator, push_data, action_functor);
}
#endif

void
ComputeDynamicFrictionalForceLMMechanicalContact::post()
{
  ComputeDynamicWeightedGapLMMechanicalContact::post();

#ifdef MOOSE_SPARSE_AD
  communicateVelocities(_dof_to_weighted_tangential_velocity);

  if (_has_friction_function)
    communicateVelocities(_dof_to_real_tangential_velocity);
#endif

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    if (dof->processor_id() != this->processor_id())
      continue;

    // Use always weighted gap for dynamic PDASS. Omit the dynamic weighted gap approach that is
    // used in normal contact where the discretized gap velocity is enforced if a node has
    // identified to be into contact.

    _weighted_gap_ptr = &_dof_to_weighted_gap[dof].first;
    _normalization_ptr = &_dof_to_weighted_gap[dof].second;
    _tangential_vel_ptr[0] = &(pr.second[0]);

    if (_3d)
    {
      _tangential_vel_ptr[1] = &(pr.second[1]);
      enforceConstraintOnDof3d(dof);
    }
    else
      enforceConstraintOnDof(dof);
  }
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  ComputeDynamicWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(inactive_lm_nodes);

#ifdef MOOSE_SPARSE_AD
  communicateVelocities(_dof_to_weighted_tangential_velocity);

  if (_has_friction_function)
    communicateVelocities(_dof_to_real_tangential_velocity);
#endif

  // Enforce frictional complementarity constraints
  for (const auto & pr : _dof_to_weighted_tangential_velocity)
  {
    const DofObject * const dof = pr.first;

    // If node inactive, skip
    if ((inactive_lm_nodes.find(static_cast<const Node *>(dof)) != inactive_lm_nodes.end()) ||
        (dof->processor_id() != this->processor_id()))
      continue;

    // Use always weighted gap for dynamic PDASS
    _weighted_gap_ptr = &_dof_to_weighted_gap[dof].first;
    _normalization_ptr = &_dof_to_weighted_gap[dof].second;
    _tangential_vel_ptr[0] = &pr.second[0];

    if (_3d)
    {
      _tangential_vel_ptr[1] = &pr.second[1];
      enforceConstraintOnDof3d(dof);
    }
    else
      enforceConstraintOnDof(dof);
  }
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::enforceConstraintOnDof3d(
    const DofObject * const dof)
{
  // Get normal LM
  const auto normal_dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  const ADReal & weighted_gap = *_weighted_gap_ptr;
  ADReal contact_pressure = (*_sys.currentSolution())(normal_dof_index);
  Moose::derivInsert(contact_pressure.derivatives(), normal_dof_index, 1.);

  // Get friction LMs
  std::array<const ADReal *, 2> & tangential_vel = _tangential_vel_ptr;
  std::array<dof_id_type, 2> friction_dof_indices;
  std::array<ADReal, 2> friction_lm_values;

  const unsigned int num_tangents = 2;
  for (const auto i : make_range(num_tangents))
  {
    friction_dof_indices[i] = dof->dof_number(_sys.number(), _friction_vars[i]->number(), 0);
    friction_lm_values[i] = (*_sys.currentSolution())(friction_dof_indices[i]);
    Moose::derivInsert(friction_lm_values[i].derivatives(), friction_dof_indices[i], 1.);
  }
  // Normal pressure for friction model
  _dof_to_normal_pressure[dof] = MetaPhysicL::raw_value(contact_pressure);

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  // Compute the friction coefficient (constant or function)
  ADReal mu_ad = computeFrictionValue(_dof_to_old_normal_pressure[dof],
                                      _dof_to_old_real_tangential_velocity[dof][0],
                                      _dof_to_old_real_tangential_velocity[dof][1]);

  ADReal dof_residual;
  ADReal dof_residual_dir;

  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
  {
    dof_residual = friction_lm_values[0];
    dof_residual_dir = friction_lm_values[1];
  }
  else
  {
    const Real epsilon_sqrt = 1.0e-48;

    const auto lamdba_plus_cg = contact_pressure + c * weighted_gap;
    std::array<ADReal, 2> lambda_t_plus_ctu;
    lambda_t_plus_ctu[0] = friction_lm_values[0] + c_t * *tangential_vel[0] * _dt;
    lambda_t_plus_ctu[1] = friction_lm_values[1] + c_t * *tangential_vel[1] * _dt;

    const auto term_1_x =
        std::max(mu_ad * lamdba_plus_cg,
                 std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                           lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
        friction_lm_values[0];

    const auto term_1_y =
        std::max(mu_ad * lamdba_plus_cg,
                 std::sqrt(lambda_t_plus_ctu[0] * lambda_t_plus_ctu[0] +
                           lambda_t_plus_ctu[1] * lambda_t_plus_ctu[1] + epsilon_sqrt)) *
        friction_lm_values[1];

    const auto term_2_x = mu_ad * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[0];

    const auto term_2_y = mu_ad * std::max(0.0, lamdba_plus_cg) * lambda_t_plus_ctu[1];

    dof_residual = term_1_x - term_2_x;
    dof_residual_dir = term_1_y - term_2_y;
  }

#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_subproblem.currentlyComputingJacobian())
  {
    _assembly.processUnconstrainedDerivatives({dof_residual, dof_residual_dir},
                                              {friction_dof_indices[0], friction_dof_indices[1]},
                                              _matrix_tags);
  }
  else
  {
    _assembly.processResidual(dof_residual.value(), friction_dof_indices[0], _vector_tags);
    _assembly.processResidual(dof_residual_dir.value(), friction_dof_indices[1], _vector_tags);
  }
#endif
}

void
ComputeDynamicFrictionalForceLMMechanicalContact::enforceConstraintOnDof(
    const DofObject * const dof)
{
  // Get friction LM
  const auto friction_dof_index = dof->dof_number(_sys.number(), _friction_vars[0]->number(), 0);
  const ADReal & tangential_vel = *_tangential_vel_ptr[0];
  ADReal friction_lm_value = (*_sys.currentSolution())(friction_dof_index);
  Moose::derivInsert(friction_lm_value.derivatives(), friction_dof_index, 1.);

  // Get normal LM
  const auto normal_dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  const ADReal & weighted_gap = *_weighted_gap_ptr;
  ADReal contact_pressure = (*_sys.currentSolution())(normal_dof_index);
  Moose::derivInsert(contact_pressure.derivatives(), normal_dof_index, 1.);

  // Normal pressure for friction model
  _dof_to_normal_pressure[dof] = MetaPhysicL::raw_value(contact_pressure);

  // Get normalized c and c_t values (if normalization specified
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;
  const Real c_t = _normalize_c ? _c_t / *_normalization_ptr : _c_t;

  // Compute the friction coefficient (constant or function)
  ADReal mu_ad = computeFrictionValue(
      _dof_to_old_normal_pressure[dof], _dof_to_old_real_tangential_velocity[dof][0], 0.0);

  ADReal dof_residual;
  // Primal-dual active set strategy (PDASS)
  if (contact_pressure < _epsilon)
    dof_residual = friction_lm_value;
  else
  {
    const auto term_1 = std::max(mu_ad * (contact_pressure + c * weighted_gap),
                                 std::abs(friction_lm_value + c_t * tangential_vel * _dt)) *
                        friction_lm_value;
    const auto term_2 = mu_ad * std::max(0.0, contact_pressure + c * weighted_gap) *
                        (friction_lm_value + c_t * tangential_vel * _dt);

    dof_residual = term_1 - term_2;
  }

#ifdef MOOSE_GLOBAL_AD_INDEXING
  if (_subproblem.currentlyComputingJacobian())
    _assembly.processUnconstrainedDerivatives({dof_residual}, {friction_dof_index}, _matrix_tags);
  else
    _assembly.processResidual(dof_residual.value(), friction_dof_index, _vector_tags);
#endif
}

ADReal
ComputeDynamicFrictionalForceLMMechanicalContact::computeFrictionValue(
    const ADReal & contact_pressure, const Real & tangential_vel, const Real & tangential_vel_dir)
{
  // TODO: Introduce temperature dependence in the function. Do this when we have an example.
  ADReal mu_ad;

  if (!_has_friction_function)
    mu_ad = _mu;
  else
  {
    ADReal tangential_vel_magnitude = std::sqrt(tangential_vel * tangential_vel +
                                                tangential_vel_dir * tangential_vel_dir + 1.0e-24);

    mu_ad = _function_friction->value<ADReal>(0.0, contact_pressure, tangential_vel_magnitude, 0.0);
  }

  return mu_ad;
}
