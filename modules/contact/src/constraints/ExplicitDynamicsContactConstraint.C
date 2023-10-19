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
  params.addRequiredCoupledVar("nodal_area", "The nodal area");
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
  params.addClassDescription(
      "Apply non-penetration constraints on the mechanical deformation "
      "using a node on face, primary/secondary algorithm, and multiple options "
      "for the physical behavior on the interface and the mathematical "
      "formulation for constraint enforcement");
  params.addParam<MaterialPropertyName>(
      "wave_speed", 0.0, "The wave speed used to solve the impact problem node-wise.");
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
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution()),
    _print_contact_nodes(getParam<bool>("print_contact_nodes")),
    _wave_speed(getMaterialProperty<Real>("wave_speed"))
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
      // pinfo->_contact_force = -pinfo->_normal * (pinfo->_normal * res_vec);
      pinfo->_contact_force = pinfo->_normal * (pinfo->_normal * pen_force);
      break;
    case ExplicitDynamicsContactModel::FRICTIONLESS_ITERATION:
      solveImpactEquations(node, pinfo);
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
ExplicitDynamicsContactConstraint::solveImpactEquations(const Node & /*node*/,
                                                        PenetrationInfo * /*pinfo*/)
{
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
