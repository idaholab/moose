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
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ContactMaster::ContactMaster(const std::string & name, InputParameters parameters)
  :DiracKernel(name, parameters),
   _penetration_locator(getPenetrationLocator(getParam<unsigned int>("boundary"), getParam<unsigned int>("slave"))),
   _residual_copy(residualCopy()),
   _jacobian_copy(jacobianCopy()),
   _nl_it(99999)
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
      if(_residual_copy(dof_number) > 0)
        _penetration_locator._has_penetrated[slave_node_num] = false;
      else if(pinfo->_distance > 0)
        _penetration_locator._has_penetrated[slave_node_num] = true;
    }  
    
    if(_penetration_locator._has_penetrated[slave_node_num])
    {
      addPoint(pinfo->_elem, pinfo->_closest_point);
      point_to_info[pinfo->_closest_point] = pinfo;
    }
  }

  _nl_it = _moose_system._current_nl_it;
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

  return _phi[_i][_qp]*_residual_copy(dof_number);
}

Real
ContactMaster::computeQpJacobian()
{
  PenetrationLocator::PenetrationInfo * pinfo = point_to_info[_current_point];
  Node * node = pinfo->_node;
  long int dof_number = node->dof_number(0, _var_num, 0);

//  return 0;
  
  if(_i != _j)
    return 0;

  //  std::cout<<dof_number<<std::endl;
  //std::cout<<_jacobian_copy(dof_number,dof_number)<<std::endl;
  return _phi[_i][_qp]*_jacobian_copy(dof_number,dof_number);
}

