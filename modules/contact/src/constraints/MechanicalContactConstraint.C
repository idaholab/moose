//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MechanicalContactConstraint.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "AuxiliarySystem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "AugmentedLagrangianContactProblem.h"
#include "Executioner.h"
#include "AddVariableAction.h"
#include "ContactLineSearchBase.h"
#include "ContactAction.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("ContactApp", MechanicalContactConstraint);

const unsigned int MechanicalContactConstraint::_no_iterations = 0;

InputParameters
MechanicalContactConstraint::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params += ContactAction::commonParameters();

  params.addRequiredParam<BoundaryName>("boundary", "The primary boundary");
  params.addParam<BoundaryName>("secondary", "The secondary boundary");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this constraint acts on. (0 for x, "
                                        "1 for y, 2 for z)");

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addCoupledVar("secondary_gap_offset", "offset to the gap distance from secondary side");
  params.addCoupledVar("mapped_primary_gap_offset",
                       "offset to the gap distance mapped from primary side");
  params.addRequiredCoupledVar("nodal_area", "The nodal area");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("penalty_multiplier",
                        1.0,
                        "The growth factor for the penalty applied at the end of each augmented "
                        "Lagrange update iteration");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "capture_tolerance", 0, "Normal distance from surface within which nodes are captured");

  params.addParam<Real>("tension_release",
                        0.0,
                        "Tension release threshold.  A node in contact "
                        "will not be released if its tensile load is below "
                        "this value.  No tension release if negative.");

  params.addParam<bool>(
      "normalize_penalty",
      false,
      "Whether to normalize the penalty parameter with the nodal area for penalty contact.");
  params.addParam<bool>(
      "primary_secondary_jacobian",
      true,
      "Whether to include jacobian entries coupling primary and secondary nodes.");
  params.addParam<bool>(
      "connected_secondary_nodes_jacobian",
      true,
      "Whether to include jacobian entries coupling nodes connected to secondary nodes.");
  params.addParam<bool>("non_displacement_variables_jacobian",
                        true,
                        "Whether to include jacobian entries coupling with variables that are not "
                        "displacement variables.");
  params.addParam<unsigned int>("stick_lock_iterations",
                                std::numeric_limits<unsigned int>::max(),
                                "Number of times permitted to switch between sticking and slipping "
                                "in a solution before locking node in a sticked state.");
  params.addParam<Real>("stick_unlock_factor",
                        1.5,
                        "Factor by which frictional capacity must be "
                        "exceeded to permit stick-locked node to slip "
                        "again.");
  params.addParam<Real>("al_penetration_tolerance",
                        "The tolerance of the penetration for augmented Lagrangian method.");
  params.addParam<Real>("al_incremental_slip_tolerance",
                        "The tolerance of the incremental slip for augmented Lagrangian method.");

  params.addParam<Real>("al_frictional_force_tolerance",
                        "The tolerance of the frictional force for augmented Lagrangian method.");
  params.addParam<bool>(
      "print_contact_nodes", false, "Whether to print the number of nodes in contact.");

  params.addClassDescription(
      "Apply non-penetration constraints on the mechanical deformation "
      "using a node on face, primary/secondary algorithm, and multiple options "
      "for the physical behavior on the interface and the mathematical "
      "formulation for constraint enforcement");

  return params;
}

Threads::spin_mutex MechanicalContactConstraint::_contact_set_mutex;

MechanicalContactConstraint::MechanicalContactConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _displaced_problem(parameters.get<FEProblemBase *>("_fe_problem_base")->getDisplacedProblem()),
    _component(getParam<unsigned int>("component")),
    _model(getParam<MooseEnum>("model").getEnum<ContactModel>()),
    _formulation(getParam<MooseEnum>("formulation").getEnum<ContactFormulation>()),
    _normalize_penalty(getParam<bool>("normalize_penalty")),
    _penalty(getParam<Real>("penalty")),
    _penalty_multiplier(getParam<Real>("penalty_multiplier")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _tension_release(getParam<Real>("tension_release")),
    _capture_tolerance(getParam<Real>("capture_tolerance")),
    _stick_lock_iterations(getParam<unsigned int>("stick_lock_iterations")),
    _stick_unlock_factor(getParam<Real>("stick_unlock_factor")),
    _update_stateful_data(true),
    _residual_copy(_sys.residualGhosted()),
    _mesh_dimension(_mesh.dimension()),
    _vars(3, libMesh::invalid_uint),
    _var_objects(3, nullptr),
    _has_secondary_gap_offset(isCoupled("secondary_gap_offset")),
    _secondary_gap_offset_var(_has_secondary_gap_offset ? getVar("secondary_gap_offset", 0)
                                                        : nullptr),
    _has_mapped_primary_gap_offset(isCoupled("mapped_primary_gap_offset")),
    _mapped_primary_gap_offset_var(
        _has_mapped_primary_gap_offset ? getVar("mapped_primary_gap_offset", 0) : nullptr),
    _nodal_area_var(getVar("nodal_area", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution()),
    _primary_secondary_jacobian(getParam<bool>("primary_secondary_jacobian")),
    _connected_secondary_nodes_jacobian(getParam<bool>("connected_secondary_nodes_jacobian")),
    _non_displacement_vars_jacobian(getParam<bool>("non_displacement_variables_jacobian")),
    _contact_linesearch(dynamic_cast<ContactLineSearchBase *>(_subproblem.getLineSearch())),
    _print_contact_nodes(getParam<bool>("print_contact_nodes")),
    _augmented_lagrange_problem(dynamic_cast<AugmentedLagrangianContactProblem *>(&_fe_problem)),
    _lagrangian_iteration_number(_augmented_lagrange_problem
                                     ? _augmented_lagrange_problem->getLagrangianIterationNumber()
                                     : _no_iterations)
{
  _overwrite_secondary_residual = false;

  if (isParamValid("displacements"))
  {
    // modern parameter scheme for displacements
    for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
    {
      _vars[i] = coupled("displacements", i);
      _var_objects[i] = getVar("displacements", i);
    }
  }

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));

  if (_formulation == ContactFormulation::TANGENTIAL_PENALTY && _model != ContactModel::COULOMB)
    mooseError("The 'tangential_penalty' formulation can only be used with the 'coulomb' model");

  if (_model == ContactModel::GLUED)
    _penetration_locator.setUpdate(false);

  if (_friction_coefficient < 0)
    mooseError("The friction coefficient must be nonnegative");

  // set _penalty_tangential to the value of _penalty for now
  _penalty_tangential = _penalty;

  if (_formulation == ContactFormulation::AUGMENTED_LAGRANGE)
  {
    if (_model == ContactModel::GLUED)
      mooseError("The Augmented Lagrangian contact formulation does not support GLUED case.");

    if (!_augmented_lagrange_problem)
      mooseError("The Augmented Lagrangian contact formulation must use "
                 "AugmentedLagrangianContactProblem.");

    if (!parameters.isParamValid("al_penetration_tolerance"))
      mooseError("For Augmented Lagrangian contact, al_penetration_tolerance must be provided.");
    else
      _al_penetration_tolerance = parameters.get<Real>("al_penetration_tolerance");

    if (_model != ContactModel::FRICTIONLESS)
    {
      if (!parameters.isParamValid("al_incremental_slip_tolerance") ||
          !parameters.isParamValid("al_frictional_force_tolerance"))
      {
        mooseError("For the Augmented Lagrangian frictional contact formualton, "
                   "al_incremental_slip_tolerance and "
                   "al_frictional_force_tolerance must be provided.");
      }
      else
      {
        _al_incremental_slip_tolerance = parameters.get<Real>("al_incremental_slip_tolerance");
        _al_frictional_force_tolerance = parameters.get<Real>("al_frictional_force_tolerance");
      }
    }
  }
}

void
MechanicalContactConstraint::timestepSetup()
{
  if (_component == 0)
  {
    updateContactStatefulData(/* beginning_of_step = */ true);
    if (_formulation == ContactFormulation::AUGMENTED_LAGRANGE)
      updateAugmentedLagrangianMultiplier(/* beginning_of_step = */ true);

    _update_stateful_data = false;

    if (_contact_linesearch)
      _contact_linesearch->reset();
  }
}

void
MechanicalContactConstraint::jacobianSetup()
{
  if (_component == 0)
  {
    if (_update_stateful_data)
      updateContactStatefulData(/* beginning_of_step = */ false);
    _update_stateful_data = true;
  }
}

void
MechanicalContactConstraint::updateAugmentedLagrangianMultiplier(bool beginning_of_step)
{
  for (auto & pinfo_pair : _penetration_locator._penetration_info)
  {
    const dof_id_type secondary_node_num = pinfo_pair.first;
    PenetrationInfo * pinfo = pinfo_pair.second;

    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    const Real distance =
        pinfo->_normal * (pinfo->_closest_point - _mesh.nodeRef(secondary_node_num)) -
        gapOffset(_mesh.nodePtr(secondary_node_num));

    if (beginning_of_step && _model == ContactModel::COULOMB)
    {
      pinfo->_lagrange_multiplier_slip.zero();
      if (pinfo->isCaptured())
        pinfo->_mech_status = PenetrationInfo::MS_STICKING;
    }

    if (pinfo->isCaptured())
    {
      if (_model == ContactModel::FRICTIONLESS)
        pinfo->_lagrange_multiplier -= getPenalty(*pinfo) * distance;

      if (_model == ContactModel::COULOMB)
      {
        if (!beginning_of_step)
        {
          Real penalty = getPenalty(*pinfo);
          RealVectorValue pen_force_normal =
              penalty * (-distance) * pinfo->_normal + pinfo->_lagrange_multiplier * pinfo->_normal;

          // update normal lagrangian multiplier
          pinfo->_lagrange_multiplier += penalty * (-distance);

          // Frictional capacity
          const Real capacity(_friction_coefficient * (pen_force_normal * pinfo->_normal < 0
                                                           ? -pen_force_normal * pinfo->_normal
                                                           : 0));

          RealVectorValue tangential_inc_slip =
              pinfo->_incremental_slip -
              (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

          Real penalty_slip = getTangentialPenalty(*pinfo);

          RealVectorValue inc_pen_force_tangential =
              pinfo->_lagrange_multiplier_slip + penalty_slip * tangential_inc_slip;

          RealVectorValue tau_old = pinfo->_contact_force_old -
                                    pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old);

          RealVectorValue contact_force_tangential = inc_pen_force_tangential + tau_old;
          const Real tan_mag(contact_force_tangential.norm());

          if (tan_mag > capacity * (_al_frictional_force_tolerance + 1.0))
          {
            pinfo->_lagrange_multiplier_slip =
                -tau_old + capacity * contact_force_tangential / tan_mag;
            if (MooseUtils::absoluteFuzzyEqual(capacity, 0.0))
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
          {
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
            pinfo->_lagrange_multiplier_slip += penalty_slip * tangential_inc_slip;
          }
        }
      }
    }
  }
}

bool
MechanicalContactConstraint::AugmentedLagrangianContactConverged()
{
  Real contactResidual = 0.0;
  unsigned int converged = 0;

  for (auto & pinfo_pair : _penetration_locator._penetration_info)
  {
    const dof_id_type secondary_node_num = pinfo_pair.first;
    PenetrationInfo * pinfo = pinfo_pair.second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    const Real distance =
        pinfo->_normal * (pinfo->_closest_point - _mesh.nodeRef(secondary_node_num)) -
        gapOffset(_mesh.nodePtr(secondary_node_num));

    if (pinfo->isCaptured())
    {
      if (contactResidual < std::abs(distance))
        contactResidual = std::abs(distance);

      // penetration < tol
      if (contactResidual > _al_penetration_tolerance)
      {
        converged = 1;
        break;
      }

      if (_model == ContactModel::COULOMB)
      {
        RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) *
                                             pinfo->_normal);
        RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);

        RealVectorValue tangential_inc_slip =
            pinfo->_incremental_slip - (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

        const Real tan_mag(contact_force_tangential.norm());
        const Real tangential_inc_slip_mag = tangential_inc_slip.norm();

        RealVectorValue distance_vec =
            (pinfo->_normal * (_mesh.nodeRef(secondary_node_num) - pinfo->_closest_point) +
             gapOffset(_mesh.nodePtr(secondary_node_num))) *
            pinfo->_normal;

        Real penalty = getPenalty(*pinfo);
        RealVectorValue pen_force_normal =
            penalty * distance_vec + pinfo->_lagrange_multiplier * pinfo->_normal;

        // Frictional capacity
        Real capacity(_friction_coefficient * (pen_force_normal * pinfo->_normal < 0
                                                   ? -pen_force_normal * pinfo->_normal
                                                   : 0.0));

        // incremental slip <= tol for all pinfo_pair such that tan_mag < capacity
        if (MooseUtils::absoluteFuzzyLessThan(tan_mag, capacity) &&
            pinfo->_mech_status == PenetrationInfo::MS_STICKING)
        {
          if (MooseUtils::absoluteFuzzyGreaterThan(tangential_inc_slip_mag,
                                                   _al_incremental_slip_tolerance))
          {
            converged = 2;
            break;
          }
        }

        // for all pinfo_pair, tag_mag should be less than (1 + tol) * (capacity + tol)
        if (tan_mag >
            (1 + _al_frictional_force_tolerance) * (capacity + _al_frictional_force_tolerance))
        {
          converged = 3;
          break;
        }
      }
    }
  }

  _communicator.max(converged);

  if (converged == 1)
    _console << "The Augmented Lagrangian contact tangential sliding enforcement is NOT satisfied "
             << std::endl;
  else if (converged == 2)
    _console << "The Augmented Lagrangian contact tangential sliding enforcement is NOT satisfied "
             << std::endl;
  else if (converged == 3)
    _console << "The Augmented Lagrangian contact frictional force enforcement is NOT satisfied "
             << std::endl;
  else
    return true;

  return false;
}

void
MechanicalContactConstraint::updateContactStatefulData(bool beginning_of_step)
{
  for (auto & pinfo_pair : _penetration_locator._penetration_info)
  {
    PenetrationInfo * pinfo = pinfo_pair.second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    if (beginning_of_step)
    {
      if (_app.getExecutioner()->lastSolveConverged())
      {
        pinfo->_contact_force_old = pinfo->_contact_force;
        pinfo->_accumulated_slip_old = pinfo->_accumulated_slip;
        pinfo->_frictional_energy_old = pinfo->_frictional_energy;
        pinfo->_mech_status_old = pinfo->_mech_status;
      }
      else if (pinfo->_mech_status_old == PenetrationInfo::MS_NO_CONTACT &&
               pinfo->_mech_status != PenetrationInfo::MS_NO_CONTACT)
      {
        // The penetration info object could be based on a bad state so delete it
        delete pinfo_pair.second;
        pinfo_pair.second = NULL;
        continue;
      }

      pinfo->_locked_this_step = 0;
      pinfo->_stick_locked_this_step = 0;
      pinfo->_starting_elem = pinfo->_elem;
      pinfo->_starting_side_num = pinfo->_side_num;
      pinfo->_starting_closest_point_ref = pinfo->_closest_point_ref;
    }
    pinfo->_incremental_slip_prev_iter = pinfo->_incremental_slip;
  }
}

bool
MechanicalContactConstraint::shouldApply()
{
  bool in_contact = false;

  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      bool is_nonlinear = _subproblem.computingNonlinearResid();

      // This computes the contact force once per constraint, rather than once per quad point
      // and for both primary and secondary cases.
      if (_component == 0)
        computeContactForce(pinfo, is_nonlinear);

      if (pinfo->isCaptured())
      {
        in_contact = true;
        if (is_nonlinear)
        {
          Threads::spin_mutex::scoped_lock lock(_contact_set_mutex);
          _current_contact_state.insert(pinfo->_node->id());
        }
      }
    }
  }

  return in_contact;
}

void
MechanicalContactConstraint::computeContactForce(PenetrationInfo * pinfo, bool update_contact_set)
{
  const Node * node = pinfo->_node;

  // Build up residual vector
  RealVectorValue res_vec;
  for (unsigned int i = 0; i < _mesh_dimension; ++i)
  {
    dof_id_type dof_number = node->dof_number(0, _vars[i], 0);
    res_vec(i) = _residual_copy(dof_number) / _var_objects[i]->scalingFactor();
  }

  RealVectorValue distance_vec(_mesh.nodeRef(node->id()) - pinfo->_closest_point);
  if (distance_vec.norm() != 0)
    distance_vec += gapOffset(node) * pinfo->_normal * distance_vec.unit() * distance_vec.unit();

  const Real gap_size = -1.0 * pinfo->_normal * distance_vec;

  // This is for preventing an increment of pinfo->_locked_this_step for nodes that are
  // captured and released in this function
  bool newly_captured = false;

  // Capture nodes that are newly in contact
  if (update_contact_set && !pinfo->isCaptured() &&
      MooseUtils::absoluteFuzzyGreaterEqual(gap_size, 0.0, _capture_tolerance))
  {
    newly_captured = true;
    pinfo->capture();

    // Increment the lock count every time the node comes back into contact from not being in
    // contact.
    if (_formulation == ContactFormulation::KINEMATIC ||
        _formulation == ContactFormulation::TANGENTIAL_PENALTY)
      ++pinfo->_locked_this_step;
  }

  if (!pinfo->isCaptured())
    return;

  const Real penalty = getPenalty(*pinfo);
  const Real penalty_slip = getTangentialPenalty(*pinfo);

  RealVectorValue pen_force(penalty * distance_vec);

  switch (_model)
  {
    case ContactModel::FRICTIONLESS:
      switch (_formulation)
      {
        case ContactFormulation::KINEMATIC:
          pinfo->_contact_force = -pinfo->_normal * (pinfo->_normal * res_vec);
          break;

        case ContactFormulation::PENALTY:
          pinfo->_contact_force = pinfo->_normal * (pinfo->_normal * pen_force);
          break;

        case ContactFormulation::AUGMENTED_LAGRANGE:
          pinfo->_contact_force =
              (pinfo->_normal *
               (pinfo->_normal * (pen_force + pinfo->_lagrange_multiplier * pinfo->_normal)));
          break;

        default:
          mooseError("Invalid contact formulation");
          break;
      }
      pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
      break;
    case ContactModel::COULOMB:
      switch (_formulation)
      {
        case ContactFormulation::KINEMATIC:
        {
          // Frictional capacity
          const Real capacity(_friction_coefficient *
                              (res_vec * pinfo->_normal > 0 ? res_vec * pinfo->_normal : 0));

          // Normal and tangential components of predictor force
          pinfo->_contact_force = -res_vec;
          RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) *
                                               pinfo->_normal);
          RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);

          RealVectorValue tangential_inc_slip =
              pinfo->_incremental_slip -
              (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

          // Magnitude of tangential predictor force
          const Real tan_mag(contact_force_tangential.norm());
          const Real tangential_inc_slip_mag = tangential_inc_slip.norm();
          const Real slip_tol = capacity / penalty;
          pinfo->_slip_tol = slip_tol;

          if ((tangential_inc_slip_mag > slip_tol || tan_mag > capacity) &&
              (pinfo->_stick_locked_this_step < _stick_lock_iterations ||
               tan_mag > capacity * _stick_unlock_factor))
          {
            if (pinfo->_stick_locked_this_step >= _stick_lock_iterations)
              pinfo->_stick_locked_this_step = 0;

            // Check for scenario where node has slipped too far in a previous iteration
            // for special treatment (only done if the incremental slip is non-trivial)
            bool slipped_too_far = false;
            RealVectorValue slip_inc_direction;
            if (tangential_inc_slip_mag > slip_tol)
            {
              slip_inc_direction = tangential_inc_slip / tangential_inc_slip_mag;
              Real slip_dot_tang_force = slip_inc_direction * contact_force_tangential;
              if (slip_dot_tang_force < capacity)
                slipped_too_far = true;
            }

            if (slipped_too_far) // slip back along slip increment
              pinfo->_contact_force = contact_force_normal + capacity * slip_inc_direction;
            else
            {
              if (tan_mag > 0) // slip along tangential force direction
                pinfo->_contact_force =
                    contact_force_normal + capacity * contact_force_tangential / tan_mag;
              else // treat as frictionless
                pinfo->_contact_force = contact_force_normal;
            }
            if (capacity == 0)
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
          {
            if (pinfo->_mech_status != PenetrationInfo::MS_STICKING &&
                pinfo->_mech_status != PenetrationInfo::MS_NO_CONTACT)
              ++pinfo->_stick_locked_this_step;
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
          }
          break;
        }

        case ContactFormulation::PENALTY:
        {
          distance_vec = pinfo->_incremental_slip +
                         (pinfo->_normal * (_mesh.nodeRef(node->id()) - pinfo->_closest_point) +
                          gapOffset(node)) *
                             pinfo->_normal;
          pen_force = penalty * distance_vec;

          // Frictional capacity
          // const Real capacity( _friction_coefficient * (pen_force * pinfo->_normal < 0 ?
          // -pen_force * pinfo->_normal : 0) );
          const Real capacity(_friction_coefficient *
                              (res_vec * pinfo->_normal > 0 ? res_vec * pinfo->_normal : 0));

          // Elastic predictor
          pinfo->_contact_force =
              pen_force + (pinfo->_contact_force_old -
                           pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old));
          RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) *
                                               pinfo->_normal);
          RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);

          // Tangential magnitude of elastic predictor
          const Real tan_mag(contact_force_tangential.norm());

          if (tan_mag > capacity)
          {
            pinfo->_contact_force =
                contact_force_normal + capacity * contact_force_tangential / tan_mag;
            if (MooseUtils::absoluteFuzzyEqual(capacity, 0))
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
          break;
        }

        case ContactFormulation::AUGMENTED_LAGRANGE:
        {
          distance_vec = (pinfo->_normal * (_mesh.nodeRef(node->id()) - pinfo->_closest_point) +
                          gapOffset(node)) *
                         pinfo->_normal;

          RealVectorValue contact_force_normal =
              penalty * distance_vec + pinfo->_lagrange_multiplier * pinfo->_normal;

          RealVectorValue tangential_inc_slip =
              pinfo->_incremental_slip -
              (pinfo->_incremental_slip * pinfo->_normal) * pinfo->_normal;

          RealVectorValue contact_force_tangential =
              pinfo->_lagrange_multiplier_slip +
              (pinfo->_contact_force_old -
               pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old));

          RealVectorValue inc_pen_force_tangential = penalty_slip * tangential_inc_slip;

          if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
            pinfo->_contact_force =
                contact_force_normal + contact_force_tangential + inc_pen_force_tangential;
          else
            pinfo->_contact_force = contact_force_normal + contact_force_tangential;

          break;
        }

        case ContactFormulation::TANGENTIAL_PENALTY:
        {
          // Frictional capacity (kinematic formulation)
          const Real capacity = _friction_coefficient * std::max(res_vec * pinfo->_normal, 0.0);

          // Normal component of contact force (kinematic formulation)
          RealVectorValue contact_force_normal((-res_vec * pinfo->_normal) * pinfo->_normal);

          // Predictor tangential component of contact force (penalty formulation)
          RealVectorValue inc_pen_force_tangential = penalty * pinfo->_incremental_slip;
          RealVectorValue contact_force_tangential =
              inc_pen_force_tangential +
              (pinfo->_contact_force_old -
               pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old));

          // Magnitude of tangential predictor
          const Real tan_mag(contact_force_tangential.norm());

          if (tan_mag > capacity)
          {
            pinfo->_contact_force =
                contact_force_normal + capacity * contact_force_tangential / tan_mag;
            if (MooseUtils::absoluteFuzzyEqual(capacity, 0))
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
            else
              pinfo->_mech_status = PenetrationInfo::MS_SLIPPING_FRICTION;
          }
          else
          {
            pinfo->_contact_force = contact_force_normal + contact_force_tangential;
            pinfo->_mech_status = PenetrationInfo::MS_STICKING;
          }
          break;
        }

        default:
          mooseError("Invalid contact formulation");
          break;
      }
      break;

    case ContactModel::GLUED:
      switch (_formulation)
      {
        case ContactFormulation::KINEMATIC:
          pinfo->_contact_force = -res_vec;
          break;

        case ContactFormulation::PENALTY:
          pinfo->_contact_force = pen_force;
          break;

        case ContactFormulation::AUGMENTED_LAGRANGE:
          pinfo->_contact_force =
              pen_force + pinfo->_lagrange_multiplier * distance_vec / distance_vec.norm();
          break;

        default:
          mooseError("Invalid contact formulation");
          break;
      }
      pinfo->_mech_status = PenetrationInfo::MS_STICKING;
      break;

    default:
      mooseError("Invalid or unavailable contact model");
      break;
  }

  // Release
  if (update_contact_set && _model != ContactModel::GLUED && pinfo->isCaptured() &&
      !newly_captured && _tension_release >= 0.0 &&
      (_contact_linesearch ? true : pinfo->_locked_this_step < 2))
  {
    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force) / nodalArea(*pinfo);
    if (-contact_pressure >= _tension_release)
    {
      pinfo->release();
      pinfo->_contact_force.zero();
    }
  }
}

Real
MechanicalContactConstraint::computeQpSecondaryValue()
{
  return _u_secondary[_qp];
}

Real
MechanicalContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  Real resid = pinfo->_contact_force(_component);
  switch (type)
  {
    case Moose::Secondary:
      if (_formulation == ContactFormulation::KINEMATIC)
      {
        RealVectorValue distance_vec(*_current_node - pinfo->_closest_point);
        if (distance_vec.norm() != 0)
          distance_vec +=
              gapOffset(_current_node) * pinfo->_normal * distance_vec.unit() * distance_vec.unit();

        const Real penalty = getPenalty(*pinfo);
        RealVectorValue pen_force(penalty * distance_vec);

        if (_model == ContactModel::FRICTIONLESS)
          resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;
        else if (_model == ContactModel::COULOMB)
        {
          distance_vec = distance_vec - pinfo->_incremental_slip;
          pen_force = penalty * distance_vec;
          if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
              pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
            resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;
          else
            resid += pen_force(_component);
        }
        else if (_model == ContactModel::GLUED)
          resid += pen_force(_component);
      }
      else if (_formulation == ContactFormulation::TANGENTIAL_PENALTY &&
               _model == ContactModel::COULOMB)
      {
        RealVectorValue distance_vec =
            (pinfo->_normal * (*_current_node - pinfo->_closest_point) + gapOffset(_current_node)) *
            pinfo->_normal;

        const Real penalty = getPenalty(*pinfo);
        RealVectorValue pen_force(penalty * distance_vec);
        resid += pinfo->_normal(_component) * pinfo->_normal * pen_force;
      }
      return _test_secondary[_i][_qp] * resid;

    case Moose::Primary:
      return _test_primary[_i][_qp] * -resid;
  }

  return 0.0;
}

Real
MechanicalContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  const Real penalty = getPenalty(*pinfo);
  const Real penalty_slip = getTangentialPenalty(*pinfo);

  switch (type)
  {
    default:
      mooseError("Unhandled ConstraintJacobianType");

    case Moose::SecondarySecondary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                             _var_objects[i]->scalingFactor();
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                     (_phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                         pinfo->_normal(_component) * pinfo->_normal(_component);
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::COULOMB:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                               _var_objects[i]->scalingFactor();
                }
                return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                       (_phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                           pinfo->_normal(_component) * pinfo->_normal(_component);
              }
              else
              {
                const Real curr_jac =
                    (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                 _connected_dof_indices[_j]) /
                    _var.scalingFactor();
                return (-curr_jac + _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp]);
              }
            }

            case ContactFormulation::PENALTY:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
                return _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                       pinfo->_normal(_component) * pinfo->_normal(_component);
              else
                return _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp];
            }
            case ContactFormulation::AUGMENTED_LAGRANGE:
            {
              Real normal_comp = _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = _phi_secondary[_j][_qp] * penalty_slip * _test_secondary[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              return normal_comp + tang_comp;
            }

            case ContactFormulation::TANGENTIAL_PENALTY:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                             _var_objects[i]->scalingFactor();
              }
              Real normal_comp = -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                                 (_phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                                     pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));

              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::GLUED:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              const Real curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                                 _connected_dof_indices[_j]) /
                                    _var.scalingFactor();
              return (-curr_jac + _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp]);
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp];

            default:
              mooseError("Invalid contact formulation");
          }
        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::SecondaryPrimary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              const Node * curr_primary_node = _current_primary->node_ptr(_j);

              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number,
                                          curr_primary_node->dof_number(0, _vars[_component], 0)) /
                             _var_objects[i]->scalingFactor();
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                     (_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                         pinfo->_normal(_component) * pinfo->_normal(_component);
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::COULOMB:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                const Node * curr_primary_node = _current_primary->node_ptr(_j);

                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) =
                      (*_jacobian)(dof_number,
                                   curr_primary_node->dof_number(0, _vars[_component], 0)) /
                      _var_objects[i]->scalingFactor();
                }
                return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                       (_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                           pinfo->_normal(_component) * pinfo->_normal(_component);
              }
              else
              {
                const Node * curr_primary_node = _current_primary->node_ptr(_j);
                const Real curr_jac =
                    (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                 curr_primary_node->dof_number(0, _vars[_component], 0)) /
                    _var.scalingFactor();
                return (-curr_jac - _phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp]);
              }
            }

            case ContactFormulation::PENALTY:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
                return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                       pinfo->_normal(_component) * pinfo->_normal(_component);
              else
                return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp];
            }
            case ContactFormulation::AUGMENTED_LAGRANGE:
            {
              Real normal_comp = -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = -_phi_primary[_j][_qp] * penalty_slip * _test_secondary[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              return normal_comp + tang_comp;
            }

            case ContactFormulation::TANGENTIAL_PENALTY:
            {
              const Node * curr_primary_node = _current_primary->node_ptr(_j);

              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number,
                                          curr_primary_node->dof_number(0, _vars[_component], 0)) /
                             _var_objects[i]->scalingFactor();
              }
              Real normal_comp = -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                                 (_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                                     pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));

              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }
        case ContactModel::GLUED:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              const Node * curr_primary_node = _current_primary->node_ptr(_j);
              const Real curr_jac =
                  (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                               curr_primary_node->dof_number(0, _vars[_component], 0)) /
                  _var.scalingFactor();
              return (-curr_jac - _phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp]);
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp];

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::PrimarySecondary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                             _var_objects[i]->scalingFactor();
              }
              return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                     _test_primary[_i][_qp];
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return -_test_primary[_i][_qp] * penalty * _phi_secondary[_j][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }
        case ContactModel::COULOMB:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                               _var_objects[i]->scalingFactor();
                }
                return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                       _test_primary[_i][_qp];
              }
              else
              {
                const Real secondary_jac =
                    (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                 _connected_dof_indices[_j]) /
                    _var.scalingFactor();
                return secondary_jac * _test_primary[_i][_qp];
              }
            }

            case ContactFormulation::PENALTY:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
                return -_test_primary[_i][_qp] * penalty * _phi_secondary[_j][_qp] *
                       pinfo->_normal(_component) * pinfo->_normal(_component);
              else
                return -_test_primary[_i][_qp] * penalty * _phi_secondary[_j][_qp];
            }
            case ContactFormulation::AUGMENTED_LAGRANGE:
            {
              Real normal_comp = -_phi_secondary[_j][_qp] * penalty * _test_primary[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = -_phi_secondary[_j][_qp] * penalty_slip * _test_primary[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              return normal_comp + tang_comp;
            }

            case ContactFormulation::TANGENTIAL_PENALTY:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                             _var_objects[i]->scalingFactor();
              }
              Real normal_comp =
                  pinfo->_normal(_component) * (pinfo->_normal * jac_vec) * _test_primary[_i][_qp];

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = -_test_primary[_i][_qp] * penalty * _phi_secondary[_j][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));

              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::GLUED:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              const Real secondary_jac =
                  (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                               _connected_dof_indices[_j]) /
                  _var.scalingFactor();
              return secondary_jac * _test_primary[_i][_qp];
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return -_test_primary[_i][_qp] * penalty * _phi_secondary[_j][_qp];

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::PrimaryPrimary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
              return 0.0;

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp] *
                     pinfo->_normal(_component) * pinfo->_normal(_component);

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::COULOMB:
        case ContactModel::GLUED:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
              return 0.0;

            case ContactFormulation::PENALTY:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
                return _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp] *
                       pinfo->_normal(_component) * pinfo->_normal(_component);
              else
                return _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp];
            }

            case ContactFormulation::TANGENTIAL_PENALTY:
            {
              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              return tang_comp; // normal component is zero
            }

            case ContactFormulation::AUGMENTED_LAGRANGE:
            {
              Real normal_comp = _phi_primary[_j][_qp] * penalty * _test_primary[_i][_qp] *
                                 pinfo->_normal(_component) * pinfo->_normal(_component);

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = _phi_primary[_j][_qp] * penalty_slip * _test_primary[_i][_qp] *
                            (1.0 - pinfo->_normal(_component) * pinfo->_normal(_component));
              return normal_comp + tang_comp;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0.0;
}

Real
MechanicalContactConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                      unsigned int jvar)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  const Real penalty = getPenalty(*pinfo);
  const Real penalty_slip = getTangentialPenalty(*pinfo);

  unsigned int coupled_component;
  Real normal_component_in_coupled_var_dir = 1.0;
  if (getCoupledVarComponent(jvar, coupled_component))
    normal_component_in_coupled_var_dir = pinfo->_normal(coupled_component);

  switch (type)
  {
    default:
      mooseError("Unhandled ConstraintJacobianType");

    case Moose::SecondarySecondary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                             _var_objects[i]->scalingFactor();
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                     (_phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                         pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::COULOMB:
        {
          if ((_formulation == ContactFormulation::KINEMATIC ||
               _formulation == ContactFormulation::TANGENTIAL_PENALTY) &&
              (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
               pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
          {
            RealVectorValue jac_vec;
            for (unsigned int i = 0; i < _mesh_dimension; ++i)
            {
              dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
              jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                           _var_objects[i]->scalingFactor();
            }

            return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) +
                   (_phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                       pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          }
          else if ((_formulation == ContactFormulation::PENALTY) &&
                   (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                    pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          else if (_formulation == ContactFormulation::AUGMENTED_LAGRANGE)
          {
            Real normal_comp = _phi_secondary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                               pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            Real tang_comp = 0.0;
            if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
              tang_comp = _phi_secondary[_j][_qp] * penalty_slip * _test_secondary[_i][_qp] *
                          (-pinfo->_normal(_component) * normal_component_in_coupled_var_dir);
            return normal_comp + tang_comp;
          }
          else
          {
            const Real curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                               _connected_dof_indices[_j]);
            return -curr_jac;
          }
        }

        case ContactModel::GLUED:
        {
          const Real curr_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                             _connected_dof_indices[_j]);
          return -curr_jac;
        }
        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::SecondaryPrimary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              const Node * curr_primary_node = _current_primary->node_ptr(_j);

              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number,
                                          curr_primary_node->dof_number(0, _vars[_component], 0)) /
                             _var_objects[i]->scalingFactor();
              }
              return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                     (_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                         pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::COULOMB:
          if ((_formulation == ContactFormulation::KINEMATIC ||
               _formulation == ContactFormulation::TANGENTIAL_PENALTY) &&
              (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
               pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
          {
            const Node * curr_primary_node = _current_primary->node_ptr(_j);

            RealVectorValue jac_vec;
            for (unsigned int i = 0; i < _mesh_dimension; ++i)
            {
              dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
              jac_vec(i) =
                  (*_jacobian)(dof_number, curr_primary_node->dof_number(0, _vars[_component], 0)) /
                  _var_objects[i]->scalingFactor();
            }

            return -pinfo->_normal(_component) * (pinfo->_normal * jac_vec) -
                   (_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp]) *
                       pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          }
          else if ((_formulation == ContactFormulation::PENALTY) &&
                   (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                    pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          else if (_formulation == ContactFormulation::AUGMENTED_LAGRANGE)
          {
            Real normal_comp = -_phi_primary[_j][_qp] * penalty * _test_secondary[_i][_qp] *
                               pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            Real tang_comp = 0.0;
            if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
              tang_comp = -_phi_primary[_j][_qp] * penalty_slip * _test_secondary[_i][_qp] *
                          (-pinfo->_normal(_component) * normal_component_in_coupled_var_dir);
            return normal_comp + tang_comp;
          }
          else
            return 0.0;

        case ContactModel::GLUED:
          return 0;

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::PrimarySecondary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              RealVectorValue jac_vec;
              for (unsigned int i = 0; i < _mesh_dimension; ++i)
              {
                dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                             _var_objects[i]->scalingFactor();
              }
              return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                     _test_primary[_i][_qp];
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return -_test_primary[_i][_qp] * penalty * _phi_secondary[_j][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }
        case ContactModel::COULOMB:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                               _var_objects[i]->scalingFactor();
                }
                return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                       _test_primary[_i][_qp];
              }
              else
              {
                const Real secondary_jac =
                    (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                 _connected_dof_indices[_j]) /
                    _var.scalingFactor();
                return secondary_jac * _test_primary[_i][_qp];
              }
            }

            case ContactFormulation::PENALTY:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
                return -_test_primary[_i][_qp] * penalty * _phi_secondary[_j][_qp] *
                       pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
              else
                return 0.0;
            }
            case ContactFormulation::AUGMENTED_LAGRANGE:
            {
              Real normal_comp = -_phi_secondary[_j][_qp] * penalty * _test_primary[_i][_qp] *
                                 pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

              Real tang_comp = 0.0;
              if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
                tang_comp = -_phi_secondary[_j][_qp] * penalty_slip * _test_primary[_i][_qp] *
                            (-pinfo->_normal(_component) * normal_component_in_coupled_var_dir);
              return normal_comp + tang_comp;
            }
            case ContactFormulation::TANGENTIAL_PENALTY:
            {
              if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                  pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              {
                RealVectorValue jac_vec;
                for (unsigned int i = 0; i < _mesh_dimension; ++i)
                {
                  dof_id_type dof_number = _current_node->dof_number(0, _vars[i], 0);
                  jac_vec(i) = (*_jacobian)(dof_number, _connected_dof_indices[_j]) /
                               _var_objects[i]->scalingFactor();
                }
                return pinfo->_normal(_component) * (pinfo->_normal * jac_vec) *
                       _test_primary[_i][_qp];
              }
              else
                return 0.0;
            }

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::GLUED:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
            {
              const Real secondary_jac =
                  (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                               _connected_dof_indices[_j]) /
                  _var.scalingFactor();
              return secondary_jac * _test_primary[_i][_qp];
            }

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return 0.0;

            default:
              mooseError("Invalid contact formulation");
          }

        default:
          mooseError("Invalid or unavailable contact model");
      }

    case Moose::PrimaryPrimary:
      switch (_model)
      {
        case ContactModel::FRICTIONLESS:
          switch (_formulation)
          {
            case ContactFormulation::KINEMATIC:
              return 0.0;

            case ContactFormulation::PENALTY:
            case ContactFormulation::AUGMENTED_LAGRANGE:
              return _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp] *
                     pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            default:
              mooseError("Invalid contact formulation");
          }

        case ContactModel::COULOMB:
        {
          if (_formulation == ContactFormulation::AUGMENTED_LAGRANGE)
          {
            Real normal_comp = _phi_primary[_j][_qp] * penalty * _test_primary[_i][_qp] *
                               pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            if (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION)
              return normal_comp;
            else
              return 0.0;
          }
          else if (_formulation == ContactFormulation::PENALTY &&
                   (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
                    pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          else
            return 0.0;
        }

        case ContactModel::GLUED:
          if (_formulation == ContactFormulation::PENALTY &&
              (pinfo->_mech_status == PenetrationInfo::MS_SLIPPING ||
               pinfo->_mech_status == PenetrationInfo::MS_SLIPPING_FRICTION))
            return _test_primary[_i][_qp] * penalty * _phi_primary[_j][_qp] *
                   pinfo->_normal(_component) * normal_component_in_coupled_var_dir;
          else if (_formulation == ContactFormulation::AUGMENTED_LAGRANGE)
          {
            Real normal_comp = _phi_primary[_j][_qp] * penalty * _test_primary[_i][_qp] *
                               pinfo->_normal(_component) * normal_component_in_coupled_var_dir;

            Real tang_comp = 0.0;
            if (pinfo->_mech_status == PenetrationInfo::MS_STICKING)
              tang_comp = _phi_primary[_j][_qp] * penalty_slip * _test_primary[_i][_qp] *
                          (-pinfo->_normal(_component) * normal_component_in_coupled_var_dir);
            return normal_comp + tang_comp;
          }
          else
            return 0.0;

        default:
          mooseError("Invalid or unavailable contact model");
      }
  }

  return 0.0;
}

Real
MechanicalContactConstraint::gapOffset(const Node * node)
{
  Real val = 0;

  if (_has_secondary_gap_offset)
    val += _secondary_gap_offset_var->getNodalValue(*node);

  if (_has_mapped_primary_gap_offset)
    val += _mapped_primary_gap_offset_var->getNodalValue(*node);

  return val;
}

Real
MechanicalContactConstraint::nodalArea(PenetrationInfo & pinfo)
{
  const Node * node = pinfo._node;

  dof_id_type dof = node->dof_number(_aux_system.number(), _nodal_area_var->number(), 0);

  Real area = (*_aux_solution)(dof);
  if (area == 0.0)
  {
    if (_t_step > 1)
      mooseError("Zero nodal area found");
    else
      area = 1.0; // Avoid divide by zero during initialization
  }

  return area;
}

Real
MechanicalContactConstraint::getPenalty(PenetrationInfo & pinfo)
{
  Real penalty = _penalty;
  if (_normalize_penalty)
    penalty *= nodalArea(pinfo);
  return penalty * MathUtils::pow(_penalty_multiplier, _lagrangian_iteration_number);
}

Real
MechanicalContactConstraint::getTangentialPenalty(PenetrationInfo & pinfo)
{
  Real penalty = _penalty_tangential;
  if (_normalize_penalty)
    penalty *= nodalArea(pinfo);

  return penalty * MathUtils::pow(_penalty_multiplier, _lagrangian_iteration_number);
}

void
MechanicalContactConstraint::computeJacobian()
{
  getConnectedDofIndices(_var.number());

  prepareMatrixTagNeighbor(
      _assembly, _primary_var.number(), _var.number(), Moose::NeighborNeighbor, _Knn);

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpJacobian(Moose::SecondarySecondary);

  if (_primary_secondary_jacobian)
  {
    prepareMatrixTagNeighbor(_assembly, _var.number(), _var.number(), Moose::ElementNeighbor, _Ken);
    if (_Ken.m() && _Ken.n())
    {
      for (_i = 0; _i < _test_secondary.size(); _i++)
        for (_j = 0; _j < _phi_primary.size(); _j++)
          _Ken(_i, _j) += computeQpJacobian(Moose::SecondaryPrimary);
      accumulateTaggedLocalMatrix(
          _assembly, _var.number(), _var.number(), Moose::ElementNeighbor, _Ken);
    }

    _Kne.resize(_test_primary.size(), _connected_dof_indices.size());
    for (_i = 0; _i < _test_primary.size(); _i++)
      // Loop over the connected dof indices so we can get all the jacobian contributions
      for (_j = 0; _j < _connected_dof_indices.size(); _j++)
        _Kne(_i, _j) += computeQpJacobian(Moose::PrimarySecondary);
  }

  if (_Knn.m() && _Knn.n())
  {
    for (_i = 0; _i < _test_primary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        _Knn(_i, _j) += computeQpJacobian(Moose::PrimaryPrimary);
    accumulateTaggedLocalMatrix(
        _assembly, _primary_var.number(), _var.number(), Moose::NeighborNeighbor, _Knn);
  }
}

void
MechanicalContactConstraint::computeOffDiagJacobian(const unsigned int jvar_num)
{
  getConnectedDofIndices(jvar_num);

  _Kee.resize(_test_secondary.size(), _connected_dof_indices.size());

  prepareMatrixTagNeighbor(
      _assembly, _primary_var.number(), jvar_num, Moose::NeighborNeighbor, _Knn);

  for (_i = 0; _i < _test_secondary.size(); _i++)
    // Loop over the connected dof indices so we can get all the jacobian contributions
    for (_j = 0; _j < _connected_dof_indices.size(); _j++)
      _Kee(_i, _j) += computeQpOffDiagJacobian(Moose::SecondarySecondary, jvar_num);

  if (_primary_secondary_jacobian)
  {
    prepareMatrixTagNeighbor(_assembly, _var.number(), jvar_num, Moose::ElementNeighbor, _Ken);
    for (_i = 0; _i < _test_secondary.size(); _i++)
      for (_j = 0; _j < _phi_primary.size(); _j++)
        _Ken(_i, _j) += computeQpOffDiagJacobian(Moose::SecondaryPrimary, jvar_num);
    accumulateTaggedLocalMatrix(_assembly, _var.number(), jvar_num, Moose::ElementNeighbor, _Ken);

    _Kne.resize(_test_primary.size(), _connected_dof_indices.size());
    if (_Kne.m() && _Kne.n())
      for (_i = 0; _i < _test_primary.size(); _i++)
        // Loop over the connected dof indices so we can get all the jacobian contributions
        for (_j = 0; _j < _connected_dof_indices.size(); _j++)
          _Kne(_i, _j) += computeQpOffDiagJacobian(Moose::PrimarySecondary, jvar_num);
  }

  for (_i = 0; _i < _test_primary.size(); _i++)
    for (_j = 0; _j < _phi_primary.size(); _j++)
      _Knn(_i, _j) += computeQpOffDiagJacobian(Moose::PrimaryPrimary, jvar_num);
  accumulateTaggedLocalMatrix(
      _assembly, _primary_var.number(), jvar_num, Moose::NeighborNeighbor, _Knn);
}

void
MechanicalContactConstraint::getConnectedDofIndices(unsigned int var_num)
{
  unsigned int component;
  if (getCoupledVarComponent(var_num, component) || _non_displacement_vars_jacobian)
  {
    if (_primary_secondary_jacobian && _connected_secondary_nodes_jacobian)
      NodeFaceConstraint::getConnectedDofIndices(var_num);
    else
    {
      _connected_dof_indices.clear();
      MooseVariableFEBase & var = _sys.getVariable(0, var_num);
      _connected_dof_indices.push_back(var.nodalDofIndex());
    }
  }

  _phi_secondary.resize(_connected_dof_indices.size());

  dof_id_type current_node_var_dof_index = _sys.getVariable(0, var_num).nodalDofIndex();
  _qp = 0;

  // Fill up _phi_secondary so that it is 1 when j corresponds to the dof associated with this node
  // and 0 for every other dof
  // This corresponds to evaluating all of the connected shape functions at _this_ node
  for (unsigned int j = 0; j < _connected_dof_indices.size(); j++)
  {
    _phi_secondary[j].resize(1);

    if (_connected_dof_indices[j] == current_node_var_dof_index)
      _phi_secondary[j][_qp] = 1.0;
    else
      _phi_secondary[j][_qp] = 0.0;
  }
}

bool
MechanicalContactConstraint::getCoupledVarComponent(unsigned int var_num, unsigned int & component)
{
  component = std::numeric_limits<unsigned int>::max();
  bool coupled_var_is_disp_var = false;
  for (const auto i : make_range(Moose::dim))
  {
    if (var_num == _vars[i])
    {
      coupled_var_is_disp_var = true;
      component = i;
      break;
    }
  }

  return coupled_var_is_disp_var;
}

void
MechanicalContactConstraint::residualEnd()
{
  if (_component == 0 && (_print_contact_nodes || _contact_linesearch))
  {
    _communicator.set_union(_current_contact_state);
    if (_print_contact_nodes)
    {
      if (_current_contact_state == _old_contact_state)
        _console << "Unchanged contact state. " << _current_contact_state.size()
                 << " nodes in contact.\n";
      else
        _console << "Changed contact state!!! " << _current_contact_state.size()
                 << " nodes in contact.\n";
    }
    if (_contact_linesearch)
      _contact_linesearch->insertSet(_current_contact_state);
    _old_contact_state.swap(_current_contact_state);
    _current_contact_state.clear();
  }
}
