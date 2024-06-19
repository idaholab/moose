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
  params.addRequiredCoupledVar("nodal_area", "The nodal area.");
  params.addRequiredCoupledVar("nodal_density", "The nodal density.");
  params.addRequiredCoupledVar("nodal_wave_speed", "The nodal wave speed.");
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
  params.addParam<bool>("overwrite_current_solution",
                        false,
                        "Whether to overwrite the position of contact boundaries with the velocity "
                        "computed with the contact algorithm.");
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
    _nodal_area_var(getVar("nodal_area", 0)),
    _nodal_density_var(getVar("nodal_density", 0)),
    _nodal_wave_speed_var(getVar("nodal_wave_speed", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution()),
    _penalty(getParam<Real>("penalty")),
    _print_contact_nodes(getParam<bool>("print_contact_nodes")),
    _residual_copy(_sys.residualGhosted()),
    _neighbor_density(getNeighborMaterialPropertyByName<Real>("density")),
    _neighbor_wave_speed(getNeighborMaterialPropertyByName<Real>("wave_speed")),
    _gap_rate(&writableVariable("gap_rate")),
    _neighbor_vel_x(isCoupled("vel_x") ? coupledNeighborValue("vel_x") : _zero),
    _neighbor_vel_y(isCoupled("vel_y") ? coupledNeighborValue("vel_y") : _zero),
    _neighbor_vel_z((_mesh.dimension() == 3 && isCoupled("vel_z")) ? coupledNeighborValue("vel_z")
                                                                   : _zero),
    _overwrite_current_solution(getParam<bool>("overwrite_current_solution"))
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
  if (distance_vec.norm() != 0)
    distance_vec += gapOffset(node) * pinfo->_normal * distance_vec.unit() * distance_vec.unit();

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

  const Real penalty = getPenalty(node);

  RealVectorValue pen_force(penalty * distance_vec);

  switch (_model)
  {
    case ExplicitDynamicsContactModel::FRICTIONLESS:
      pinfo->_contact_force = pinfo->_normal * (pinfo->_normal * pen_force);
      break;
    case ExplicitDynamicsContactModel::FRICTIONLESS_BALANCE:
      solveImpactEquations(node, pinfo, distance_vec);
      break;
    default:
      mooseError("Invalid or unavailable contact model");
      break;
  }

  if (update_contact_set && pinfo->isCaptured() && !newly_captured)
  {
    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force) / nodalArea(node);
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

  const auto nodal_area = nodalArea(node);

  dof_id_type dof_wave_speed =
      node.dof_number(_aux_system.number(), _nodal_wave_speed_var->number(), 0);
  const Real wave_speed_secondary = (*_aux_solution)(dof_wave_speed);

  dof_id_type dof_density = node.dof_number(_aux_system.number(), _nodal_density_var->number(), 0);
  const Real density_secondary = (*_aux_solution)(dof_density);

  Real mass_contact_pressure(0.0);

  Real gap_rate(0.0);

  mass_contact_pressure =
      density_secondary * _neighbor_density[0] * wave_speed_secondary * _neighbor_wave_speed[0];
  mass_contact_pressure /=
      (density_secondary * wave_speed_secondary + _neighbor_density[0] * _neighbor_wave_speed[0]);
  mass_contact_pressure *= nodal_area;

  dof_id_type dof_x = node.dof_number(_sys.number(), _var_objects[0]->number(), 0);
  dof_id_type dof_y = node.dof_number(_sys.number(), _var_objects[1]->number(), 0);
  dof_id_type dof_z = node.dof_number(_sys.number(), _var_objects[2]->number(), 0);

  auto & u_dot = *_sys.solutionUDot();
  auto & u_old = _sys.solutionOld();
  auto & u_old_old = _sys.solutionOlder();
  // Mass proxy for secondary node.
  const Real mass_proxy = density_secondary * wave_speed_secondary * _dt * nodal_area;

  // Include effects of other forces:
  // Initial guess: v_{n-1/2} + dt * M^{-1} * (F^{ext} - F^{int})
  Real velocity_x = u_dot(dof_x) + _dt / mass_proxy * _residual_copy(dof_x);
  Real velocity_y = u_dot(dof_y) + _dt / mass_proxy * _residual_copy(dof_y);
  Real velocity_z = u_dot(dof_z) + _dt / mass_proxy * _residual_copy(dof_z);

  Real n_velocity_x = _neighbor_vel_x[0];
  Real n_velocity_y = _neighbor_vel_y[0];
  Real n_velocity_z = _neighbor_vel_z[0];

  RealVectorValue secondary_velocity(
      velocity_x, velocity_y, _mesh.dimension() == 3 ? velocity_z : 0.0);
  RealVectorValue closest_point_velocity(
      n_velocity_x, n_velocity_y, _mesh.dimension() == 3 ? n_velocity_z : 0.0);
  gap_rate = pinfo->_normal * (secondary_velocity - closest_point_velocity);

  // Prepare equilibrium loop
  bool is_converged(false);
  unsigned int iteration_no(0);
  const unsigned int max_no_iterations(20000);

  // Initialize augmented iteration variable
  Real gap_rate_old(0.0);
  Real force_increment(0.0);
  Real force_increment_old(0.0);
  Real lambda_iteration(0);

  while (!is_converged && iteration_no < max_no_iterations)
  {
    // Start a loop until we converge on normal contact forces
    gap_rate_old = gap_rate;
    gap_rate = pinfo->_normal * (secondary_velocity - closest_point_velocity);
    force_increment_old = force_increment;

    force_increment = mass_contact_pressure * gap_rate;

    velocity_x -= _dt / mass_proxy * (pinfo->_normal(0) * (force_increment));
    velocity_y -= _dt / mass_proxy * (pinfo->_normal(1) * (force_increment));
    velocity_z -= _dt / mass_proxy * (pinfo->_normal(2) * (force_increment));

    // Let's not modify the neighbor velocity, but apply the corresponding force.
    // TODO: Update for multi-body impacts
    // n_velocity_x = n_velocity_x;
    // n_velocity_y = n_velocity_y;
    // n_velocity_z = n_velocity_z;

    secondary_velocity = {velocity_x, velocity_y, _mesh.dimension() == 3 ? velocity_z : 0.0};
    closest_point_velocity = {
        n_velocity_x, n_velocity_y, _mesh.dimension() == 3 ? n_velocity_z : 0.0};

    // Convergence check
    lambda_iteration += force_increment;

    const Real relative_error = (force_increment - force_increment_old) / force_increment;
    const Real absolute_error = std::abs(force_increment);

    if (std::abs(relative_error) < TOLERANCE * TOLERANCE || absolute_error < TOLERANCE ||
        (gap_rate_old) * (gap_rate) < 0.0)
      is_converged = true;
    else
      iteration_no++;
  }

  _gap_rate->setNodalValue(gap_rate);

  u_old.set(dof_x, u_old_old(dof_x) + velocity_x * _dt);
  u_old.set(dof_y, u_old_old(dof_y) + velocity_y * _dt);
  u_old.set(dof_z, u_old_old(dof_z) + velocity_z * _dt);

  _dof_to_position[dof_x] = u_old_old(dof_x) + velocity_x * _dt;
  _dof_to_position[dof_y] = u_old_old(dof_y) + velocity_y * _dt;
  _dof_to_position[dof_z] = u_old_old(dof_z) + velocity_z * _dt;

  pinfo->_contact_force = pinfo->_normal * lambda_iteration;
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
ExplicitDynamicsContactConstraint::nodalArea(const Node & node)
{
  dof_id_type dof = node.dof_number(_aux_system.number(), _nodal_area_var->number(), 0);

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
ExplicitDynamicsContactConstraint::getPenalty(const Node & /*node*/)
{
  // TODO: Include normalized penalty values.
  return _penalty;
}

void
ExplicitDynamicsContactConstraint::overwriteBoundaryVariables(NumericVector<Number> & soln,
                                                              const Node & secondary_node) const
{
  if (_component == 0 && _overwrite_current_solution)
  {
    dof_id_type dof_x = secondary_node.dof_number(_sys.number(), _var_objects[0]->number(), 0);
    dof_id_type dof_y = secondary_node.dof_number(_sys.number(), _var_objects[1]->number(), 0);
    dof_id_type dof_z = secondary_node.dof_number(_sys.number(), _var_objects[2]->number(), 0);

    if (_dof_to_position.find(dof_x) != _dof_to_position.end())
    {
      const auto & position_x = libmesh_map_find(_dof_to_position, dof_x);
      const auto & position_y = libmesh_map_find(_dof_to_position, dof_y);
      const auto & position_z = libmesh_map_find(_dof_to_position, dof_z);

      soln.set(dof_x, position_x);
      soln.set(dof_y, position_y);
      soln.set(dof_z, position_z);
    }
  }
}
