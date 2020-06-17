//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ContactMaster.h"
#include "NodalArea.h"
#include "SystemBase.h"
#include "PenetrationInfo.h"
#include "MooseMesh.h"
#include "Executioner.h"
#include "ContactAction.h"

#include "libmesh/sparse_matrix.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("ContactApp", ContactMaster);

InputParameters
ContactMaster::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params += ContactAction::commonParameters();

  params.addRequiredParam<BoundaryName>("boundary", "The primary boundary");
  params.addRequiredParam<BoundaryName>("secondary", "The secondary boundary");
  params.addRequiredParam<unsigned int>("component",
                                        "An integer corresponding to the direction "
                                        "the variable this kernel acts in. (0 for x, "
                                        "1 for y, 2 for z)");

  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

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
  return params;
}

ContactMaster::ContactMaster(const InputParameters & parameters)
  : DiracKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _model(getParam<MooseEnum>("model").getEnum<ContactModel>()),
    _formulation(getParam<MooseEnum>("formulation").getEnum<ContactFormulation>()),
    _normalize_penalty(getParam<bool>("normalize_penalty")),
    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("boundary"),
                              getParam<BoundaryName>("secondary"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order")))),
    _penalty(getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _tension_release(getParam<Real>("tension_release")),
    _capture_tolerance(getParam<Real>("capture_tolerance")),
    _residual_copy(_sys.residualGhosted()),
    _mesh_dimension(_mesh.dimension()),
    _vars(3, libMesh::invalid_uint),
    _nodal_area_var(getVar("nodal_area", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution())
{
  if (isParamValid("displacements"))
  {
    // modern parameter scheme for displacements
    for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
      _vars[i] = coupled("displacements", i);
  }
  else
  {
    // Legacy parameter scheme for displacements
    if (isParamValid("disp_x"))
      _vars[0] = coupled("disp_x");
    if (isParamValid("disp_y"))
      _vars[1] = coupled("disp_y");
    if (isParamValid("disp_z"))
      _vars[2] = coupled("disp_z");

    mooseDeprecated("use the `displacements` parameter rather than the `disp_*` parameters (those "
                    "will go away with the deprecation of the Solid Mechanics module).");
  }

  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<MooseEnum>("normal_smoothing_method"));

  if (_model == ContactModel::GLUED ||
      (_model == ContactModel::COULOMB && _formulation == ContactFormulation::KINEMATIC))
    _penetration_locator.setUpdate(false);

  if (_friction_coefficient < 0)
    mooseError("The friction coefficient must be nonnegative");
}

void
ContactMaster::timestepSetup()
{
  if (_component == 0)
    updateContactStatefulData();
}

void
ContactMaster::updateContactStatefulData()
{
  std::map<dof_id_type, PenetrationInfo *>::iterator
      it = _penetration_locator._penetration_info.begin(),
      end = _penetration_locator._penetration_info.end();
  for (; it != end; ++it)
  {
    PenetrationInfo * pinfo = it->second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    pinfo->_locked_this_step = 0;
    pinfo->_starting_elem = it->second->_elem;
    pinfo->_starting_side_num = it->second->_side_num;
    pinfo->_starting_closest_point_ref = it->second->_closest_point_ref;
    pinfo->_contact_force_old = pinfo->_contact_force;
    pinfo->_accumulated_slip_old = pinfo->_accumulated_slip;
    pinfo->_frictional_energy_old = pinfo->_frictional_energy;
  }
}

void
ContactMaster::addPoints()
{
  _point_to_info.clear();

  std::map<dof_id_type, PenetrationInfo *>::iterator
      it = _penetration_locator._penetration_info.begin(),
      end = _penetration_locator._penetration_info.end();

  for (; it != end; ++it)
  {
    PenetrationInfo * pinfo = it->second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    bool is_nonlinear =
        _subproblem.getMooseApp().getExecutioner()->feProblem().computingNonlinearResid();

    if (_component == 0)
      computeContactForce(pinfo, is_nonlinear);

    if (pinfo->isCaptured())
    {
      addPoint(pinfo->_elem, pinfo->_closest_point);
      _point_to_info[pinfo->_closest_point] = pinfo;
    }
  }
}

void
ContactMaster::computeContactForce(PenetrationInfo * pinfo, bool update_contact_set)
{
  const Node * node = pinfo->_node;

  RealVectorValue res_vec;
  // Build up residual vector
  for (unsigned int i = 0; i < _mesh_dimension; ++i)
  {
    dof_id_type dof_number = node->dof_number(0, _vars[i], 0);
    res_vec(i) = _residual_copy(dof_number);
  }

  RealVectorValue distance_vec(_mesh.nodeRef(node->id()) - pinfo->_closest_point);
  const Real gap_size = -1. * pinfo->_normal * distance_vec;

  // This is for preventing an increment of pinfo->_locked_this_step for nodes that are
  // captured and released in this function
  bool newly_captured = false;

  // Capture
  if (update_contact_set && !pinfo->isCaptured() &&
      MooseUtils::absoluteFuzzyGreaterEqual(gap_size, 0, _capture_tolerance))
  {
    newly_captured = true;
    pinfo->capture();

    // Increment the lock count every time the node comes back into contact from not being in
    // contact.
    if (_formulation == ContactFormulation::KINEMATIC)
      ++pinfo->_locked_this_step;
  }

  if (!pinfo->isCaptured())
    return;

  const Real area = nodalArea(*pinfo);

  RealVectorValue pen_force(_penalty * distance_vec);
  if (_normalize_penalty)
    pen_force *= area;

  if (_model == ContactModel::FRICTIONLESS)
  {
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
  }
  else if (_model == ContactModel::COULOMB && _formulation == ContactFormulation::PENALTY)
  {
    distance_vec =
        pinfo->_incremental_slip +
        (pinfo->_normal * (_mesh.nodeRef(node->id()) - pinfo->_closest_point)) * pinfo->_normal;
    pen_force = _penalty * distance_vec;
    if (_normalize_penalty)
      pen_force *= area;

    // Frictional capacity
    // const Real capacity( _friction_coefficient * (pen_force * pinfo->_normal < 0 ? -pen_force *
    // pinfo->_normal : 0) );
    const Real capacity(_friction_coefficient *
                        (res_vec * pinfo->_normal > 0 ? res_vec * pinfo->_normal : 0));

    // Elastic predictor
    pinfo->_contact_force =
        pen_force +
        (pinfo->_contact_force_old - pinfo->_normal * (pinfo->_normal * pinfo->_contact_force_old));
    RealVectorValue contact_force_normal((pinfo->_contact_force * pinfo->_normal) * pinfo->_normal);
    RealVectorValue contact_force_tangential(pinfo->_contact_force - contact_force_normal);

    // Tangential magnitude of elastic predictor
    const Real tan_mag(contact_force_tangential.norm());

    if (tan_mag > capacity)
    {
      pinfo->_contact_force = contact_force_normal + capacity * contact_force_tangential / tan_mag;
      pinfo->_mech_status = PenetrationInfo::MS_SLIPPING;
    }
    else
    {
      pinfo->_mech_status = PenetrationInfo::MS_STICKING;
    }
  }
  else if (_model == ContactModel::GLUED ||
           (_model == ContactModel::COULOMB && _formulation == ContactFormulation::KINEMATIC))
  {
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
  }
  else
  {
    mooseError("Invalid or unavailable contact model");
  }

  // Release
  if (update_contact_set && _model != ContactModel::GLUED && pinfo->isCaptured() &&
      !newly_captured && _tension_release >= 0 && pinfo->_locked_this_step < 2)
  {
    const Real contact_pressure = -(pinfo->_normal * pinfo->_contact_force) / area;
    if (-contact_pressure >= _tension_release)
    {
      pinfo->release();
      pinfo->_contact_force.zero();
    }
  }

  if (_formulation == ContactFormulation::AUGMENTED_LAGRANGE && pinfo->isCaptured())
    pinfo->_lagrange_multiplier -= getPenalty(*pinfo) * gap_size;
}

Real
ContactMaster::computeQpResidual()
{
  PenetrationInfo * pinfo = _point_to_info[_current_point];
  Real resid = -pinfo->_contact_force(_component);
  return _test[_i][_qp] * resid;
}

Real
ContactMaster::computeQpJacobian()
{

  PenetrationInfo * pinfo = _point_to_info[_current_point];
  Real penalty = _penalty;
  if (_normalize_penalty)
    penalty *= nodalArea(*pinfo);

  switch (_model)
  {
    case ContactModel::FRICTIONLESS:
      switch (_formulation)
      {
        case ContactFormulation::KINEMATIC:
          return 0;
          break;
        case ContactFormulation::PENALTY:
        case ContactFormulation::AUGMENTED_LAGRANGE:
          return _test[_i][_qp] * penalty * _phi[_j][_qp] * pinfo->_normal(_component) *
                 pinfo->_normal(_component);
          break;
        default:
          mooseError("Invalid contact formulation");
          break;
      }
      break;
    case ContactModel::GLUED:
    case ContactModel::COULOMB:
      switch (_formulation)
      {
        case ContactFormulation::KINEMATIC:
          return 0;
          break;
        case ContactFormulation::PENALTY:
        case ContactFormulation::AUGMENTED_LAGRANGE:
          return _test[_i][_qp] * penalty * _phi[_j][_qp];
          break;
        default:
          mooseError("Invalid contact formulation");
          break;
      }
      break;
    default:
      mooseError("Invalid or unavailable contact model");
      break;
  }

  return 0;
  /*
    if (_i != _j)
      return 0;

    Node * node = pinfo->_node;

    RealVectorValue jac_vec;

    // Build up jac vector
    for (unsigned int i=0; i<_dim; i++)
    {
      long int dof_number = node->dof_number(0, _vars[i], 0);
      jac_vec(i) = _jacobian_copy(dof_number, dof_number);
    }

    Real jac_mag = pinfo->_normal * jac_vec;

    return _test[_i][_qp]*pinfo->_normal(_component)*jac_mag;
  */
}

Real
ContactMaster::nodalArea(PenetrationInfo & pinfo)
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
ContactMaster::getPenalty(PenetrationInfo & pinfo)
{
  Real penalty = _penalty;
  if (_normalize_penalty)
    penalty *= nodalArea(pinfo);

  return penalty;
}
