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
  // Cohesive Zone Model parameters
  params.addRequiredParam<Real>("czm_normal_stiffness",
                                "The normal stiffness that determines the traction that initiates "
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
    _czm_normal_stiffness(getParam<Real>("czm_normal_stiffness")),
    _penalty(getParam<Real>("penalty")),
    _penalty_friction(isParamValid("penalty_friction") ? getParam<Real>("penalty_friction")
                                                       : getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _epsilon_tolerance(1.0e-40)

{
  if (_augmented_lagrange_problem)
    mooseError("PenaltySimpleCohesiveZoneModel constraints cannot be enforced with an augmented "
               "Lagrange approach.");
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
}

void
PenaltySimpleCohesiveZoneModel::reinit()
{
  // Normal contact pressure with penalty
  PenaltyWeightedGapUserObject::reinit();

  // Compute all rotations that were created as material properties in CZMComputeDisplacementJump
  prepareJumpKinematicQuantities();

  // Reset frictional pressure
  _frictional_contact_traction_one.resize(_qrule_msm->n_points());
  _frictional_contact_traction_two.resize(_qrule_msm->n_points()); // 3D

  for (const auto qp : make_range(_qrule_msm->n_points()))
  {
    _frictional_contact_traction_one[qp] = 0.0;
    _frictional_contact_traction_two[qp] = 0.0;
  }

  // zero vector
  const static TwoVector zero{0.0, 0.0};

  // iterate over nodes
  for (const auto i : make_range(_test->size()))
  {
    // current node
    const Node * const node = _lower_secondary_elem->node_ptr(i);

    // Compute the weighted nodal deformation gradient and rotation tensors.
    computeFandR(node);

    // The call below is a 'macro' call. Create a utility function or user object for it.
    computeBilinearMixedModeTraction(node);

    // Build final traction vector
    computeGlobalTraction(node);

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
