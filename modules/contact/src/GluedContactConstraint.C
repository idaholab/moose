/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GluedContactConstraint.h"

#include "SystemBase.h"
#include "PenetrationLocator.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

template<>
InputParameters validParams<GluedContactConstraint>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<SparsityBasedContactConstraint>();
  params.addRequiredParam<BoundaryName>("boundary", "The master boundary");
  params.addRequiredParam<BoundaryName>("slave", "The slave boundary");
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addRequiredCoupledVar("nodal_area", "The nodal area");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");

  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");
  params.addParam<Real>("friction_coefficient", 0, "The friction coefficient");
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<Real>("normal_smoothing_distance", "Distance from edge in parametric coordinates over which to smooth contact normal");
  params.addParam<std::string>("normal_smoothing_method","Method to use to smooth normals (edge_based|nodal_normal_based)");
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<Real>("tension_release", 0.0, "Tension release threshold.  A node in contact will not be released if its tensile load is below this value.  Must be positive.");

  params.addParam<std::string>("formulation", "default", "The contact formulation");
  return params;
}

GluedContactConstraint::GluedContactConstraint(const InputParameters & parameters) :
    SparsityBasedContactConstraint(parameters),
    _component(getParam<unsigned int>("component")),
    _model(contactModel(getParam<std::string>("model"))),
    _formulation(contactFormulation(getParam<std::string>("formulation"))),
    _penalty(getParam<Real>("penalty")),
    _friction_coefficient(getParam<Real>("friction_coefficient")),
    _tension_release(getParam<Real>("tension_release")),
    _updateContactSet(true),
    _residual_copy(_sys.residualGhosted()),
    _x_var(isCoupled("disp_x") ? coupled("disp_x") : libMesh::invalid_uint),
    _y_var(isCoupled("disp_y") ? coupled("disp_y") : libMesh::invalid_uint),
    _z_var(isCoupled("disp_z") ? coupled("disp_z") : libMesh::invalid_uint),
    _vars(_x_var, _y_var, _z_var),
    _nodal_area_var(getMooseVar("nodal_area", 0)),
    _aux_system(_nodal_area_var->sys()),
    _aux_solution(_aux_system.currentSolution())
{
//  _overwrite_slave_residual = false;

  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  if (parameters.isParamValid("normal_smoothing_distance"))
  {
    _penetration_locator.setNormalSmoothingDistance(getParam<Real>("normal_smoothing_distance"));
  }
  if (parameters.isParamValid("normal_smoothing_method"))
  {
    _penetration_locator.setNormalSmoothingMethod(parameters.get<std::string>("normal_smoothing_method"));
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
    {
      updateContactSet();
    }
    _updateContactSet = true;
  }
}

void
GluedContactConstraint::updateContactSet(bool beginning_of_step)
{
  std::set<dof_id_type> & has_penetrated = _penetration_locator._has_penetrated;

  std::map<dof_id_type, PenetrationInfo *>::iterator
    it  = _penetration_locator._penetration_info.begin(),
    end = _penetration_locator._penetration_info.end();

  for (; it!=end; ++it)
  {
    PenetrationInfo * pinfo = it->second;

    // Skip this pinfo if there are no DOFs on this node.
    if ( ! pinfo || pinfo->_node->n_comp(_sys.number(), _vars(_component)) < 1 )
      continue;

    const dof_id_type slave_node_num = it->first;
    std::set<dof_id_type>::iterator hpit = has_penetrated.find(slave_node_num);

    if (beginning_of_step)
    {
      pinfo->_starting_elem = it->second->_elem;
      pinfo->_starting_side_num = it->second->_side_num;
      pinfo->_starting_closest_point_ref = it->second->_closest_point_ref;
    }

    if (pinfo->_distance >= 0)
      if (hpit == has_penetrated.end())
        has_penetrated.insert(slave_node_num);
  }
}

bool
GluedContactConstraint::shouldApply()
{
  std::set<dof_id_type>::iterator hpit = _penetration_locator._has_penetrated.find(_current_node->id());
  return (hpit != _penetration_locator._has_penetrated.end());
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
      Real distance = (*_current_node)(_component) - pinfo->_closest_point(_component);
      Real pen_force = _penalty * distance;

      Real resid = pen_force;
      pinfo->_contact_force(_component) = resid;
      pinfo->_mech_status=PenetrationInfo::MS_STICKING;

      return _test_slave[_i][_qp] * resid;
    }
    case Moose::Master:
    {
      PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

      dof_id_type dof_number = _current_node->dof_number(0, _vars(_component), 0);
      Real resid = _residual_copy(dof_number);

      pinfo->_contact_force(_component) = -resid;
      pinfo->_mech_status=PenetrationInfo::MS_STICKING;

      return _test_master[_i][_qp] * resid;
    }
  }

  return 0;
}

Real
GluedContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::SlaveSlave:
    {
      return _penalty*_phi_slave[_j][_qp]*_test_slave[_i][_qp];
    }
    case Moose::SlaveMaster:
    {
      return _penalty*-_phi_master[_j][_qp]*_test_slave[_i][_qp];
    }
    case Moose::MasterSlave:
    {
      double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
      return slave_jac*_test_master[_i][_qp];
    }
    case Moose::MasterMaster:
      return 0;
  }

  return 0;
}

Real
GluedContactConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type, unsigned int /*jvar*/)
{
  Real retVal = 0;

  switch (type)
  {
    case Moose::SlaveSlave:
      break;
    case Moose::SlaveMaster:
      break;
    case Moose::MasterSlave:
    {
      double slave_jac = (*_jacobian)(_current_node->dof_number(0, _vars(_component), 0), _connected_dof_indices[_j]);
      retVal =  slave_jac*_test_master[_i][_qp];
      break;
    }
    case Moose::MasterMaster:
    break;
  }

  return retVal;
}
