/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GluedContactConstraint.h"

#include "SystemBase.h"
#include "PenetrationLocator.h"
#include "AddVariableAction.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<GluedContactConstraint>()
{
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());

  InputParameters params = validParams<SparsityBasedContactConstraint>();
  params.addRequiredParam<BoundaryName>("boundary", "The master boundary");
  params.addRequiredParam<BoundaryName>("slave", "The slave boundary");
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
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>(
      "penalty",
      1e8,
      "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0.0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance",
                        "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>(
      "normal_smoothing_distance",
      "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method",
                               "Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<Real>("tension_release",
                        0.0,
                        "Tension release threshold.  A node in contact "
                        "will not be released if its tensile load is below "
                        "this value.  Must be positive.");

  params.addParam<std::string>("formulation", "default", "The contact formulation");
  return params;
}

GluedContactConstraint::GluedContactConstraint(const InputParameters & parameters)
  : SparsityBasedContactConstraint(parameters),
    _component(getParam<unsigned int>("component")),
    _model(ContactMaster::contactModel(getParam<std::string>("model"))),
    _formulation(ContactMaster::contactFormulation(getParam<std::string>("formulation"))),
    _penalty(getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _tension_release(getParam<Real>("tension_release")),
    _updateContactSet(true),
    _residual_copy(_sys.residualGhosted()),
    _vars(3, libMesh::invalid_uint),
    _nodal_area_var(getVar("nodal_area", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution())
{
  if (parameters.isParamValid("tangential_tolerance"))
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));

  if (parameters.isParamValid("normal_smoothing_distance"))
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));

  if (parameters.isParamValid("normal_smoothing_method"))
    _penetration_locator.setNormalSmoothingMethod(
        parameters.get<std::string>("normal_smoothing_method"));

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

  _penetration_locator.setUpdate(false);
}

void
GluedContactConstraint::timestepSetup()
{
  if (_component == 0)
  {
    updateContactSet(true);
    _updateContactSet = false;
  }
}

void
GluedContactConstraint::jacobianSetup()
{
  if (_component == 0)
  {
    if (_updateContactSet)
      updateContactSet();

    _updateContactSet = true;
  }
}

void
GluedContactConstraint::updateContactSet(bool beginning_of_step)
{
  auto & has_penetrated = _penetration_locator._has_penetrated;

  for (auto & pinfo_pair : _penetration_locator._penetration_info)
  {
    PenetrationInfo * pinfo = pinfo_pair.second;

    // Skip this pinfo if there are no DOFs on this node.
    if (!pinfo || pinfo->_node->n_comp(_sys.number(), _vars[_component]) < 1)
      continue;

    const dof_id_type slave_node_num = pinfo_pair.first;
    auto hpit = has_penetrated.find(slave_node_num);

    if (beginning_of_step)
    {
      pinfo->_starting_elem = pinfo->_elem;
      pinfo->_starting_side_num = pinfo->_side_num;
      pinfo->_starting_closest_point_ref = pinfo->_closest_point_ref;
    }

    if (pinfo->_distance >= 0)
      if (hpit == has_penetrated.end())
        has_penetrated.insert(slave_node_num);
  }
}

bool
GluedContactConstraint::shouldApply()
{
  auto hpit = _penetration_locator._has_penetrated.find(_current_node->id());
  return hpit != _penetration_locator._has_penetrated.end();
}

Real
GluedContactConstraint::computeQpSlaveValue()
{
  return _u_slave[_qp];
}

Real
GluedContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Slave:
    {
      PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
      Real distance = (*_current_node)(_component)-pinfo->_closest_point(_component);
      Real pen_force = _penalty * distance;

      Real resid = pen_force;
      pinfo->_contact_force(_component) = resid;
      pinfo->_mech_status = PenetrationInfo::MS_STICKING;

      return _test_slave[_i][_qp] * resid;
    }

    case Moose::Master:
    {
      PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

      dof_id_type dof_number = _current_node->dof_number(0, _vars[_component], 0);
      Real resid = _residual_copy(dof_number);

      pinfo->_contact_force(_component) = -resid;
      pinfo->_mech_status = PenetrationInfo::MS_STICKING;

      return _test_master[_i][_qp] * resid;
    }
  }

  return 0.0;
}

Real
GluedContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::SlaveSlave:
      return _penalty * _phi_slave[_j][_qp] * _test_slave[_i][_qp];

    case Moose::SlaveMaster:
      return _penalty * -_phi_master[_j][_qp] * _test_slave[_i][_qp];

    case Moose::MasterSlave:
    {
      const double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                            _connected_dof_indices[_j]);
      return slave_jac * _test_master[_i][_qp];
    }

    case Moose::MasterMaster:
      return 0.0;
  }

  return 0.0;
}

Real
GluedContactConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                 unsigned int /*jvar*/)
{
  Real retVal = 0.0;

  switch (type)
  {
    case Moose::SlaveSlave:
    case Moose::SlaveMaster:
    case Moose::MasterMaster:
      break;

    case Moose::MasterSlave:
    {
      const double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars[_component], 0),
                                            _connected_dof_indices[_j]);
      retVal = slave_jac * _test_master[_i][_qp];
      break;
    }
  }

  return retVal;
}
