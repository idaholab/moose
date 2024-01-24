//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BilinearMixedModeCohesiveZoneModel.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "MortarUtils.h"
#include "MooseUtils.h"
#include "MathUtils.h"

#include "MortarContactUtils.h"
#include "FactorizedRankTwoTensor.h"

#include "ADReal.h"
#include "CohesiveZoneModelTools.h"
#include <Eigen/Core>

registerMooseObject("ContactApp", BilinearMixedModeCohesiveZoneModel);

InputParameters
BilinearMixedModeCohesiveZoneModel::validParams()
{
  InputParameters params = PenaltySimpleCohesiveZoneModel::validParams();

  params.addClassDescription("Computes the bilinear mixed mode cohesive zone model.");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");

  // Input parameters for bilinear mixed mode traction.
  params.addParam<MaterialPropertyName>("GI_c",
                                        "Critical energy release rate in normal direction.");
  params.addParam<MaterialPropertyName>("GII_c",
                                        "Critical energy release rate in shear direction.");
  params.addParam<MaterialPropertyName>("normal_strength", "Tensile strength in normal direction.");
  params.addParam<MaterialPropertyName>("shear_strength", "Tensile strength in shear direction.");
  params.addParam<Real>("power_law_parameter", "The power law parameter.");
  MooseEnum criterion("POWER_LAW BK", "BK");
  params.addParam<Real>("viscosity", 0.0, "Viscosity for damage model.");
  params.addParam<MooseEnum>(
      "mixed_mode_criterion", criterion, "Option for mixed mode propagation criterion.");
  params.addParam<bool>(
      "lag_displacement_jump",
      false,
      "Whether to use old displacement jumps to compute the effective displacement jump.");
  params.addParam<Real>(
      "regularization_alpha", 1e-10, "Regularization parameter for the Macaulay bracket.");
  params.addRangeCheckedParam<Real>(
      "penalty_stiffness", "penalty_stiffness > 0.0", "Penalty stiffness for CZM.");
  params.addParamNamesToGroup(
      "GI_c GII_c normal_strength shear_strength power_law_parameter viscosity "
      "mixed_mode_criterion lag_displacement_jump regularization_alpha "
      "penalty_stiffness",
      "Bilinear mixed mode traction");
  // End of input parameters for bilinear mixed mode traction.

  // Suppress augmented Lagrange parameters. AL implementation for CZM remains to be done.
  params.suppressParameter<Real>("max_penalty_multiplier");
  params.suppressParameter<Real>("penalty_multiplier");
  params.suppressParameter<Real>("penetration_tolerance");
  return params;
}

BilinearMixedModeCohesiveZoneModel::BilinearMixedModeCohesiveZoneModel(
    const InputParameters & parameters)
  : WeightedGapUserObject(parameters),
    PenaltyWeightedGapUserObject(parameters),
    WeightedVelocitiesUserObject(parameters),
    PenaltySimpleCohesiveZoneModel(parameters),
    _ndisp(coupledComponents("displacements")),
    _normal_strength(getMaterialProperty<Real>("normal_strength")),
    _shear_strength(getMaterialProperty<Real>("shear_strength")),
    _GI_c(getMaterialProperty<Real>("GI_c")),
    _GII_c(getMaterialProperty<Real>("GII_c")),
    _penalty_stiffness_czm(getParam<Real>("penalty_stiffness")),
    _mix_mode_criterion(getParam<MooseEnum>("mixed_mode_criterion").getEnum<MixedModeCriterion>()),
    _power_law_parameter(getParam<Real>("power_law_parameter")),
    _viscosity(getParam<Real>("viscosity")),
    _regularization_alpha(getParam<Real>("regularization_alpha"))
{
  _czm_interpolated_traction.resize(_ndisp);

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _grad_disp.push_back(&adCoupledGradient("displacements", i));
    _grad_disp_neighbor.push_back(&adCoupledGradient("displacements", i));
  }

  // Set non-intervening components to zero
  for (unsigned int i = _ndisp; i < 3; i++)
  {
    _grad_disp.push_back(&_ad_grad_zero);
    _grad_disp_neighbor.push_back(&_ad_grad_zero);
  }

  // Checks for bilinear traction input.
  if (!isParamValid("GI_c") || !isParamValid("GII_c") || !isParamValid("normal_strength") ||
      !isParamValid("shear_strength") || !isParamValid("power_law_parameter") ||
      !isParamValid("penalty_stiffness"))
    paramError("GI_c",
               "The CZM bilinear mixed mode traction parameters GI_c, GII_c, normal_strength, "
               "shear_strength, and power_law_parameter are required. Revise your input and add "
               "those parameters if you want to use the bilinear mixed mode traction model. ");
}

void
BilinearMixedModeCohesiveZoneModel::computeQpProperties()
{
  WeightedVelocitiesUserObject::computeQpProperties();

  // Called after computeQpProperties() within the same algorithmic step.

  // Compute F and R.
  const auto F = (ADRankTwoTensor::Identity() +
                  ADRankTwoTensor::initializeFromRows(
                      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]));
  const auto F_neighbor = (ADRankTwoTensor::Identity() +
                           ADRankTwoTensor::initializeFromRows((*_grad_disp_neighbor[0])[_qp],
                                                               (*_grad_disp_neighbor[1])[_qp],
                                                               (*_grad_disp_neighbor[2])[_qp]));

  // TODO in follow-on PRs: Trim interior node variable derivatives
  _F_interpolation = F * (_JxW_msm[_qp] * _coord[_qp]);
  _F_neighbor_interpolation = F_neighbor * (_JxW_msm[_qp] * _coord[_qp]);
  _normal_strength_interpolation = _normal_strength[_qp] * _JxW_msm[_qp] * _coord[_qp];
  _shear_strength_interpolation = _shear_strength[_qp] * _JxW_msm[_qp] * _coord[_qp];
  _GI_c_interpolation = _GI_c[_qp] * _JxW_msm[_qp] * _coord[_qp];
  _GII_c_interpolation = _GII_c[_qp] * _JxW_msm[_qp] * _coord[_qp];
}

void
BilinearMixedModeCohesiveZoneModel::computeQpIProperties()
{
  WeightedVelocitiesUserObject::computeQpIProperties();

  // Get the _dof_to_weighted_gap map
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));

  // TODO: Probably better to interpolate the deformation gradients.
  _dof_to_F[dof] += (*_test)[_i][_qp] * _F_interpolation;
  _dof_to_F_neighbor[dof] += (*_test)[_i][_qp] * _F_neighbor_interpolation;
  _dof_to_normal_strength[dof] += (*_test)[_i][_qp] * _normal_strength_interpolation;
  _dof_to_shear_strength[dof] += (*_test)[_i][_qp] * _shear_strength_interpolation;

  _dof_to_GI_c[dof] += (*_test)[_i][_qp] * _GI_c_interpolation;
  _dof_to_GII_c[dof] += (*_test)[_i][_qp] * _GII_c_interpolation;
}

template <class T>
T
BilinearMixedModeCohesiveZoneModel::normalizeQuantity(
    const std::unordered_map<const DofObject *, T> & map, const Node * const node)
{
  return libmesh_map_find(map, node) / _dof_to_weighted_gap[node].second;
}

void
BilinearMixedModeCohesiveZoneModel::timestepSetup()
{
  // instead we call it explicitly here
  PenaltySimpleCohesiveZoneModel::timestepSetup();

  // save off tangential traction from the last timestep
  for (auto & map_pr : _dof_to_damage)
  {
    auto & [damage, old_damage] = map_pr.second;
    old_damage = {MetaPhysicL::raw_value(damage)};
    damage = {0.0};
  }

  for (auto & [dof_object, delta_tangential_lm] : _dof_to_frictional_lagrange_multipliers)
    delta_tangential_lm.setZero();
}

void
BilinearMixedModeCohesiveZoneModel::initialize()
{
  // instead we call it explicitly here
  PenaltySimpleCohesiveZoneModel::initialize();

  // Avoid accumulating interpolation over the time step
  _dof_to_F.clear();
  _dof_to_F_neighbor.clear();
  _dof_to_mode_mixity_ratio.clear();
  _dof_to_normal_strength.clear();
  _dof_to_shear_strength.clear();
  _dof_to_GI_c.clear();
  _dof_to_GII_c.clear();
  _dof_to_delta_initial.clear();
  _dof_to_delta_final.clear();
  _dof_to_delta_max.clear();
  _dof_to_czm_traction.clear();
  _dof_to_interface_displacement_jump.clear();
  _dof_to_czm_normal_traction.clear();
  _dof_to_interface_F.clear();
  _dof_to_interface_R.clear();

  for (auto & map_pr : _dof_to_rotation_matrix)
    map_pr.second.setToIdentity();
}

void
BilinearMixedModeCohesiveZoneModel::prepareJumpKinematicQuantities()
{
  // Compute rotation matrix from secondary surface
  // Rotation matrices and local interface displacement jump.
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    // First call does not have maps available
    const bool return_boolean = _dof_to_weighted_gap.find(node) == _dof_to_weighted_gap.end();
    if (return_boolean)
      return;

    _dof_to_rotation_matrix[node] = CohesiveZoneModelTools::computeReferenceRotation<true>(
        _normals[i], _subproblem.mesh().dimension());

    // Every time we used quantities from a map we "denormalize" it from the mortar integral.
    // See normalizing member functions.
    if (_dof_to_weighted_displacements.find(node) != _dof_to_weighted_displacements.end())
      _dof_to_interface_displacement_jump[node] =
          (libmesh_map_find(_dof_to_weighted_displacements, node));
  }
}

void
BilinearMixedModeCohesiveZoneModel::computeFandR(const Node * const node)
{
  // First call does not have maps available
  const bool return_boolean = _dof_to_F.find(node) == _dof_to_F.end();
  if (return_boolean)
    return;

  const auto normalized_F = normalizeQuantity(_dof_to_F, node);
  const auto normalized_F_neighbor = normalizeQuantity(_dof_to_F_neighbor, node);

  // This 'averaging' assumption below can probably be improved upon.
  _dof_to_interface_F[node] = 0.5 * (normalized_F + normalized_F_neighbor);

  for (const auto i : make_range(3))
    for (const auto j : make_range(3))
      if (!std::isfinite(MetaPhysicL::raw_value(normalized_F(i, j))))
        throw MooseException(
            "The deformation gradient on the secondary surface is not finite in "
            "PenaltySimpleCohesiveZoneModel. MOOSE needs to cut the time step size.");

  const auto dof_to_interface_F_node = libmesh_map_find(_dof_to_interface_F, node);

  const ADFactorizedRankTwoTensor C = dof_to_interface_F_node.transpose() * dof_to_interface_F_node;
  const auto Uinv = MathUtils::sqrt(C).inverse().get();
  _dof_to_interface_R[node] = dof_to_interface_F_node * Uinv;

  // Transform interface jump according to two rotation matrices
  const auto global_interface_displacement = _dof_to_interface_displacement_jump[node];
  _dof_to_interface_displacement_jump[node] =
      (_dof_to_interface_R[node] * _dof_to_rotation_matrix[node]).transpose() *
      global_interface_displacement;
}

void
BilinearMixedModeCohesiveZoneModel::computeBilinearMixedModeTraction(const Node * const node)
{
  // First call does not have maps available
  const bool return_boolean = _dof_to_weighted_gap.find(node) == _dof_to_weighted_gap.end();
  if (return_boolean)
    return;

  computeModeMixity(node);
  computeCriticalDisplacementJump(node);
  computeFinalDisplacementJump(node);
  computeEffectiveDisplacementJump(node);
  computeDamage(node);

  // Split displacement jump into active and inactive parts
  const auto interface_displacement_jump =
      normalizeQuantity(_dof_to_interface_displacement_jump, node);

  const ADRealVectorValue delta_active(std::max(interface_displacement_jump(0), 0.0),
                                       interface_displacement_jump(1),
                                       interface_displacement_jump(2));
  const ADRealVectorValue delta_inactive(std::min(interface_displacement_jump(0), 0.0), 0.0, 0.0);

  // This traction vector is local at this point.
  _dof_to_czm_traction[node] =
      -(1.0 - _dof_to_damage[node].first) * _penalty_stiffness_czm * delta_active -
      _penalty_stiffness_czm * delta_inactive;
}

void
BilinearMixedModeCohesiveZoneModel::computeGlobalTraction(const Node * const node)
{
  // First call does not have maps available
  const bool return_boolean = _dof_to_czm_traction.find(node) == _dof_to_czm_traction.end();
  if (return_boolean)
    return;

  const auto local_traction_vector = libmesh_map_find(_dof_to_czm_traction, node);
  const auto rotation_matrix = libmesh_map_find(_dof_to_rotation_matrix, node);

  _dof_to_czm_traction[node] = rotation_matrix * local_traction_vector;
}

void
BilinearMixedModeCohesiveZoneModel::computeModeMixity(const Node * const node)
{
  const auto interface_displacement_jump =
      normalizeQuantity(_dof_to_interface_displacement_jump, node);

  if (interface_displacement_jump(0) > _epsilon_tolerance)
  {
    const auto delta_s = std::sqrt(interface_displacement_jump(1) * interface_displacement_jump(1) +
                                   interface_displacement_jump(2) * interface_displacement_jump(2) +
                                   _epsilon_tolerance);

    _dof_to_mode_mixity_ratio[node] = delta_s / interface_displacement_jump(0);
  }
  else
    _dof_to_mode_mixity_ratio[node] = 0;
}

void
BilinearMixedModeCohesiveZoneModel::computeCriticalDisplacementJump(const Node * const node)
{
  const auto interface_displacement_jump =
      normalizeQuantity(_dof_to_interface_displacement_jump, node);

  const auto mixity_ratio = libmesh_map_find(_dof_to_mode_mixity_ratio, node);

  const auto delta_normal_knot =
      normalizeQuantity(_dof_to_normal_strength, node) / _penalty_stiffness_czm;
  const auto delta_shear_knot =
      normalizeQuantity(_dof_to_shear_strength, node) / _penalty_stiffness_czm;

  _dof_to_delta_initial[node] = delta_shear_knot;

  if (interface_displacement_jump(0) > _epsilon_tolerance)
  {
    const auto delta_mixed = std::sqrt(delta_shear_knot * delta_shear_knot +
                                       Utility::pow<2>(mixity_ratio * delta_normal_knot));

    _dof_to_delta_initial[node] = delta_normal_knot * delta_shear_knot *
                                  std::sqrt(1.0 + mixity_ratio * mixity_ratio) / delta_mixed;
  }
}

void
BilinearMixedModeCohesiveZoneModel::computeFinalDisplacementJump(const Node * const node)
{
  const auto interface_displacement_jump =
      normalizeQuantity(_dof_to_interface_displacement_jump, node);

  const auto mixity_ratio = libmesh_map_find(_dof_to_mode_mixity_ratio, node);

  const auto normalized_GI_c = normalizeQuantity(_dof_to_GI_c, node);
  const auto normalized_GII_c = normalizeQuantity(_dof_to_GII_c, node);

  _dof_to_delta_final[node] =
      std::sqrt(2.0) * 2.0 * normalized_GII_c / normalizeQuantity(_dof_to_shear_strength, node);

  if (interface_displacement_jump(0) > _epsilon_tolerance)
  {
    if (_mix_mode_criterion == MixedModeCriterion::BK)
    {
      _dof_to_delta_final[node] =
          2.0 / _penalty_stiffness_czm / libmesh_map_find(_dof_to_delta_initial, node) *
          (normalized_GI_c +
           (normalized_GII_c - normalized_GI_c) *
               std::pow(mixity_ratio * mixity_ratio / (1 + mixity_ratio * mixity_ratio),
                        _power_law_parameter));
    }
    else if (_mix_mode_criterion == MixedModeCriterion::POWER_LAW)
    {
      const auto Gc_mixed =
          std::pow(1.0 / normalized_GI_c, _power_law_parameter) +
          std::pow(mixity_ratio * mixity_ratio / normalized_GII_c, _power_law_parameter);
      _dof_to_delta_final[node] = (2.0 + 2.0 * mixity_ratio * mixity_ratio) /
                                  _penalty_stiffness_czm /
                                  libmesh_map_find(_dof_to_delta_initial, node) *
                                  std::pow(Gc_mixed, -1.0 / _power_law_parameter);
    }
  }
}

void
BilinearMixedModeCohesiveZoneModel::computeEffectiveDisplacementJump(const Node * const node)
{
  const auto interface_displacement_jump =
      normalizeQuantity(_dof_to_interface_displacement_jump, node);

  const auto delta_normal_pos =
      MathUtils::regularizedHeavyside(interface_displacement_jump(0), _regularization_alpha) *
      interface_displacement_jump(0);

  _dof_to_delta_max[node] = std::sqrt(Utility::pow<2>(interface_displacement_jump(1)) +
                                      Utility::pow<2>(interface_displacement_jump(2)) +
                                      Utility::pow<2>(delta_normal_pos) + _epsilon_tolerance);
}

void
BilinearMixedModeCohesiveZoneModel::computeDamage(const Node * const node)
{
  const auto delta_max = libmesh_map_find(_dof_to_delta_max, node);
  const auto delta_initial = libmesh_map_find(_dof_to_delta_initial, node);
  const auto delta_final = libmesh_map_find(_dof_to_delta_final, node);

  if (delta_max < delta_initial)
    _dof_to_damage[node].first = 0;
  else if (delta_max > delta_final)
    _dof_to_damage[node].first = 1.0;
  else
    _dof_to_damage[node].first =
        delta_final * (delta_max - delta_initial) / delta_max / (delta_final - delta_initial);

  if (_dof_to_damage[node].first < _dof_to_damage[node].second)
    // Irreversibility
    _dof_to_damage[node].first = _dof_to_damage[node].second;

  // Viscous regularization
  _dof_to_damage[node].first =
      (_dof_to_damage[node].first + _viscosity * _dof_to_damage[node].second / _dt) /
      (_viscosity / _dt + 1.0);
}

void
BilinearMixedModeCohesiveZoneModel::reinit()
{
  // Normal contact pressure with penalty
  PenaltySimpleCohesiveZoneModel::reinit();

  for (const auto i : index_range(_czm_interpolated_traction))
  {
    _czm_interpolated_traction[i].resize(_qrule_msm->n_points());

    for (const auto qp : make_range(_qrule_msm->n_points()))
      _czm_interpolated_traction[i][qp] = 0.0;
  }

  // iterate over nodes
  for (const auto i : make_range(_test->size()))
  {
    // current node
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    // End of CZM bilinear computations
    if (_dof_to_czm_traction.find(node) == _dof_to_czm_traction.end())
      return;

    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
      for (const auto idx : index_range(_czm_interpolated_traction))
        _czm_interpolated_traction[idx][qp] += test_i[qp] * _dof_to_czm_traction[node](idx);
  }
}

void
BilinearMixedModeCohesiveZoneModel::finalize()
{
  PenaltySimpleCohesiveZoneModel::finalize();

  const bool send_data_back = !constrainedByOwner();

  Moose::Mortar::Contact::communicateR2T(
      _dof_to_F, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateR2T(
      _dof_to_F_neighbor, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_normal_strength, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_shear_strength, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_GI_c, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_GII_c, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_weighted_displacements, _subproblem.mesh(), _nodal, _communicator, send_data_back);
}

const ADVariableValue &
BilinearMixedModeCohesiveZoneModel::czmGlobalTraction(const unsigned int i) const
{
  mooseAssert(i <= 3,
              "Internal error in czmGlobalTraction. Index exceeds the traction vector size.");

  return _czm_interpolated_traction[i];
}

Real
BilinearMixedModeCohesiveZoneModel::getModeMixityRatio(const Node * const node) const
{
  const auto it = _dof_to_mode_mixity_ratio.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_mode_mixity_ratio.end())
    return MetaPhysicL::raw_value(it->second);
  else
    return 0.0;
}

Real
BilinearMixedModeCohesiveZoneModel::getCohesiveDamage(const Node * const node) const
{
  const auto it = _dof_to_damage.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_damage.end())
    return MetaPhysicL::raw_value(it->second.first);
  else
    return 0.0;
}

Real
BilinearMixedModeCohesiveZoneModel::getLocalDisplacementNormal(const Node * const node) const
{
  const auto it = _dof_to_interface_displacement_jump.find(_subproblem.mesh().nodePtr(node->id()));
  const auto it2 = _dof_to_weighted_gap.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_interface_displacement_jump.end() && it2 != _dof_to_weighted_gap.end())
    return MetaPhysicL::raw_value(it->second(0) / it2->second.second);
  else
    return 0.0;
}

Real
BilinearMixedModeCohesiveZoneModel::getLocalDisplacementTangential(const Node * const node) const
{
  const auto it = _dof_to_interface_displacement_jump.find(_subproblem.mesh().nodePtr(node->id()));
  const auto it2 = _dof_to_weighted_gap.find(_subproblem.mesh().nodePtr(node->id()));

  if (it != _dof_to_interface_displacement_jump.end() && it2 != _dof_to_weighted_gap.end())
    return MetaPhysicL::raw_value(it->second(1) / it2->second.second);
  else
    return 0.0;
}
