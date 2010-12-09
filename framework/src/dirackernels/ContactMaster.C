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
  params.addRequiredParam<std::vector<unsigned int> >("boundary", "The master boundary");
  params.addRequiredParam<std::vector<unsigned int> >("slave", "The slave boundary");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ContactMaster::ContactMaster(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _penetration_locator(_moose_system, _mesh, parameters.get<std::vector<unsigned int> >("slave"),getParam<std::vector<unsigned int> >("boundary")[0]),
   _residual_copy(residualCopy()),
   _jacobian_copy(jacobianCopy())
{}
           
void
ContactMaster::addPoints()
{
  _penetration_locator.detectPenetration();

  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator it = _penetration_locator._penetration_info.begin();
  std::map<unsigned int, PenetrationLocator::PenetrationInfo *>::iterator end = _penetration_locator._penetration_info.end();

  for(; it!=end; ++it)
  {
    unsigned int slave_node_num = it->first;
    PenetrationLocator::PenetrationInfo * pinfo = it->second;
    
    addPoint(pinfo->_elem, pinfo->_closest_point);

    point_to_info[pinfo->_closest_point] = pinfo;
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

  if(pinfo->_distance < -1e-15)
    return 0;
  
  return _phi[_i][_qp]*_residual_copy(dof_number);
}

Real
ContactMaster::computeQpJacobian()
{
  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
  Node * node = pinfo->_node;
  long int dof_number = node->dof_number(0, _var_num, 0);

  return 0;
  
  if(_i != _j || pinfo->_distance < -1e-15)
    return 0;

  //  std::cout<<dof_number<<std::endl;
  //std::cout<<_jacobian_copy(dof_number,dof_number)<<std::endl;
  return _phi[_i][_qp]*_jacobian_copy(dof_number,dof_number);
}

