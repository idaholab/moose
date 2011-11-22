/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "MultiDContactConstraint.h"

#include "SystemBase.h"
#include "PenetrationLocator.h"

// libMesh includes
#include "string_to_enum.h"

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

MultiDContactConstraint::MultiDContactConstraint(const std::string & name, InputParameters parameters) :
    NodeFaceConstraint(name, parameters),
    _residual_copy(_sys.residualGhosted()),
    _jacobian_update(getParam<bool>("jacobian_update")),
    _component(getParam<unsigned int>("component")),
    _model(contactModel(getParam<std::string>("model"))),
    _penalty(getParam<Real>("penalty")),
    _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
    _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
    _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
    _vars(_x_var, _y_var, _z_var)
{
  _overwrite_slave_residual = false;
}

void
MultiDContactConstraint::timestepSetup()
{
  updateContactSet();
}

void
MultiDContactConstraint::jacobianSetup()
{
  updateContactSet();
}

void
MultiDContactConstraint::updateContactSet()
{
  std::map<unsigned int, bool> & has_penetrated = _penetration_locator._has_penetrated;

  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();

  for (; it!=end; ++it)
  {
    PenetrationLocator::PenetrationInfo * pinfo = it->second;

    if (!pinfo)
    {
      continue;
    }

    if (pinfo->_distance > 0)
    {
      unsigned int slave_node_num = it->first;
      has_penetrated[slave_node_num] = true;
    }
  }
}

bool
MultiDContactConstraint::shouldApply()
{
  return _penetration_locator._has_penetrated[_current_node->id()];
}

Real
MultiDContactConstraint::computeQpSlaveValue()
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  std::cerr<<std::endl
           <<"Popping out node: "<<_current_node->id()<<std::endl
           <<"Closest Point "<<_component<<": "<<pinfo->_closest_point(_component)<<std::endl
           <<"Current Node "<<_component<<": "<<(*_current_node)(_component)<<std::endl
           <<"Current Value: "<<_u_slave[_qp]<<std::endl
           <<"New Value: "<<pinfo->_closest_point(_component) - ((*_current_node)(_component) - _u_slave[_qp])<<std::endl
           <<"Change: "<<_u_slave[_qp] - (pinfo->_closest_point(_component) - ((*_current_node)(_component) - _u_slave[_qp]))<<std::endl<<std::endl;
  
  
  return pinfo->_closest_point(_component) - ((*_current_node)(_component) - _u_slave[_qp]);
}

Real
MultiDContactConstraint::computeQpResidual(Moose::ConstraintType type)
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  const Node * node = pinfo->_node;

  RealVectorValue res_vec;
  // Build up residual vector
  for(unsigned int i=0; i<_dim; ++i)
  {
    int dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);
  }

  const RealVectorValue distance_vec(_mesh.node(node->id()) - pinfo->_closest_point);
  const RealVectorValue pen_force(_penalty * distance_vec);
  Real resid = 0;

  switch(type)
  {
  case Moose::Slave:
    switch(_model)
    {
    case CM_FRICTIONLESS:
      
      resid = pinfo->_normal(_component) * (pinfo->_normal * ( pen_force - res_vec ));
      break;

    case CM_GLUED:
    case CM_TIED:

      resid = pen_force(_component)
        - res_vec(_component)
        ;

      break;
      
    default:
      mooseError("Invalid or unavailable contact model");
    }
    return _test_slave[_i][_qp] * resid;
  case Moose::Master:
    switch(_model)
    {
    case CM_FRICTIONLESS:

      resid = pinfo->_normal(_component) * (pinfo->_normal * res_vec);
      break;

    case CM_GLUED:
    case CM_TIED:
      resid = res_vec(_component);
      break;

    default:
      mooseError("Invalid or unavailable contact model");
    }
    return _test_master[_i][_qp] * resid;
  }
  return 0;
}

Real
MultiDContactConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  //const Node * node = pinfo->_node;

  if(_name != "contact_x")
    return 0;
  
  double slave_jac = 0;
  switch(type)
  {
  case Moose::SlaveSlave:
    switch(_model)
    {
    case CM_FRICTIONLESS:
      
      slave_jac = pinfo->_normal(_component) * pinfo->_normal(_component) * ( _penalty*_phi_slave[_j][_qp] - (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]) );
      break;

    case CM_GLUED:
    case CM_TIED:
/*
      resid = pen_force(_component)
        - res_vec(_component)
        ;
*/
      break;
      
    default:
      mooseError("Invalid or unavailable contact model");
    }
    return _test_slave[_i][_qp] * slave_jac;
  case Moose::SlaveMaster:
    switch(_model)
    {
    case CM_FRICTIONLESS:
      
      slave_jac = pinfo->_normal(_component) * pinfo->_normal(_component) * ( -_penalty*_phi_master[_j][_qp] );
      break;

    case CM_GLUED:
    case CM_TIED:
/*
      resid = pen_force(_component)
        - res_vec(_component)
        ;
*/
      break;
      
    default:
      mooseError("Invalid or unavailable contact model");
    }
    return _test_slave[_i][_qp] * slave_jac;


//    return -_phi_master[_j][_qp]*_test_slave[_i][_qp];
  case Moose::MasterSlave:
    slave_jac = (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]);
    return slave_jac*_test_master[_i][_qp];
  case Moose::MasterMaster:
    return 0;
  }
  return 0;
}
