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

#include "UpdateNodalAuxVariable.h"

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
  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
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
      "print_contact_nodes", false, "Whether to print the number of nodes in contact.");
  // params.addParam<std::vector<UserObjectName>>(
  //     "vel_uos", "List of nodal user objects to update the velocity vector.");
  params.addClassDescription(
      "Apply non-penetration constraints on the mechanical deformation "
      "using a node on face, primary/secondary algorithm, and multiple options "
      "for the physical behavior on the interface and the mathematical "
      "formulation for constraint enforcement");

  return params;
}

Threads::spin_mutex ExplicitDynamicsContactConstraint::_contact_set_mutex;

ExplicitDynamicsContactConstraint::ExplicitDynamicsContactConstraint(
    const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, getBoundaryIDs()),
    _displaced_problem(parameters.get<FEProblemBase *>("_fe_problem_base")->getDisplacedProblem()),
    _component(getParam<unsigned int>("component")),
    _model(getParam<MooseEnum>("model").getEnum<ExplicitDynamicsContactModel>()),
    _normalize_penalty(getParam<bool>("normalize_penalty")),
    _tension_release(getParam<Real>("tension_release")),
    _capture_tolerance(getParam<Real>("capture_tolerance")),
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
    _print_contact_nodes(getParam<bool>("print_contact_nodes")),
    _neighbor_density(getNeighborMaterialPropertyByName<Real>("density")),
    _neighbor_wave_speed(getNeighborMaterialPropertyByName<Real>("wave_speed")),
    _vel_x(&writableVariable("vel_x")),
    _vel_y(&writableVariable("vel_y")),
    _vel_z(&writableVariable("vel_z")),
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

  mooseInfo("This is the constructor of the explicit dynamics contact constraint.");

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
        // delete pinfo;
        // pinfo = nullptr;
        // continue;
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
      {
        in_contact = true;

        Threads::spin_mutex::scoped_lock lock(_contact_set_mutex);
        _current_contact_state.insert(_current_node->id());
      }
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
      MooseUtils::absoluteFuzzyGreaterEqual(gap_size, 0.0, _capture_tolerance))
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

  if (update_contact_set && pinfo->isCaptured() && !newly_captured && _tension_release >= 0.0)
  {
    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force) / nodalArea(node);
    if (-contact_pressure >= _tension_release)
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
  // Stab at momentum balance, uncoupled normal pressure
  // See Heinstein et al, 2000, Contact-impact modeling in explicit transient dynamics.

  // Secondary surface
  const auto nodal_area = nodalArea(node);

  dof_id_type dof_wave_speed =
      node.dof_number(_aux_system.number(), _nodal_wave_speed_var->number(), 0);
  const Real wave_speed_secondary = (*_aux_solution)(dof_wave_speed);

  dof_id_type dof_density = node.dof_number(_aux_system.number(), _nodal_density_var->number(), 0);
  const Real density_secondary = (*_aux_solution)(dof_density);

  Real contact_pressure_balance(0.0);
  Real mass_contact_pressure(0.0);

  Real gap_rate(0.0);
  // Real gap(0.0);

  mass_contact_pressure =
      density_secondary * _neighbor_density[0] * wave_speed_secondary * _neighbor_wave_speed[0];
  mass_contact_pressure /=
      (density_secondary * wave_speed_secondary + _neighbor_density[0] * _neighbor_wave_speed[0]);
  mass_contact_pressure *= nodal_area;

  // Prepare equilibrium loop

  bool is_converged(false);
  const unsigned int max_no_iterations(20000);
  unsigned int iteration_no(0);

  // for (unsigned int i = 0; i < _ndisp; ++i)
  // {
  //   // translational velocities and accelerations
  //   unsigned int dof_index_0 = node[0]->dof_number(nonlinear_sys.number(), _disp_num[i], 0);
  //   _vel_0(i) = vel(dof_index_0);
  // }

  Real velocity_x = dynamic_cast<MooseVariableFE<Real> *>(_vel_x)->getNodalValue(node);
  Real velocity_y = dynamic_cast<MooseVariableFE<Real> *>(_vel_y)->getNodalValue(node);
  Real velocity_z = dynamic_cast<MooseVariableFE<Real> *>(_vel_z)->getNodalValue(node);

  Real n_velocity_x = _neighbor_vel_x[0];
  Real n_velocity_y = _neighbor_vel_y[0];
  Real n_velocity_z = _neighbor_vel_z[0];

  Real contact_pressure_old(0.0);

  // Mass proxy for secondary node.
  const Real mass_proxy = density_secondary * wave_speed_secondary * _dt * nodal_area;
  const Real n_mass_proxy = _neighbor_density[0] * _neighbor_wave_speed[0] * _dt * nodal_area;

  while (!is_converged && iteration_no < max_no_iterations)
  {
    // Start a loop until we converge on normal contact forces
    RealVectorValue secondary_velocity(
        velocity_x, velocity_y, _mesh.dimension() == 3 ? velocity_z : 0.0);
    RealVectorValue closest_point_velocity(
        n_velocity_x, n_velocity_y, _mesh.dimension() == 3 ? n_velocity_z : 0.0);
    gap_rate = pinfo->_normal * (secondary_velocity - closest_point_velocity);

    contact_pressure_balance = mass_contact_pressure * gap_rate;

    velocity_x = velocity_x - _dt / mass_proxy * pinfo->_normal(0) *
                                  (contact_pressure_balance - contact_pressure_old);
    velocity_y = velocity_y - _dt / mass_proxy * pinfo->_normal(1) *
                                  (contact_pressure_balance - contact_pressure_old);
    velocity_z = velocity_z - _dt / mass_proxy * pinfo->_normal(2) *
                                  (contact_pressure_balance - contact_pressure_old);

    n_velocity_x = n_velocity_x - _dt / n_mass_proxy * pinfo->_normal(0) *
                                      (contact_pressure_balance - contact_pressure_old);
    n_velocity_y = n_velocity_y - _dt / n_mass_proxy * pinfo->_normal(1) *
                                      (contact_pressure_balance - contact_pressure_old);
    n_velocity_z = n_velocity_z - _dt / n_mass_proxy * pinfo->_normal(2) *
                                      (contact_pressure_balance - contact_pressure_old);

    // Convergence check:
    const Real relative_error =
        (contact_pressure_balance - contact_pressure_old) / contact_pressure_balance;
    const Real absolute_error = std::abs(contact_pressure_balance - contact_pressure_old);

    // Moose::out << "For error, current vs old is: " << contact_pressure_balance << ", "
    //            << contact_pressure_old << "\n";

    contact_pressure_old = contact_pressure_balance;

    if (std::abs(relative_error) < TOLERANCE || absolute_error < TOLERANCE)
    {
      Moose::out << "Relative error of loop: " << std::abs(relative_error) << "\n";
      is_converged = true;
    }
    else
      iteration_no++;
  }

  auto & u_dot = *_sys.solutionUDot();
  dof_id_type dof_x = node.dof_number(_sys.number(), _var_objects[0]->number(), 0);
  dof_id_type dof_y = node.dof_number(_sys.number(), _var_objects[1]->number(), 0);
  dof_id_type dof_z = node.dof_number(_sys.number(), _var_objects[2]->number(), 0);

  Moose::out << "Dofs set up in this nodal constraint are: " << dof_x << ", " << dof_y << ", and "
             << dof_z << "\n";

  // Set velocities on contact interface according to local, converged solution.
  u_dot.set(dof_x, velocity_x);
  u_dot.set(dof_y, velocity_y);
  u_dot.set(dof_z, velocity_z);
  u_dot.close();

  Moose::out << "Total iterations: " << iteration_no << "\n";
  Moose::out << "vel z *before* local update: "
             << dynamic_cast<MooseVariableFE<Real> *>(_vel_z)->getNodalValue(node) << "\n";
  Moose::out << "vel z locally after update update: " << velocity_z << "\n";

  // NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase(_sys.number());

  if (true)
  {
    numeric_index_type dof_index_x = node.dof_number(_aux_system.number(), _vel_x->number(), 0);
    numeric_index_type dof_index_y = node.dof_number(_aux_system.number(), _vel_y->number(), 0);
    numeric_index_type dof_index_z = node.dof_number(_aux_system.number(), _vel_z->number(), 0);

    _vel_x->setNodalValue(velocity_x, dof_index_x);
    _vel_y->setNodalValue(velocity_y, dof_index_y);
    _vel_z->setNodalValue(velocity_z, dof_index_z);

    // vel(dof_index_x) = velocity_x;
    // vel(dof_index_y) = velocity_y;
    // vel(dof_index_z) = velocity_z;
  }

  Moose::out << "vel z *after* local update: "
             << dynamic_cast<MooseVariableFE<Real> *>(_vel_z)->nodalValue() << "\n";

  // The gap rate can probably be saved in pinfo by adequately evaluating the velocity variable at
  // the corresponding points. Let's do that.
  if (contact_pressure_balance < 0.0)
    pinfo->_contact_force = pinfo->_normal * contact_pressure_balance;
  else
    pinfo->_contact_force = 0.0;
}

Real
ExplicitDynamicsContactConstraint::computeQpSecondaryValue()
{
  return _u_secondary[_qp];
}

Real
ExplicitDynamicsContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
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
  return 1.0e3;
}

bool
ExplicitDynamicsContactConstraint::getCoupledVarComponent(unsigned int var_num,
                                                          unsigned int & component)
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
ExplicitDynamicsContactConstraint::residualEnd()
{
  if (_component == 0 && (_print_contact_nodes))
  {
    _communicator.set_union(_current_contact_state);
    if (_print_contact_nodes)
    {
      if (_current_contact_state == _old_contact_state)
        _console << "Unchanged contact state. " << _current_contact_state.size()
                 << " nodes in contact.\n";
      else
        _console << "Changed contact state. " << _current_contact_state.size()
                 << " nodes in contact.\n";
    }

    _old_contact_state.swap(_current_contact_state);
    _current_contact_state.clear();
  }
}
