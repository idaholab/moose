//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ExplicitDynamicsOverwrite.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "PenetrationLocator.h"
#include "NearestNodeLocator.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "MathUtils.h"
#include "Executioner.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("MooseApp", ExplicitDynamicsOverwrite);

InputParameters
ExplicitDynamicsOverwrite::validParams()
{
  InputParameters params = NodeFaceConstraint::validParams();

  params.addParam<MooseEnum>("model",
                             MooseEnum("frictionless frictionless_balance", "frictionless"),
                             "The contact model to use");
  params.addCoupledVar("vel_x", "x-component of velocity.");
  params.addCoupledVar("vel_y", "y-component of velocity.");
  params.addCoupledVar("vel_z", "z-component of velocity.");

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
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addCoupledVar("gap_rate", "Gap rate for output (writable auxiliary variable).");
  params.addClassDescription(
      "Apply non-penetration constraints on the mechanical deformation in explicit dynamics "
      "using a node on face formulation by solving uncoupled momentum-balance equations.");
  return params;
}

ExplicitDynamicsOverwrite::ExplicitDynamicsOverwrite(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _component(getParam<unsigned int>("component")),
    _mesh_dimension(_mesh.dimension()),
    _vars(3, libMesh::invalid_uint),
    _var_objects(3, nullptr),
    _has_secondary_gap_offset(isCoupled("secondary_gap_offset")),
    _secondary_gap_offset_var(_has_secondary_gap_offset ? getVar("secondary_gap_offset", 0)
                                                        : nullptr),
    _has_mapped_primary_gap_offset(isCoupled("mapped_primary_gap_offset")),
    _mapped_primary_gap_offset_var(
        _has_mapped_primary_gap_offset ? getVar("mapped_primary_gap_offset", 0) : nullptr),
    _overwrite_current_solution(true),
    _gap_rate(&writableVariable("gap_rate"))
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

  bool is_correct =
      (isCoupled("vel_x") && isCoupled("vel_y") && _mesh.dimension() == 2) ||
      (isCoupled("vel_x") && isCoupled("vel_y") && isCoupled("vel_z") && _mesh.dimension() == 3);

  if (!is_correct)
    paramError("vel_x",
               "Velocities vel_x and vel_y (also vel_z in three dimensions) need to be provided "
               "for the 'balance' option of solving normal contact in explicit dynamics.");
}

void
ExplicitDynamicsOverwrite::timestepSetup()
{
  if (_component == 0)
    _dof_to_gap.clear();
}

bool
ExplicitDynamicsOverwrite::shouldApply()
{
  if (_current_node->processor_id() != _fe_problem.processor_id())
    return false;

  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != nullptr)
      if (_component == 0)
        computeContactForce(*_current_node, pinfo);
  }
  return false;
}

void
ExplicitDynamicsOverwrite::computeContactForce(const Node & node, PenetrationInfo * pinfo)
{
  pinfo->_contact_force.zero();
  RealVectorValue distance_vec(node - pinfo->_closest_point);
  if (distance_vec.norm() != 0)
    distance_vec += gapOffset(node) * pinfo->_normal * distance_vec.unit() * distance_vec.unit();

  const Real gap_size = -1.0 * pinfo->_normal * distance_vec;

  if (!pinfo->isCaptured() && MooseUtils::absoluteFuzzyGreaterEqual(gap_size, 0.0, 0.0))
  {
    pinfo->capture();
  }
  dof_id_type dof_x = node.dof_number(_sys.number(), _var_objects[0]->number(), 0);

  _dof_to_gap[dof_x] = gap_size;
  _gap_rate->setNodalValue(-1.23456);
}

Real
ExplicitDynamicsOverwrite::gapOffset(const Node & node)
{
  Real val = 0;

  if (_has_secondary_gap_offset)
    val += _secondary_gap_offset_var->getNodalValue(node);

  if (_has_mapped_primary_gap_offset)
    val += _mapped_primary_gap_offset_var->getNodalValue(node);

  return val;
}

Real
ExplicitDynamicsOverwrite::computeQpSecondaryValue()
{
  // Not used in current implementation.
  return -9999.9;
}

Real
ExplicitDynamicsOverwrite::computeQpResidual(Moose::ConstraintType /*type*/)
{
  return 0.0;
}

void
ExplicitDynamicsOverwrite::overwriteBoundaryVariables(NumericVector<Number> & soln,
                                                      const Node & secondary_node) const
{
  dof_id_type dof_x = secondary_node.dof_number(_sys.number(), _var_objects[0]->number(), 0);

  if (_component == 0 && _overwrite_current_solution && libmesh_map_find(_dof_to_gap, dof_x) >= 0.0)
  {
    dof_id_type dof_z = secondary_node.dof_number(_sys.number(), _var_objects[2]->number(), 0);
    // Blatantly attached the 'cube' bottom surface to zero.
    // This doesn't do anything physical, just overwrites the z displacement to give the appearance
    // of contact.
    if (_dof_to_gap.find(dof_x) != _dof_to_gap.end())
      soln.set(dof_z, 0.0);
  }
}
