//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeDynamicWeightedGapLMMechanicalContact.h"
#include "MortarContactUtils.h"
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
const InputParameters &
assignVarsInParamsDyn(const InputParameters & params_in)
{
  InputParameters & ret = const_cast<InputParameters &>(params_in);
  const auto & disp_x_name = ret.get<std::vector<VariableName>>("disp_x");
  if (disp_x_name.size() != 1)
    mooseError("We require that the disp_x parameter have exactly one coupled name");

  // We do this so we don't get any variable errors during MortarConstraint(Base) construction
  ret.set<VariableName>("secondary_variable") = disp_x_name[0];
  ret.set<VariableName>("primary_variable") = disp_x_name[0];

  return ret;
}
}
InputParameters
ComputeDynamicWeightedGapLMMechanicalContact::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
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
      "newmark_gamma", "newmark_gamma >= 0.0", "Gamma parameter for the Newmark time integrator");
  params.suppressParameter<VariableName>("secondary_variable");
  params.suppressParameter<VariableName>("primary_variable");
  params.addRequiredCoupledVar("disp_x", "The x displacement variable");
  params.addRequiredCoupledVar("disp_y", "The y displacement variable");
  params.addCoupledVar("disp_z", "The z displacement variable");
  params.addParam<Real>(
      "c", 1e6, "Parameter for balancing the size of the gap and contact pressure");
  params.addParam<bool>(
      "normalize_c",
      false,
      "Whether to normalize c by weighting function norm. When unnormalized "
      "the value of c effectively depends on element size since in the constraint we compare nodal "
      "Lagrange Multiplier values to integrated gap values (LM nodal value is independent of "
      "element size, where integrated values are dependent on element size).");
  params.set<bool>("use_displaced_mesh") = true;
  params.set<bool>("interpolate_normals") = false;
  return params;
}

ComputeDynamicWeightedGapLMMechanicalContact::ComputeDynamicWeightedGapLMMechanicalContact(
    const InputParameters & parameters)
  : ADMortarConstraint(assignVarsInParamsDyn(parameters)),
    _secondary_disp_x(adCoupledValue("disp_x")),
    _primary_disp_x(adCoupledNeighborValue("disp_x")),
    _secondary_disp_y(adCoupledValue("disp_y")),
    _primary_disp_y(adCoupledNeighborValue("disp_y")),
    _has_disp_z(isCoupled("disp_z")),
    _secondary_disp_z(_has_disp_z ? &adCoupledValue("disp_z") : nullptr),
    _primary_disp_z(_has_disp_z ? &adCoupledNeighborValue("disp_z") : nullptr),
    _c(getParam<Real>("c")),
    _normalize_c(getParam<bool>("normalize_c")),
    _nodal(getVar("disp_x", 0)->feType().family == LAGRANGE),
    _disp_x_var(getVar("disp_x", 0)),
    _disp_y_var(getVar("disp_y", 0)),
    _disp_z_var(_has_disp_z ? getVar("disp_z", 0) : nullptr),
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

  std::array<const MooseVariable *, 3> var_array{{_disp_x_var, _disp_y_var, _disp_z_var}};
  std::array<ADReal, 3> primary_disp{
      {_primary_disp_x[_qp], _primary_disp_y[_qp], _has_disp_z ? (*_primary_disp_z)[_qp] : 0}};
  std::array<ADReal, 3> secondary_disp{{_secondary_disp_x[_qp],
                                        _secondary_disp_y[_qp],
                                        _has_disp_z ? (*_secondary_disp_z)[_qp] : 0}};

  trimInteriorNodeDerivatives(primary_ip_lowerd_map, var_array, primary_disp, false);
  trimInteriorNodeDerivatives(secondary_ip_lowerd_map, var_array, secondary_disp, true);

  const ADReal & prim_x = primary_disp[0];
  const ADReal & prim_y = primary_disp[1];
  const ADReal * prim_z = nullptr;
  if (_has_disp_z)
    prim_z = &primary_disp[2];

  const ADReal & sec_x = secondary_disp[0];
  const ADReal & sec_y = secondary_disp[1];
  const ADReal * sec_z = nullptr;
  if (_has_disp_z)
    sec_z = &secondary_disp[2];

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

  // Compute dynamic constraint-related quantities
  ADRealVectorValue gap_vec = _phys_points_primary[_qp] - _phys_points_secondary[_qp];
  gap_vec(0).derivatives() = prim_x.derivatives() - sec_x.derivatives();
  gap_vec(1).derivatives() = prim_y.derivatives() - sec_y.derivatives();

  _relative_velocity = ADRealVectorValue(prim_x_dot - sec_x_dot, prim_y_dot - sec_y_dot, 0.0);

  if (_has_disp_z)
  {
    gap_vec(2).derivatives() = prim_z->derivatives() - sec_z->derivatives();
    _relative_velocity(2) = *prim_z_dot - *sec_z_dot;
  }

  _qp_gap_nodal = gap_vec * (_JxW_msm[_qp] * _coord[_qp]);
  _qp_velocity = _relative_velocity * (_JxW_msm[_qp] * _coord[_qp]);

  // Current part of the gap velocity Newmark-beta time discretization
  _qp_gap_nodal_dynamics =
      (_newmark_gamma / _newmark_beta * gap_vec / _dt) * (_JxW_msm[_qp] * _coord[_qp]);

  // To do normalization of constraint coefficient (c_n)
  _qp_factor = _JxW_msm[_qp] * _coord[_qp];
}

ADReal ComputeDynamicWeightedGapLMMechanicalContact::computeQpResidual(Moose::MortarType)
{
  mooseError(
      "We should never call computeQpResidual for ComputeDynamicWeightedGapLMMechanicalContact");
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
  _dof_to_weighted_gap.clear();
  _dof_to_weighted_gap_dynamics.clear();
  _dof_to_velocity.clear();

  // Wear
  _dof_to_nodal_wear_depth.clear();
}

void
ComputeDynamicWeightedGapLMMechanicalContact::post()
{
  Moose::Mortar::Contact::communicateGaps(
      _dof_to_weighted_gap, _mesh, _nodal, _normalize_c, _communicator, false);

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

    ComputeDynamicWeightedGapLMMechanicalContact::enforceConstraintOnDof(pr.first);
  }
}

void
ComputeDynamicWeightedGapLMMechanicalContact::incorrectEdgeDroppingPost(
    const std::unordered_set<const Node *> & inactive_lm_nodes)
{
  Moose::Mortar::Contact::communicateGaps(
      _dof_to_weighted_gap, _mesh, _nodal, _normalize_c, _communicator, false);

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
      ADReal term = _dof_to_weighted_gap[pr.first].first * _newmark_gamma / (_newmark_beta * _dt);
      term -= _dof_to_old_weighted_gap[pr.first] * _newmark_gamma / (_newmark_beta * _dt);
      term -= _dof_to_old_velocity[pr.first];
      _dof_to_weighted_gap_dynamics[pr.first] = term;
      // Enable the application of persistency condition
      _weighted_gap_ptr = &_dof_to_weighted_gap_dynamics[pr.first];
    }

    _normalization_ptr = &pr.second.second;

    ComputeDynamicWeightedGapLMMechanicalContact::enforceConstraintOnDof(pr.first);
  }
}

void
ComputeDynamicWeightedGapLMMechanicalContact::communicateWear()
{
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
}

void
ComputeDynamicWeightedGapLMMechanicalContact::enforceConstraintOnDof(const DofObject * const dof)
{
  const auto & weighted_gap = *_weighted_gap_ptr;
  const Real c = _normalize_c ? _c / *_normalization_ptr : _c;

  const auto dof_index = dof->dof_number(_sys.number(), _var->number(), 0);
  ADReal lm_value = (*_sys.currentSolution())(dof_index);
  Moose::derivInsert(lm_value.derivatives(), dof_index, 1.);

  const ADReal dof_residual = std::min(lm_value, weighted_gap * c);

  _assembly.processResidualAndJacobian(dof_residual, dof_index, _vector_tags, _matrix_tags);
}

void
ComputeDynamicWeightedGapLMMechanicalContact::computeResidual(const Moose::MortarType mortar_type)
{
  if (mortar_type != Moose::MortarType::Lower)
    return;

  mooseAssert(_var, "LM variable is null");

  for (_qp = 0; _qp < _qrule_msm->n_points(); _qp++)
  {
    computeQpProperties();
    for (_i = 0; _i < _test.size(); ++_i)
      computeQpIProperties();
  }
}

void
ComputeDynamicWeightedGapLMMechanicalContact::jacobianSetup()
{
  residualSetup();
}

void
ComputeDynamicWeightedGapLMMechanicalContact::computeJacobian(const Moose::MortarType mortar_type)
{
  // During "computeResidual" and "computeJacobian" we are actually just computing properties on the
  // mortar segment element mesh. We are *not* actually assembling into the residual/Jacobian. For
  // the zero-penetration constraint, the property of interest is the map from node to weighted gap.
  // Computation of the properties proceeds identically for residual and Jacobian evaluation hence
  // why we simply call computeResidual here. We will assemble into the residual/Jacobian later from
  // the post() method
  computeResidual(mortar_type);
}
