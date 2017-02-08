/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// MOOSE includes
#include "MultiDContactConstraint.h"
#include "SystemBase.h"
#include "PenetrationLocator.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/string_to_enum.h"
#include "libmesh/sparse_matrix.h"

template<>
InputParameters validParams<MultiDContactConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<bool>("jacobian_update", false, "Whether or not to update the 'in contact' list every jacobian evaluation (by default it will happen once per timestep");

  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.addParam<std::string>("model", "frictionless", "The contact model to use");
  params.addParam<Real>("penalty", 1e8, "The penalty to apply.  This can vary depending on the stiffness of your materials");

  // TODO: Reenable this
//  params.addParam<std::string>("order", "FIRST", "The finite element order");

  return params;
}

MultiDContactConstraint::MultiDContactConstraint(const InputParameters & parameters) :
    NodeFaceConstraint(parameters),
    _residual_copy(_sys.residualGhosted()),
    _jacobian_update(getParam<bool>("jacobian_update")),
    _component(getParam<unsigned int>("component")),
    _model(contactModel(getParam<std::string>("model"))),
    _penalty(getParam<Real>("penalty")),
    _x_var(isCoupled("disp_x") ? coupled("disp_x") : libMesh::invalid_uint),
    _y_var(isCoupled("disp_y") ? coupled("disp_y") : libMesh::invalid_uint),
    _z_var(isCoupled("disp_z") ? coupled("disp_z") : libMesh::invalid_uint),
    _mesh_dimension(_mesh.dimension()),
    _vars(_x_var, _y_var, _z_var)
{
  _overwrite_slave_residual = false;
}

void
MultiDContactConstraint::timestepSetup()
{
  if (_component == 0)
  {
    updateContactSet();
  }
}

void
MultiDContactConstraint::jacobianSetup()
{
  if (_component == 0)
    updateContactSet();
}

void
MultiDContactConstraint::updateContactSet()
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

    const Node * node = pinfo->_node;

    dof_id_type slave_node_num = it->first;
    std::set<dof_id_type>::iterator hpit = has_penetrated.find(slave_node_num);

    RealVectorValue res_vec;
    // Build up residual vector
    for (unsigned int i=0; i<_mesh_dimension; ++i)
    {
      dof_id_type dof_number = node->dof_number(0, _vars(i), 0);
      res_vec(i) = _residual_copy(dof_number);
    }

//    Real resid = 0;
    switch (_model)
    {
    case CM_FRICTIONLESS:

//      resid = pinfo->_normal * res_vec;
      break;

    case CM_GLUED:

//      resid = pinfo->_normal * res_vec;
      break;

    default:
      mooseError2("Invalid or unavailable contact model");
      break;
    }

//    if (hpit != has_penetrated.end() && resid < 0)
//      Moose::err<<resid<<std::endl;
/*
    if (hpit != has_penetrated.end() && resid < -.15)
    {
      Moose::err<<std::endl<<"Unlocking node "<<node->id()<<" because resid: "<<resid<<std::endl<<std::endl;

      has_penetrated.erase(hpit);
      unlocked_this_step[slave_node_num] = true;
    }
    else*/
    if (pinfo->_distance > 0 && hpit == has_penetrated.end())// && !unlocked_this_step[slave_node_num])
    {
//      Moose::err<<std::endl<<"Locking node "<<node->id()<<" because distance: "<<pinfo->_distance<<std::endl<<std::endl;

      has_penetrated.insert(slave_node_num);
    }
  }
}

bool
MultiDContactConstraint::shouldApply()
{
  std::set<dof_id_type>::iterator hpit = _penetration_locator._has_penetrated.find(_current_node->id());
  return (hpit != _penetration_locator._has_penetrated.end());
}

Real
MultiDContactConstraint::computeQpSlaveValue()
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
/*
  Moose::err<<std::endl
           <<"Popping out node: "<<_current_node->id()<<std::endl
           <<"Closest Point "<<_component<<": "<<pinfo->_closest_point(_component)<<std::endl
           <<"Current Node "<<_component<<": "<<(*_current_node)(_component)<<std::endl
           <<"Current Value: "<<_u_slave[_qp]<<std::endl
           <<"New Value: "<<pinfo->_closest_point(_component) - ((*_current_node)(_component) - _u_slave[_qp])<<std::endl
           <<"Change: "<<_u_slave[_qp] - (pinfo->_closest_point(_component) - ((*_current_node)(_component) - _u_slave[_qp]))<<std::endl<<std::endl;
*/
  return  pinfo->_closest_point(_component) - ((*_current_node)(_component) - _u_slave[_qp]);
}

Real
MultiDContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  const Node * node = pinfo->_node;

  RealVectorValue res_vec;
  // Build up residual vector
  for (unsigned int i=0; i<_mesh_dimension; ++i)
  {
    dof_id_type dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);
  }

  const RealVectorValue distance_vec(_mesh.nodeRef(node->id()) - pinfo->_closest_point);
  const RealVectorValue pen_force(_penalty * distance_vec);
  Real resid = 0;

  switch (type)
  {
  case Moose::Slave:
    switch (_model)
    {
    case CM_FRICTIONLESS:

      resid = pinfo->_normal(_component) * (pinfo->_normal * ( pen_force - res_vec ));
      break;

    case CM_GLUED:

      resid = pen_force(_component)
        - res_vec(_component)
        ;

      break;

    default:
      mooseError2("Invalid or unavailable contact model");
      break;
    }
    return _test_slave[_i][_qp] * resid;
  case Moose::Master:
    switch (_model)
    {
    case CM_FRICTIONLESS:

      resid = pinfo->_normal(_component) * (pinfo->_normal * res_vec);
      break;

    case CM_GLUED:
      resid = res_vec(_component);
      break;

    default:
      mooseError2("Invalid or unavailable contact model");
      break;
    }
    return _test_master[_i][_qp] * resid;
  }
  return 0;
}

Real
MultiDContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  double slave_jac = 0;
  switch (type)
  {
  case Moose::SlaveSlave:
    switch (_model)
    {
    case CM_FRICTIONLESS:

      slave_jac = pinfo->_normal(_component) * pinfo->_normal(_component) * ( _penalty*_phi_slave[_j][_qp] - (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]) );
      break;

    case CM_GLUED:
/*
      resid = pen_force(_component)
        - res_vec(_component)
        ;
*/
      break;

    default:
      mooseError2("Invalid or unavailable contact model");
      break;
    }
    return _test_slave[_i][_qp] * slave_jac;
  case Moose::SlaveMaster:
    switch (_model)
    {
    case CM_FRICTIONLESS:

      slave_jac = pinfo->_normal(_component) * pinfo->_normal(_component) * ( -_penalty*_phi_master[_j][_qp] );
      break;

    case CM_GLUED:
/*
      resid = pen_force(_component)
        - res_vec(_component)
        ;
*/
      break;

    default:
      mooseError2("Invalid or unavailable contact model");
      break;
    }
    return _test_slave[_i][_qp] * slave_jac;
  case Moose::MasterSlave:
    slave_jac = (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]);
    return slave_jac*_test_master[_i][_qp];
  case Moose::MasterMaster:
    return 0;
  }
  return 0;
}

