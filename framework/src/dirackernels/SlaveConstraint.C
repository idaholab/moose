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

#include "SlaveConstraint.h"

// Moose includes
#include "MooseSystem.h"

// libmesh includes
#include "plane.h"
#include "sparse_matrix.h"

template<>
InputParameters validParams<SlaveConstraint>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredParam<unsigned int>("boundary", "The slave boundary");
  params.addRequiredParam<unsigned int>("master", "The master boundary");
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");

  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

SlaveConstraint::SlaveConstraint(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _component(getParam<Real>("component")),
   _penetration_locator(getPenetrationLocator(getParam<unsigned int>("master"), getParam<unsigned int>("boundary"))),
   _residual_copy(residualCopy()),
   _jacobian_copy(jacobianCopy()),
   _x_var(isCoupled("disp_x") ? coupled("disp_x") : 99999),
   _y_var(isCoupled("disp_y") ? coupled("disp_y") : 99999),
   _z_var(isCoupled("disp_z") ? coupled("disp_z") : 99999),
   _vars(_x_var, _y_var, _z_var)
{}
           
void
SlaveConstraint::addPoints()
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
      
      if(res_mag < 0 && _penetration_locator._has_penetrated[slave_node_num])
      {
        _penetration_locator._has_penetrated[slave_node_num] = false;
      }
      else
        if(pinfo->_distance > 0)
          _penetration_locator._has_penetrated[slave_node_num] = true;
    }  
    
    if(_penetration_locator._has_penetrated[slave_node_num])
    {
      // Just use the first element this node is connected to
      Elem * elem = _mesh.elem(_moose_system.node_to_elem_map[slave_node_num][0]);
      addPoint(elem, *node);
      point_to_info[*node] = pinfo;
    }
  }
}

Real
SlaveConstraint::computeQpResidual()
{
  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
  Node * node = pinfo->_node;
/*
  if(node->id() == 10)
    std::cout<<"Constraining"<<std::endl;
*/

  RealVectorValue res_vec;

  // Build up residual vector
  for(unsigned int i=0; i<_dim; i++)
  {
    long int dof_number = node->dof_number(0, _vars(i), 0);
    res_vec(i) = _residual_copy(dof_number);  
  }
  
  Real res_mag = pinfo->_normal * res_vec;
  
  return _phi[_i][_qp] * (
                          1000*(
                               (pinfo->_closest_point(_component) - (_moose_system.getMesh()->node(node->id())(_component)+_u[_qp]))
                              ) -
                          (pinfo->_normal(_component)*res_mag)
                         );  
}

Real
SlaveConstraint::computeQpJacobian()
{
  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
  Node * node = pinfo->_node;
  long int dof_number = node->dof_number(0, _var_num, 0);  

  return _phi[_i][_qp] * (
                          1000*-_phi[_j][_qp] -
                          (pinfo->_normal(_component)*pinfo->_normal(_component)*_jacobian_copy(dof_number,dof_number))
                         );
}
