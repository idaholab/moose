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

#include "ContactMaster.h"

// Moose includes
#include "MooseSystem.h"

// libmesh includes
#include "sparse_matrix.h"

template<>
InputParameters validParams<ContactMaster>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<unsigned int>("boundary", "The master boundary");
  params.addRequiredParam<unsigned int>("slave", "The slave boundary");
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ContactMaster::ContactMaster(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _component(getParam<Real>("component")),
   _penetration_locator(getPenetrationLocator(getParam<unsigned int>("boundary"), getParam<unsigned int>("slave"))),
   _residual_copy(residualCopy()),
   _jacobian_copy(jacobianCopy()),
   _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
   _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
   _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
   _vars(_x_var, _y_var, _z_var)
{}
           
void
ContactMaster::addPoints()
{
  point_to_info.clear();
  
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();

  for(; it!=end; ++it)
  {
    unsigned int slave_node_num = it->first;
    PenetrationLocator::PenetrationInfo * pinfo = it->second;

    if(!pinfo)
      continue;

    Node * node = pinfo->_node;
    long int dof_number = node->dof_number(0, _var_num, 0);

    if(_moose_system.DUMMY_CONTACT_FLAG)
    {
      RealVectorValue res_vec;

      // Build up residual vector
      for(unsigned int i=0; i<_dim; i++)
      {
        long int dof_number = node->dof_number(0, _vars(i), 0);
        res_vec(i) = _residual_copy(dof_number);  
      }
  
      Real res_mag = pinfo->_normal * res_vec;

      
      if(res_mag < 0 && std::abs(res_mag) > 1e-3 && _penetration_locator._has_penetrated[slave_node_num])
      {
        _penetration_locator._has_penetrated[slave_node_num] = false;
      }
      else
        if(pinfo->_distance > 0)
          _penetration_locator._has_penetrated[slave_node_num] = true;
    }  
    
    if(_penetration_locator._has_penetrated[slave_node_num])
    {
      addPoint(pinfo->_elem, pinfo->_closest_point);
      point_to_info[pinfo->_closest_point] = pinfo;
    }
  }
}

Real
ContactMaster::computeQpResidual()
{
  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
  Node * node = pinfo->_node;
//  std::cout<<node->id()<<std::endl;
  long int dof_number = node->dof_number(0, _var_num, 0);
//  std::cout<<dof_number<<std::endl;
//  std::cout<<_residual_copy(dof_number)<<std::endl;

//  std::cout<<node->id()<<": "<<_residual_copy(dof_number)<<std::endl;
  RealVectorValue res_vec;

  // Build up residual vector
  for(unsigned int i=0; i<_dim; i++)
  {
    long int dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);  
  }
  
  Real res_mag = pinfo->_normal * res_vec;
  
  return _phi[_i][_qp]*pinfo->_normal(_component)*res_mag;
}

Real
ContactMaster::computeQpJacobian()
{
  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
  Node * node = pinfo->_node;
  long int dof_number = node->dof_number(0, _var_num, 0);

  return 0;
  
  if(_i != _j)
    return 0;

  //  std::cout<<dof_number<<std::endl;
  //std::cout<<_jacobian_copy(dof_number,dof_number)<<std::endl;
  return _phi[_i][_qp]*pinfo->_normal(_component)*_jacobian_copy(dof_number,dof_number);
}

