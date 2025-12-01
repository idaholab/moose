//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneModelBase.h"
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

InputParameters
CohesiveZoneModelBase::validParams()
{
  InputParameters params = WeightedVelocitiesUserObject::validParams();
  params += PenaltyWeightedGapUserObject::validParams();
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  params.addClassDescription(
      "Computes the mortar frictional contact force via a penalty approach.");
  // Suppress augmented Lagrange parameters. AL implementation for CZM remains to be done.
  params.addParam<Real>("penalty_friction",
                        "The penalty factor for frictional interaction. If not provide, the normal "
                        "penalty factor is also used for the frictional problem.");
  params.addParam<Real>(
      "friction_coefficient", 0.0, "The friction coefficient ruling Coulomb friction equations.");
  params.suppressParameter<Real>("max_penalty_multiplier");
  params.suppressParameter<Real>("penalty_multiplier");
  params.suppressParameter<Real>("penetration_tolerance");
  params.setParameters("penalty", 0.0);
  return params;
}

CohesiveZoneModelBase::CohesiveZoneModelBase(const InputParameters & parameters)
  /*
   * We are using virtual inheritance to avoid the "Diamond inheritance" problem. This means that
   * we have to construct WeightedGapUserObject explicitly as it will _not_ be constructed in
   * the intermediate base classes PenaltyWeightedGapUserObject and WeightedVelocitiesUserObject.
   * Virtual inheritance ensures that only one instance of WeightedGapUserObject is included in this
   * class. The inheritance diagram is as follows:
   *
   * WeightedGapUserObject         <-----   PenaltyWeightedGapUserObject
   *          ^                                         ^
   *          |                                         |
   * WeightedVelocitiesUserObject  <-----   CohesiveZoneModelBase
   *
   */
  : WeightedGapUserObject(parameters),
    PenaltyWeightedGapUserObject(parameters),
    WeightedVelocitiesUserObject(parameters),
    _ndisp(coupledComponents("displacements")),
    _penalty_friction(isParamValid("penalty_friction") ? getParam<Real>("penalty_friction")
                                                       : getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _epsilon_tolerance(1.0e-40)

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

  if (_augmented_lagrange_problem)
    mooseError("CohesiveZoneModelBase constraints cannot be enforced with an augmented "
               "Lagrange approach.");
}

const VariableTestValue &
CohesiveZoneModelBase::test() const
{
  return _aux_lm_var ? _aux_lm_var->phiLower() : _disp_x_var->phiLower();
}

void
CohesiveZoneModelBase::computeQpProperties()
{
  WeightedVelocitiesUserObject::computeQpProperties();

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
}

void
CohesiveZoneModelBase::computeQpIProperties()
{
  WeightedVelocitiesUserObject::computeQpIProperties();
  // Get the _dof_to_weighted_gap map
  const auto * const dof = static_cast<const DofObject *>(_lower_secondary_elem->node_ptr(_i));

  // TODO: Probably better to interpolate the deformation gradients.
  _dof_to_F[dof] += (*_test)[_i][_qp] * _F_interpolation;
  _dof_to_F_neighbor[dof] += (*_test)[_i][_qp] * _F_neighbor_interpolation;
}

void
CohesiveZoneModelBase::computeFandR(const Node * const node)
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
        throw MooseException("The deformation gradient on the secondary surface is not finite in "
                             "CohesiveZoneModelBase. MOOSE needs to cut the time step size.");

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

template <class T>
T
CohesiveZoneModelBase::normalizeQuantity(const std::unordered_map<const DofObject *, T> & map,
                                         const Node * const node)
{
  return libmesh_map_find(map, node) / _dof_to_weighted_gap[node].second;
}

void
CohesiveZoneModelBase::timestepSetup()
{
  // these functions do not call WeightedGapUserObject::timestepSetup to avoid double initialization
  WeightedVelocitiesUserObject::selfTimestepSetup();
  PenaltyWeightedGapUserObject::selfTimestepSetup();

  // instead we call it explicitly here
  WeightedGapUserObject::timestepSetup();

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

  for (auto & [dof_object, delta_tangential_lm] : _dof_to_frictional_lagrange_multipliers)
    delta_tangential_lm.setZero();

  // save off tangential traction from the last timestep
  for (auto & map_pr : _dof_to_damage)
  {
    auto & [damage, old_damage] = map_pr.second;
    old_damage = {MetaPhysicL::raw_value(damage)};
    damage = {0.0};
  }
}

void
CohesiveZoneModelBase::initialize()
{
  // these functions do not call WeightedGapUserObject::initialize to avoid double initialization
  WeightedVelocitiesUserObject::selfInitialize();
  PenaltyWeightedGapUserObject::selfInitialize();

  // instead we call it explicitly here
  WeightedGapUserObject::initialize();
  _dof_to_F.clear();
  _dof_to_F_neighbor.clear();
  _dof_to_interface_displacement_jump.clear();
  _dof_to_interface_F.clear();
  _dof_to_interface_R.clear();
  _dof_to_czm_traction.clear();
  for (auto & map_pr : _dof_to_rotation_matrix)
    map_pr.second.setToIdentity();
}

void
CohesiveZoneModelBase::reinit()
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
    computeCZMTraction(node);

    // Build final traction vector
    computeGlobalTraction(node);

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

      auto damage = _dof_to_damage[node].first;

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
          penalty_friction * (MetaPhysicL::raw_value(slip_distance)).norm() >
              0.4 * _friction_coefficient * damage * std::abs(normal_pressure))
      {
        inner_iteration_penalty_friction =
            MetaPhysicL::raw_value(
                0.4 * _friction_coefficient * damage * std::abs(normal_pressure) /
                (penalty_friction * (MetaPhysicL::raw_value(slip_distance)).norm())) *
            penalty_friction * slip_distance;
      }

      ADTwoVector tangential_trial_traction =
          old_tangential_traction + tangential_lm + inner_iteration_penalty_friction;

      // Nonlinearity below
      ADReal tangential_trial_traction_norm =
          (MetaPhysicL::raw_value(tangential_trial_traction)).norm();
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

    // Now that we have consistent nodal frictional values, create an interpolated frictional
    // pressure variable.
    const auto & test_i = (*_test)[i];
    for (const auto qp : make_range(_qrule_msm->n_points()))
    {
      _frictional_contact_traction_one[qp] += test_i[qp] * tangential_traction(0);
      _frictional_contact_traction_two[qp] += test_i[qp] * tangential_traction(1);
    }
  }

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
CohesiveZoneModelBase::prepareJumpKinematicQuantities()
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
CohesiveZoneModelBase::finalize()
{
  WeightedVelocitiesUserObject::finalize();
  PenaltyWeightedGapUserObject::selfFinalize();

  const bool send_data_back = !constrainedByOwner();

  Moose::Mortar::Contact::communicateR2T(
      _dof_to_F, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateR2T(
      _dof_to_F_neighbor, _subproblem.mesh(), _nodal, _communicator, send_data_back);

  Moose::Mortar::Contact::communicateRealObject(
      _dof_to_weighted_displacements, _subproblem.mesh(), _nodal, _communicator, send_data_back);
}

const ADVariableValue &
CohesiveZoneModelBase::contactTangentialPressureDirOne() const
{
  return _frictional_contact_traction_one;
}

const ADVariableValue &
CohesiveZoneModelBase::contactTangentialPressureDirTwo() const
{
  return _frictional_contact_traction_two;
}

void
CohesiveZoneModelBase::computeGlobalTraction(const Node * const node)
{
  // First call does not have maps available
  const bool return_boolean = _dof_to_czm_traction.find(node) == _dof_to_czm_traction.end();
  if (return_boolean)
    return;

  const auto local_traction_vector = libmesh_map_find(_dof_to_czm_traction, node);
  const auto rotation_matrix = libmesh_map_find(_dof_to_rotation_matrix, node);

  _dof_to_czm_traction[node] = rotation_matrix * local_traction_vector;
}

const ADVariableValue &
CohesiveZoneModelBase::czmGlobalTraction(const unsigned int i) const
{
  mooseAssert(i <= 3,
              "Internal error in czmGlobalTraction. Index exceeds the traction vector size.");

  return _czm_interpolated_traction[i];
}
