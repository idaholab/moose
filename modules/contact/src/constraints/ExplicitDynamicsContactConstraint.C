//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExplicitDynamicsContactConstraint.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "AuxiliarySystem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "Executioner.h"
#include "AddVariableAction.h"
#include "ContactLineSearchBase.h"
#include "ExplicitDynamicsContactAction.h"

#include "libmesh/id_types.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("ContactApp", ExplicitDynamicsContactConstraint);

const unsigned int ExplicitDynamicsContactConstraint::_no_iterations = 0;

InputParameters
ExplicitDynamicsContactConstraint::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();
  params += ExplicitDynamicsContactAction::commonParameters();
  params += TwoMaterialPropertyInterface::validParams();

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
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("penalty",
                        1e8,
                        "The penalty to apply.  Its optimal value can vary depending on the "
                        "stiffness of the materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<bool>(
      "print_contact_nodes", false, "Whether to print the number of nodes in contact.");

  params.addClassDescription(
      "Apply non-penetration constraints on the mechanical deformation in explicit dynamics "
      "using a node on face formulation by solving uncoupled momentum-balance equations.");
  return params;
}

ExplicitDynamicsContactConstraint::ExplicitDynamicsContactConstraint(
    const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, buildBoundaryIDs()),
    _displaced_problem(parameters.get<FEProblemBase *>("_fe_problem_base")->getDisplacedProblem()),
    _component(getParam<unsigned int>("component")),
    _model(getParam<MooseEnum>("model").getEnum<ExplicitDynamicsContactModel>()),
    _update_stateful_data(true),
    _mesh_dimension(_mesh.dimension()),
    _vars(3, libMesh::invalid_uint),
    _var_objects(3, nullptr),
    _has_secondary_gap_offset(isCoupled("secondary_gap_offset")),
    _secondary_gap_offset_var(_has_secondary_gap_offset ? getVar("secondary_gap_offset", 0)
                                                        : nullptr),
    _has_mapped_primary_gap_offset(isCoupled("mapped_primary_gap_offset")),
    _mapped_primary_gap_offset_var(
        _has_mapped_primary_gap_offset ? getVar("mapped_primary_gap_offset", 0) : nullptr),
    _penalty(getParam<Real>("penalty")),
    _print_contact_nodes(getParam<bool>("print_contact_nodes")),
    _residual_copy(_sys.residualGhosted()),
    _gap_rate(&writableVariable("gap_rate")),
    _neighbor_vel_x(isCoupled("vel_x") ? coupledNeighborValue("vel_x") : _zero),
    _neighbor_vel_y(isCoupled("vel_y") ? coupledNeighborValue("vel_y") : _zero),
    _neighbor_vel_z((_mesh.dimension() == 3 && isCoupled("vel_z")) ? coupledNeighborValue("vel_z")
                                                                   : _zero)
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

  if (_model == ExplicitDynamicsContactModel::FRICTIONLESS_BALANCE)
  {
    bool is_correct =
        (isCoupled("vel_x") && isCoupled("vel_y") && _mesh.dimension() == 2) ||
        (isCoupled("vel_x") && isCoupled("vel_y") && isCoupled("vel_z") && _mesh.dimension() == 3);

    if (!is_correct)
      paramError("vel_x",
                 "Velocities vel_x and vel_y (also vel_z in three dimensions) need to be provided "
                 "for the 'balance' option of solving normal contact in explicit dynamics.");
  }
}

void
ExplicitDynamicsContactConstraint::timestepSetup()
{
  if (_component == 0)
  {
    updateContactStatefulData(/* beginning_of_step = */ true);
    _update_stateful_data = false;
    _dof_to_position.clear();
  }
}

void
ExplicitDynamicsContactConstraint::updateContactStatefulData(bool beginning_of_step)
{
  for (auto & [secondary_node_num, pinfo] : _penetration_locator._penetration_info)
  {
    if (!pinfo)
      continue;

    const Node & node = _mesh.nodeRef(secondary_node_num);
    if (node.n_comp(_sys.number(), _vars[_component]) < 1)
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
        mooseWarning("Previous step did not converge. Check results");
        // The penetration info object could be based on a bad state so delete it
        delete pinfo;
        pinfo = nullptr;
        continue;
      }

      pinfo->_starting_elem = pinfo->_elem;
      pinfo->_starting_side_num = pinfo->_side_num;
      pinfo->_starting_closest_point_ref = pinfo->_closest_point_ref;
    }
    pinfo->_incremental_slip_prev_iter = pinfo->_incremental_slip;
  }
}

bool
ExplicitDynamicsContactConstraint::shouldApply()
{
  if (_current_node->processor_id() != _fe_problem.processor_id())
    return false;

  bool in_contact = false;

  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != nullptr)
    {
      // This computes the contact force once per constraint, rather than once per quad point
      // and for both primary and secondary cases.
      if (_component == 0)
        computeContactForce(*_current_node, pinfo, true);

      if (pinfo->isCaptured())
        in_contact = true;
    }
  }

  return in_contact;
}

void
ExplicitDynamicsContactConstraint::computeContactForce(const Node & node,
                                                       PenetrationInfo * pinfo,
                                                       bool update_contact_set)
{
  RealVectorValue distance_vec(node - pinfo->_closest_point);

  // Adding current velocity to the distance vector to ensure proper contact check
  dof_id_type dof_x = node.dof_number(_sys.number(), _var_objects[0]->number(), 0);
  dof_id_type dof_y = node.dof_number(_sys.number(), _var_objects[1]->number(), 0);
  dof_id_type dof_z =
      _mesh.dimension() == 3 ? node.dof_number(_sys.number(), _var_objects[2]->number(), 0) : 0;
  RealVectorValue udotvec = {(*_sys.solutionUDot())(dof_x)*_dt,
                             (*_sys.solutionUDot())(dof_y)*_dt,
                             _mesh.dimension() == 3 ? (*_sys.solutionUDot())(dof_z)*_dt : 0};
  distance_vec += udotvec;

  if (distance_vec.norm() != 0)
    distance_vec += gapOffset(node) * pinfo->_normal;

  const Real gap_size = -1.0 * pinfo->_normal * distance_vec;

  // This is for preventing an increment of pinfo->_locked_this_step for nodes that are
  // captured and released in this function
  bool newly_captured = false;

  // Capture nodes that are newly in contact
  if (update_contact_set && !pinfo->isCaptured() &&
      MooseUtils::absoluteFuzzyGreaterEqual(gap_size, 0.0, 0.0))
  {
    newly_captured = true;
    pinfo->capture();
  }

  if (!pinfo->isCaptured())
    return;

  switch (_model)
  {
    case ExplicitDynamicsContactModel::FRICTIONLESS:
    {
      const Real penalty = getPenalty(node);
      RealVectorValue pen_force(penalty * distance_vec);
      pinfo->_contact_force = pinfo->_normal * (pinfo->_normal * pen_force);
      break;
    }
    case ExplicitDynamicsContactModel::FRICTIONLESS_BALANCE:
      solveImpactEquations(node, pinfo, distance_vec);
      break;
    default:
      mooseError("Invalid or unavailable contact model");
      break;
  }

  if (update_contact_set && pinfo->isCaptured() && !newly_captured)
  {
    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force);
    if (-contact_pressure >= 0.0)
    {
      pinfo->release();
      pinfo->_contact_force.zero();
    }
  }
}

void
ExplicitDynamicsContactConstraint::solveImpactEquations(const Node & node,
                                                        PenetrationInfo * pinfo,
                                                        const RealVectorValue & /*distance_gap*/)
{
  // Momentum balance, uncoupled normal pressure
  // See Heinstein et al, 2000, Contact-impact modeling in explicit transient dynamics.

  dof_id_type dof_x = node.dof_number(_sys.number(), _var_objects[0]->number(), 0);
  dof_id_type dof_y = node.dof_number(_sys.number(), _var_objects[1]->number(), 0);
  dof_id_type dof_z =
      _mesh_dimension == 3 ? node.dof_number(_sys.number(), _var_objects[2]->number(), 0) : 0;

  auto & u_dot = *_sys.solutionUDot();

  // Get lumped mass value
  const auto & diag = _sys.getVector("mass_matrix_diag_inverted");

  Real mass_node = 1.0 / diag(dof_x);
  Real mass_face = computeFaceMass(diag);

  Real mass_eff = (mass_face * mass_node) / (mass_face + mass_node);

  // Include effects of other forces:
  // Initial guess: v_{n-1/2} + dt * M^{-1} * (F^{ext} - F^{int})
  Real velocity_x = u_dot(dof_x) + _dt / mass_node * -1 * _residual_copy(dof_x);
  Real velocity_y = u_dot(dof_y) + _dt / mass_node * -1 * _residual_copy(dof_y);
  Real velocity_z = u_dot(dof_z) + _dt / mass_node * -1 * _residual_copy(dof_z);

  Real n_velocity_x = _neighbor_vel_x[0];
  Real n_velocity_y = _neighbor_vel_y[0];
  Real n_velocity_z = _neighbor_vel_z[0];

  RealVectorValue secondary_velocity(
      velocity_x, velocity_y, _mesh.dimension() == 3 ? velocity_z : 0.0);
  RealVectorValue closest_point_velocity(
      n_velocity_x, n_velocity_y, _mesh.dimension() == 3 ? n_velocity_z : 0.0);
  // Compute initial gap rate
  Real gap_rate = pinfo->_normal * (secondary_velocity - closest_point_velocity);

  // Compute the force increment needed to set gap rate to 0
  RealVectorValue impulse_force = pinfo->_normal * (gap_rate * mass_eff) / _dt;
  pinfo->_contact_force = impulse_force;

  // recalculate velocity to determine gap rate
  velocity_x -= _dt / mass_eff * impulse_force(0);
  velocity_y -= _dt / mass_eff * impulse_force(1);
  velocity_z -= _dt / mass_eff * impulse_force(2);

  // Recalculate gap rate for backwards compatibility
  secondary_velocity = {velocity_x, velocity_y, _mesh.dimension() == 3 ? velocity_z : 0.0};
  gap_rate = pinfo->_normal * (secondary_velocity - closest_point_velocity);
  // gap rate is now always near "0"
  _gap_rate->setNodalValue(gap_rate);
}

Real
ExplicitDynamicsContactConstraint::computeQpSecondaryValue()
{
  // Not used in current implementation.
  return _u_secondary[_qp];
}

Real
ExplicitDynamicsContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  // We use kinematic contact. But adding the residual helps avoid element inversion.
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  Real resid = pinfo->_contact_force(_component);

  switch (type)
  {
    case Moose::Secondary:
      return _test_secondary[_i][_qp] * resid;

    case Moose::Primary:
      return _test_primary[_i][_qp] * -resid;
  }

  return 0.0;
}

Real
ExplicitDynamicsContactConstraint::gapOffset(const Node & node)
{
  Real val = 0;

  if (_has_secondary_gap_offset)
    val += _secondary_gap_offset_var->getNodalValue(node);

  if (_has_mapped_primary_gap_offset)
    val += _mapped_primary_gap_offset_var->getNodalValue(node);

  return val;
}

Real
ExplicitDynamicsContactConstraint::getPenalty(const Node & /*node*/)
{
  // TODO: Include normalized penalty values.
  return _penalty;
}

Real
ExplicitDynamicsContactConstraint::computeFaceMass(const NumericVector<Real> & lumped_mass)
{
  // Initialize face mass to zero
  Real mass_face(0.0);

  // Get the primary side of the current contact
  const auto primary_side = _current_primary->side_ptr(_assembly.neighborSide());

  // Get the dofs on the primary (face) side of the contact
  std::vector<dof_id_type> face_dofs;
  _primary_var.getDofIndices(primary_side.get(), face_dofs);

  // Get average mass of face
  for (const auto dof : face_dofs)
    mass_face += 1.0 / lumped_mass(dof);

  mass_face /= face_dofs.size();

  return mass_face;
}
