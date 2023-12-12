//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PenaltySimpleCohesiveZoneModel.h"
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

registerMooseObject("ContactApp", PenaltySimpleCohesiveZoneModel);

InputParameters
PenaltySimpleCohesiveZoneModel::validParams()
{
  InputParameters params = WeightedVelocitiesUserObject::validParams();
  params += PenaltyWeightedGapUserObject::validParams();

  params.addClassDescription(
      "Computes the mortar frictional contact force via a penalty approach.");
  params.addParam<Real>("penalty_friction",
                        "The penalty factor for frictional interaction. If not provide, the normal "
                        "penalty factor is also used for the frictional problem.");
  params.addRequiredParam<Real>("friction_coefficient",
                                "The friction coefficient ruling Coulomb friction equations.");
  params.addRangeCheckedParam<Real>(
      "slip_tolerance",
      "slip_tolerance > 0",
      "Acceptable slip distance at which augmented Lagrange iterations can be stopped");
  MooseEnum adaptivity_penalty_friction("SIMPLE FRICTION_LIMIT", "FRICTION_LIMIT");
  adaptivity_penalty_friction.addDocumentation(
      "SIMPLE", "Keep multiplying by the frictional penalty multiplier between AL iterations");
  adaptivity_penalty_friction.addDocumentation(
      "FRICTION_LIMIT",
      "This strategy will be guided by the Coulomb limit and be less reliant on the initial "
      "penalty factor provided by the user.");
  params.addParam<MooseEnum>(
      "adaptivity_penalty_friction",
      adaptivity_penalty_friction,
      "The augmented Lagrange update strategy used on the frictional penalty coefficient.");
  params.addRangeCheckedParam<Real>(
      "penalty_multiplier_friction",
      1.0,
      "penalty_multiplier_friction > 0",
      "The penalty growth factor between augmented Lagrange "
      "iterations for penalizing relative slip distance if the node is under stick conditions.");

  // Cohesive Zone Model parameters
  params.addRequiredParam<Real>("czm_normal_stiffness",
                                "The normal stiffness that determines the traction that initiates "
                                "the cohesive zone traction.");
  params.addRequiredParam<Real>(
      "czm_tangential_stiffness",
      "The tangential stiffness that determines the traction that initiates "
      "the cohesive zone traction.");
  params.addRequiredParam<Real>("czm_normal_strength",
                                "The normal strength that determines the traction-separation law.");
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addRequiredParam<Real>(
      "czm_tangential_strength",
      "The tangential strength that determines the traction-separation law.");

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

  // Advanced parameters to improve numerical convergence
  // Never lag
  // params.addParam<bool>(
  //     "lag_mode_mixity", true, "Whether to use old displacement jumps to compute the mode
  //     mixity.");

  params.addParam<bool>(
      "lag_displacement_jump",
      false,
      "Whether to use old displacement jumps to compute the effective displacement jump.");
  params.addParam<Real>(
      "regularization_alpha", 1e-10, "Regularization parameter for the Macaulay bracket.");
  params.addParam<bool>(
      "use_bilinear_mixed_mode_traction", false, "Whether to bilinear mixed mode traction.");
  params.addRangeCheckedParam<Real>(
      "penalty_stiffness", "penalty_stiffness > 0.0", "Penalty stiffness for CZM.");
  params.addParamNamesToGroup(
      "GI_c GII_c normal_strength shear_strength power_law_parameter viscosity "
      "mixed_mode_criterion lag_displacement_jump regularization_alpha "
      "use_bilinear_mixed_mode_traction penalty_stiffness",
      "Bilinear mixed mode traction");
  // End of input parameters for bilinear mixed mode traction.

  return params;
}

PenaltySimpleCohesiveZoneModel::PenaltySimpleCohesiveZoneModel(const InputParameters & parameters)
  /*
   * We are using virtual inheritance to avoid the "Diamond inheritance" problem. This means that
   * that we have to construct WeightedGapUserObject explicitly as it will _not_ be constructed in
   * the intermediate base classes PenaltyWeightedGapUserObject and WeightedVelocitiesUserObject.
   * Virtual inheritance ensures that only one instance of WeightedGapUserObject is included in this
   * class. The inheritance diagram is as follows:
   *
   * WeightedGapUserObject         <-----   PenaltyWeightedGapUserObject
   *          ^                                         ^
   *          |                                         |
   * WeightedVelocitiesUserObject  <-----   PenaltySimpleCohesiveZoneModel
   *
   */
  : WeightedGapUserObject(parameters),
    PenaltyWeightedGapUserObject(parameters),
    WeightedVelocitiesUserObject(parameters),
    _penalty(getParam<Real>("penalty")),
    _penalty_friction(isParamValid("penalty_friction") ? getParam<Real>("penalty_friction")
                                                       : getParam<Real>("penalty")),
    _slip_tolerance(isParamValid("slip_tolerance") ? getParam<Real>("slip_tolerance") : 0.0),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _penalty_multiplier_friction(getParam<Real>("penalty_multiplier_friction")),
    _adaptivity_friction(
        getParam<MooseEnum>("adaptivity_penalty_friction").getEnum<AdaptivityFrictionalPenalty>()),
    _czm_normal_stiffness(getParam<Real>("czm_normal_stiffness")),
    _czm_tangential_stiffness(getParam<Real>("czm_tangential_stiffness")),
    _czm_normal_strength(getParam<Real>("czm_normal_strength")),
    _czm_tangential_strength(getParam<Real>("czm_tangential_strength")),
    // _stress(getADMaterialProperty<RankTwoTensor>("stress")),
    _ndisp(coupledComponents("displacements")),
    _use_bilinear_mixed_mode_traction(getParam<bool>("use_bilinear_mixed_mode_traction")),
    _normal_strength(getMaterialProperty<Real>("normal_strength")),
    _shear_strength(getMaterialProperty<Real>("shear_strength")),
    _GI_c(getMaterialProperty<Real>("GI_c")),
    _GII_c(getMaterialProperty<Real>("GII_c")),
    _penalty_stiffness_czm(_use_bilinear_mixed_mode_traction ? getParam<Real>("penalty_stiffness")
                                                             : 0.0),
    _mix_mode_criterion(getParam<MooseEnum>("mixed_mode_criterion").getEnum<MixedModeCriterion>()),
    _power_law_parameter(getParam<Real>("power_law_parameter")),
    _viscosity(getParam<Real>("viscosity")),
    _regularization_alpha(getParam<Real>("regularization_alpha")),
    _epsilon_tolerance(1.0e-40)

{
  if (_augmented_lagrange_problem)
    mooseError("PenaltySimpleCohesiveZoneModel constraints cannot be enforced with an augmented "
               "Lagrange approach.");

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _grad_disp.push_back(&adCoupledGradient("displacements", i));
    _grad_disp_neighbor.push_back(&adCoupledGradient("displacements", i));
  }

  // Set non-intervening components to zero
  for (unsigned int i = _ndisp; i < 3; i++)
  {
    _grad_disp.push_back(&adZeroGradient());
    _grad_disp_neighbor.push_back(&adZeroGradient());
  }

  // Checks for bilinear traction input.
  if (_use_bilinear_mixed_mode_traction)
  {
    if (!isParamValid("GI_c") || !isParamValid("GII_c") || !isParamValid("normal_strength") ||
        !isParamValid("shear_strength") || !isParamValid("power_law_parameter") ||
        !isParamValid("penalty_stiffness"))
      paramError("GI_c",
                 "The CZM bilinear mixed mode traction parameters GI_c, GII_c, normal_strength, "
                 "shear_strength, and power_law_parameter are required. Revise your input and add "
                 "those parameters if you want to use the bilinear mixed mode traction model. ");
  }
}

void
PenaltySimpleCohesiveZoneModel::computeQpPropertiesLocal()
{
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
PenaltySimpleCohesiveZoneModel::computeQpIPropertiesLocal()
{
  // Get the _dof_to_weighted_gap map
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));
  // _F_interpolation.zero();
  // TODO: Probably better to interpolate the deformation gradients.
  _dof_to_F[dof] += (*_test)[_i][_qp] * _F_interpolation;
  _dof_to_F_neighbor[dof] += (*_test)[_i][_qp] * _F_neighbor_interpolation;
  _dof_to_normal_strength[dof] += (*_test)[_i][_qp] * _normal_strength_interpolation;
  _dof_to_shear_strength[dof] += (*_test)[_i][_qp] * _shear_strength_interpolation;

  _dof_to_GI_c[dof] += (*_test)[_i][_qp] * _GI_c_interpolation;
  _dof_to_GII_c[dof] += (*_test)[_i][_qp] * _GII_c_interpolation;
}

ADRankTwoTensor
PenaltySimpleCohesiveZoneModel::normalizeRankTwoTensorQuantity(
    const std::unordered_map<const DofObject *, ADRankTwoTensor> & map, const Node * const node)
{
  return libmesh_map_find(map, node) / _dof_to_weighted_gap[node].second;
}

ADReal
PenaltySimpleCohesiveZoneModel::normalizeRealQuantity(
    const std::unordered_map<const DofObject *, ADReal> & map, const Node * const node)
{
  return libmesh_map_find(map, node) / _dof_to_weighted_gap[node].second;
}

const VariableTestValue &
PenaltySimpleCohesiveZoneModel::test() const
{
  return _aux_lm_var ? _aux_lm_var->phiLower() : _disp_x_var->phiLower();
}

void
PenaltySimpleCohesiveZoneModel::timestepSetup()
{
  // these functions do not call WeightedGapUserObject::timestepSetup to avoid double initialization
  WeightedVelocitiesUserObject::selfTimestepSetup();
  PenaltyWeightedGapUserObject::selfTimestepSetup();

  // instead we call it explicitly here
  WeightedGapUserObject::timestepSetup();

  // Beginning of CZM properties
  for (auto & map_pr : _dof_to_czm_normal_traction)
    map_pr.second = {0.0};

  for (auto & map_pr : _dof_to_rotation_matrix)
    map_pr.second.setToIdentity();

  for (auto & map_pr : _dof_to_interface_displacement_jump)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_interface_F)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_interface_R)
    map_pr.second = 0.0;

  // End of CZM properties

  // Clear step slip (values used in between AL iterations for penalty adaptivity)
  for (auto & map_pr : _dof_to_step_slip)
  {
    auto & [step_slip, old_step_slip] = map_pr.second;
    old_step_slip = {0.0, 0.0};
    step_slip = {0.0, 0.0};
  }

  // save off accumulated slip from the last timestep
  for (auto & map_pr : _dof_to_accumulated_slip)
  {
    auto & [accumulated_slip, old_accumulated_slip] = map_pr.second;
    old_accumulated_slip = accumulated_slip;
  }

  for (auto & dof_lp : _dof_to_local_penalty_friction)
    dof_lp.second = _penalty_friction;

  // save off tangential traction from the last timestep
  for (auto & map_pr : _dof_to_tangential_traction)
  {
    auto & [tangential_traction, old_tangential_traction] = map_pr.second;
    old_tangential_traction = {MetaPhysicL::raw_value(tangential_traction(0)),
                               MetaPhysicL::raw_value(tangential_traction(1))};
    tangential_traction = {0.0, 0.0};
  }

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
PenaltySimpleCohesiveZoneModel::initialize()
{
  // these functions do not call WeightedGapUserObject::initialize to avoid double initialization
  WeightedVelocitiesUserObject::selfInitialize();
  PenaltyWeightedGapUserObject::selfInitialize();

  // instead we call it explicitly here
  WeightedGapUserObject::initialize();

  // Avoid accumulating interpolation over the time step
  for (auto & map_pr : _dof_to_F)
    map_pr.second.zero();

  for (auto & map_pr : _dof_to_F_neighbor)
    map_pr.second.zero();

  for (auto & map_pr : _dof_to_mode_mixity_ratio)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_normal_strength)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_shear_strength)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_GI_c)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_GII_c)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_delta_initial)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_delta_final)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_delta_medium)
    map_pr.second = 0.0;

  for (auto & map_pr : _dof_to_damage)
    map_pr.second.second = 0.0;

  for (auto & map_pr : _dof_to_czm_traction)
    map_pr.second = {0.0, 0.0, 0.0};

  _normal_strength_interpolation = 0.0;
  _shear_strength_interpolation = 0.0;
  _GI_c_interpolation = 0.0;
  _GII_c_interpolation = 0.0;
}

void
PenaltySimpleCohesiveZoneModel::prepareJumpKinematicQuantities()
{
  // Compute rotation matrix from secondary surface
  // Rotation matrices and local interface displacement jump.
  for (const auto i : make_range(_test->size()))
  {
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    _dof_to_rotation_matrix[node] = CohesiveZoneModelTools::computeReferenceRotationTempl<true>(
        _normals[i], _subproblem.mesh().dimension());

    _dof_to_interface_displacement_jump[node] =
        _dof_to_rotation_matrix[node].transpose() *
        (adPhysicalGap(libmesh_map_find(_dof_to_weighted_gap, node)) * _normals[i]);
  }
}

void
PenaltySimpleCohesiveZoneModel::computeFandR(const Node * const node)
{
  const auto normalized_F = normalizeRankTwoTensorQuantity(_dof_to_F, node);
  const auto normalized_F_neighbor = normalizeRankTwoTensorQuantity(_dof_to_F_neighbor, node);

  // This 'averaging' assumption below can probably be improved upon.
  _dof_to_interface_F[node] = 0.5 * (normalized_F + normalized_F_neighbor);

  for (const auto i : make_range(3))
    for (const auto j : make_range(3))
      if (!std::isfinite(MetaPhysicL::raw_value(normalized_F(i, j))))
        throw MooseException(
            "The deformation gradient on the secondary surface is not finite in "
            "PenaltySimpleCohesiveZoneModel. MOOSE needs to cut the time step size.");

  const ADFactorizedRankTwoTensor C =
      _dof_to_interface_F[node].transpose() * _dof_to_interface_F[node];
  const auto Uinv = MathUtils::sqrt(C).inverse().get();
  _dof_to_interface_R[node] = _dof_to_interface_F[node] * Uinv;
}

void
PenaltySimpleCohesiveZoneModel::computeBilinearMixedModeTraction(const Node * const node)
{
  computeModeMixity(node);                // Done
  computeCriticalDisplacementJump(node);  // Done
  computeFinalDisplacementJump(node);     // Done
  computeEffectiveDisplacementJump(node); // Done
  computeDamage(node);                    // Done

  // Split displacement jump into active and inactive parts
  const auto interface_displacement_jump =
      libmesh_map_find(_dof_to_interface_displacement_jump, node);

  const ADRealVectorValue delta_active(std::max(interface_displacement_jump(0), 0.0),
                                       interface_displacement_jump(1),
                                       interface_displacement_jump(2));
  const ADRealVectorValue delta_inactive(std::min(interface_displacement_jump(0), 0.0), 0.0, 0.0);

  // This traction vector is local at this point.
  _dof_to_czm_traction[node] =
      (1.0 - _dof_to_damage[node].first) * _penalty_stiffness_czm * delta_active +
      _penalty_stiffness_czm * delta_inactive;
}

void
PenaltySimpleCohesiveZoneModel::computeGlobalTraction(const Node * const node)
{

  const auto local_traction_vector = libmesh_map_find(_dof_to_czm_traction, node);
  const auto rotation_matrix = libmesh_map_find(_dof_to_rotation_matrix, node);

  _dof_to_czm_traction[node] = rotation_matrix * local_traction_vector;
}

void
PenaltySimpleCohesiveZoneModel::computeModeMixity(const Node * const node)
{
  const auto interface_displacement_jump =
      libmesh_map_find(_dof_to_interface_displacement_jump, node);

  if (std::abs(libmesh_map_find(_dof_to_interface_displacement_jump, node)(0)) > _epsilon_tolerance)
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
PenaltySimpleCohesiveZoneModel::computeCriticalDisplacementJump(const Node * const node)
{
  const auto interface_displacement_jump =
      libmesh_map_find(_dof_to_interface_displacement_jump, node);

  const auto delta_normal_knot =
      normalizeRealQuantity(_dof_to_normal_strength, node) / _penalty_stiffness_czm;
  const auto delta_shear_knot =
      normalizeRealQuantity(_dof_to_shear_strength, node) / _penalty_stiffness_czm;

  _dof_to_delta_initial[node] = delta_shear_knot;

  if (std::abs(interface_displacement_jump(0)) > _epsilon_tolerance)
  {
    const auto delta_mixed =
        std::sqrt(delta_shear_knot * delta_shear_knot +
                  Utility::pow<2>(_dof_to_mode_mixity_ratio[node] * delta_normal_knot));

    _dof_to_delta_initial[node] =
        delta_normal_knot * delta_shear_knot *
        std::sqrt(1.0 + _dof_to_mode_mixity_ratio[node] * _dof_to_mode_mixity_ratio[node]) /
        delta_mixed;
  }
}

void
PenaltySimpleCohesiveZoneModel::computeFinalDisplacementJump(const Node * const node)
{
  const auto interface_displacement_jump =
      libmesh_map_find(_dof_to_interface_displacement_jump, node);

  const auto normalized_GI_c = normalizeRealQuantity(_dof_to_GI_c, node);
  const auto normalized_GII_c = normalizeRealQuantity(_dof_to_GII_c, node);

  _dof_to_delta_final[node] =
      std::sqrt(2.0) * 2.0 * normalized_GII_c / normalizeRealQuantity(_dof_to_shear_strength, node);

  if (interface_displacement_jump(0) > _epsilon_tolerance)
  {
    if (_mix_mode_criterion == MixedModeCriterion::BK)
    {
      _dof_to_delta_final[node] =
          2.0 / _penalty_stiffness_czm / libmesh_map_find(_dof_to_delta_initial, node) *
          (normalized_GI_c +
           (normalized_GII_c - normalized_GI_c) *
               std::pow(libmesh_map_find(_dof_to_mode_mixity_ratio, node) *
                            libmesh_map_find(_dof_to_mode_mixity_ratio, node) /
                            (1 + libmesh_map_find(_dof_to_mode_mixity_ratio, node) *
                                     libmesh_map_find(_dof_to_mode_mixity_ratio, node)),
                        _power_law_parameter));
    }
    else if (_mix_mode_criterion == MixedModeCriterion::POWER_LAW)
    {
      const auto Gc_mixed = std::pow(1.0 / normalized_GI_c, _power_law_parameter) +
                            std::pow(_dof_to_mode_mixity_ratio[node] *
                                         _dof_to_mode_mixity_ratio[node] / normalized_GII_c,
                                     _power_law_parameter);
      _dof_to_delta_final[node] =
          (2.0 + 2.0 * _dof_to_mode_mixity_ratio[node] * _dof_to_mode_mixity_ratio[node]) /
          _penalty_stiffness_czm / libmesh_map_find(_dof_to_delta_initial, node) *
          std::pow(Gc_mixed, -1.0 / _power_law_parameter);
    }
  }
}

void
PenaltySimpleCohesiveZoneModel::computeEffectiveDisplacementJump(const Node * const node)
{
  const auto interface_displacement_jump =
      libmesh_map_find(_dof_to_interface_displacement_jump, node);

  const auto delta_normal_pos =
      MathUtils::regularizedHeavyside(interface_displacement_jump(0), _regularization_alpha) *
      interface_displacement_jump(0);

  _dof_to_delta_medium[node] = std::sqrt(Utility::pow<2>(interface_displacement_jump(1)) +
                                         Utility::pow<2>(interface_displacement_jump(2)) +
                                         Utility::pow<2>(delta_normal_pos) + _epsilon_tolerance);
}

void
PenaltySimpleCohesiveZoneModel::computeDamage(const Node * const node)
{
  if (_dof_to_delta_medium[node] < _dof_to_delta_initial[node])
    _dof_to_damage[node].first = 0;
  else if (_dof_to_delta_medium[node] > _dof_to_delta_final[node])
    _dof_to_damage[node].first = 1.0;
  else
    _dof_to_damage[node].first =
        _dof_to_delta_final[node] * (_dof_to_delta_medium[node] - _dof_to_delta_initial[node]) /
        _dof_to_delta_medium[node] / (_dof_to_delta_final[node] - _dof_to_delta_initial[node]);

  if (_dof_to_damage[node].first < _dof_to_damage[node].second)
    // Irreversibility
    _dof_to_damage[node].first = _dof_to_damage[node].second;

  // Viscous regularization
  _dof_to_damage[node].first =
      (_dof_to_damage[node].first + _viscosity * _dof_to_damage[node].second / _dt) /
      (_viscosity / _dt + 1.0);
}

void
PenaltySimpleCohesiveZoneModel::reinit()
{
  // Normal contact pressure with penalty
  PenaltyWeightedGapUserObject::reinit();

  // Compute all rotations that were created as material properties in CZMComputeDisplacementJump
  if (_use_bilinear_mixed_mode_traction)
    prepareJumpKinematicQuantities();

  // Reset frictional pressure
  _frictional_contact_traction_one.resize(_qrule_msm->n_points());
  _frictional_contact_traction_two.resize(_qrule_msm->n_points()); // 3D

  _czm_interpolated_traction_x.resize(_qrule_msm->n_points());
  _czm_interpolated_traction_y.resize(_qrule_msm->n_points());
  _czm_interpolated_traction_z.resize(_qrule_msm->n_points());

  for (const auto qp : make_range(_qrule_msm->n_points()))
  {
    _frictional_contact_traction_one[qp] = 0.0;
    _frictional_contact_traction_two[qp] = 0.0;
    _czm_interpolated_traction_x[qp] = 0.0;
    _czm_interpolated_traction_y[qp] = 0.0;
    _czm_interpolated_traction_z[qp] = 0.0;
  }

  // zero vector
  const static TwoVector zero{0.0, 0.0};

  // iterate over nodes
  for (const auto i : make_range(_test->size()))
  {
    // current node
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    if (_use_bilinear_mixed_mode_traction)
    {
      // Compute the weighted nodal deformation gradient and rotation tensors.
      // *** TODO: Perform parallel communication ***
      computeFandR(node);

      // The call below is a 'macro' call. Create a utility function or user object for it.
      // *** TODO: Perform parallel communication ***
      computeBilinearMixedModeTraction(node);

      // Build final traction vector
      computeGlobalTraction(node);
    }

    // End of CZM bilinear computations

    // Compute mechanical contact until end of method.
    const auto penalty_friction = findValue(
        _dof_to_local_penalty_friction, static_cast<const DofObject *>(node), _penalty_friction);

    // utilized quantities
    const auto & normal_pressure = _dof_to_normal_pressure[node];

    // map the tangential traction and accumulated slip
    auto & [tangential_traction, old_tangential_traction] = _dof_to_tangential_traction[node];
    auto & [accumulated_slip, old_accumulated_slip] = _dof_to_accumulated_slip[node];

    Real normal_lm = -1;
    if (_dof_to_lagrange_multiplier.find(node) != _dof_to_lagrange_multiplier.end())
      normal_lm = libmesh_map_find(_dof_to_lagrange_multiplier, node);

    // Keep active set fixed from second Uzawa loop
    if (normal_lm < -TOLERANCE && normal_pressure > TOLERANCE)
    {
      using namespace std;

      const auto & real_tangential_velocity =
          libmesh_map_find(_dof_to_real_tangential_velocity, node);
      const ADTwoVector slip_distance = {real_tangential_velocity[0] * _dt,
                                         real_tangential_velocity[1] * _dt};

      // frictional lagrange multiplier (delta lambda^(k)_T)
      const auto & tangential_lm =
          _augmented_lagrange_problem ? _dof_to_frictional_lagrange_multipliers[node] : zero;

      // tangential trial traction (Simo 3.12)
      // Modified for implementation in MOOSE: Avoid pingponging on frictional sign (max. 0.4
      // capacity)
      ADTwoVector inner_iteration_penalty_friction = penalty_friction * slip_distance;

      const auto slip_metric = std::abs(MetaPhysicL::raw_value(slip_distance).cwiseAbs()(0)) +
                               std::abs(MetaPhysicL::raw_value(slip_distance).cwiseAbs()(1));

      if (slip_metric > _epsilon_tolerance &&
          penalty_friction * slip_distance.norm() >
              0.4 * _friction_coefficient * std::abs(normal_pressure))
      {
        inner_iteration_penalty_friction =
            MetaPhysicL::raw_value(0.4 * _friction_coefficient * std::abs(normal_pressure) /
                                   (penalty_friction * slip_distance.norm())) *
            penalty_friction * slip_distance;
      }

      ADTwoVector tangential_trial_traction =
          old_tangential_traction + tangential_lm + inner_iteration_penalty_friction;

      // Nonlinearity below
      ADReal tangential_trial_traction_norm = tangential_trial_traction.norm();
      ADReal phi_trial = tangential_trial_traction_norm - _friction_coefficient * normal_pressure;
      tangential_traction = tangential_trial_traction;

      // Simo considers this a 'return mapping'; we are just capping friction to the Coulomb limit.
      if (phi_trial > 0.0)
        // Simo 3.13/3.14 (the penalty formulation has an error in the paper)
        tangential_traction -=
            phi_trial * tangential_trial_traction / tangential_trial_traction_norm;

      // track accumulated slip for output purposes
      accumulated_slip = old_accumulated_slip + MetaPhysicL::raw_value(slip_distance).cwiseAbs();

      // Keep track of slip vector for adaptive penalty
      auto & [step_slip, old_step_slip] = _dof_to_step_slip[node];
      step_slip = MetaPhysicL::raw_value(slip_distance);
    }
    else
    {
      // reset slip and clear traction
      accumulated_slip.setZero();
      tangential_traction.setZero();
    }

    applyTractionSeparationLaw(node);

    // Now that we have consistent nodal frictional values, create an interpolated frictional
    // pressure variable.
    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
    {
      _frictional_contact_traction_one[qp] += test_i[qp] * tangential_traction(0);
      _frictional_contact_traction_two[qp] += test_i[qp] * tangential_traction(1);

      // Generate no tangential traction for CZM initially
      _frictional_contact_traction_one[qp] = 0.0;
      _frictional_contact_traction_two[qp] = 0.0;

      _czm_interpolated_traction_x[qp] += test_i[qp] * _dof_to_czm_traction[node](0);
      _czm_interpolated_traction_y[qp] += test_i[qp] * _dof_to_czm_traction[node](1);
      if (_ndisp == 3)
        _czm_interpolated_traction_z[qp] += test_i[qp] * _dof_to_czm_traction[node](2);

      // _contact_pressure[qp] += 0.0; // test_i[qp] * _dof_to_czm_normal_traction[node];
      _contact_pressure[qp] += test_i[qp] * _dof_to_czm_normal_traction[node];
    }
  }
}

void
PenaltySimpleCohesiveZoneModel::applyTractionSeparationLaw(const Node * const node)
{
  // Jump corresponding to maximum force generated in CZM
  const Real peak_displacement_jump = 0.2;

  // Jump corresponding to separation in CZM
  const Real separation_displacement_jump = 0.5;

  // If _use_physical_gap is true we "normalize" the penalty parameter with the surrounding area.
  auto gap = adPhysicalGap(libmesh_map_find(_dof_to_weighted_gap, node)) /
             libmesh_map_find(_dof_to_weighted_gap, node).second;

  if (gap < peak_displacement_jump && gap >= 0)
    _dof_to_czm_normal_traction[node] = _czm_normal_stiffness * gap;
  else if (gap >= peak_displacement_jump && gap <= separation_displacement_jump)
    _dof_to_czm_normal_traction[node] =
        _czm_normal_stiffness * (separation_displacement_jump - gap) * peak_displacement_jump /
        (separation_displacement_jump - peak_displacement_jump);
  else if (gap > separation_displacement_jump)
    _dof_to_czm_normal_traction[node] = 0.0;
  else
    _dof_to_czm_normal_traction[node] = _dof_to_normal_pressure[node];
}

void
PenaltySimpleCohesiveZoneModel::finalize()
{
  WeightedVelocitiesUserObject::finalize();
  PenaltyWeightedGapUserObject::selfFinalize();
}

const ADVariableValue &
PenaltySimpleCohesiveZoneModel::contactTangentialPressureDirOne() const
{
  return _frictional_contact_traction_one;
}

const ADVariableValue &
PenaltySimpleCohesiveZoneModel::contactTangentialPressureDirTwo() const
{
  return _frictional_contact_traction_two;
}

const ADVariableValue &
PenaltySimpleCohesiveZoneModel::czmGlobalTraction(const unsigned int i) const
{
  if (i > 3 && i < 0)
    mooseError("Internal error in czmGlobalTraction.");

  if (i == 0)
    return _czm_interpolated_traction_x;
  else if (i == 1)
    return _czm_interpolated_traction_y;
  else
    return _czm_interpolated_traction_z;
}
