#include "ContactSlave.h"

//Moose includes
#include "ComputeQPSolution.h"
#include "MooseSystem.h"

//libmesh includes
#include "fe_interface.h"

template<>
InputParameters validParams<ContactSlave>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params.addRequiredParam<unsigned int>("master", "The master boundary");
  params.set<bool>("_integrated") = false;
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ContactSlave::ContactSlave(const std::string & name, InputParameters parameters)
  :BoundaryCondition(name, parameters),
   _penetration_locator(getPenetrationLocator(getParam<unsigned int>("master"), getParam<std::vector<unsigned int> >("boundary")[0]))
{
  if(getParam<std::vector<unsigned int> >("boundary").size() > 1)
    mooseError("ContactSlave can only be used with one boundary at a time!");

  _moose_system.needSerializedSolution(true);
}

void
ContactSlave::setup()
{}

bool
ContactSlave::shouldBeApplied()
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  if(!pinfo)
    return false;
  
  return _penetration_locator._has_penetrated[_current_node->id()];  
}

Real
ContactSlave::computeQpResidual()
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];
  Elem * elem = pinfo->_elem;
/*
  std::vector<unsigned int> dof_indices;

  const DofMap& dof_map = *_moose_system.getDofMap();
  dof_map.dof_indices (elem, dof_indices, _var_num);
  FEType fe_type = dof_map.variable_type(_var_num);
  AutoPtr<FEBase> fe (FEBase::build(_moose_system.getDim(), fe_type));
  const std::vector<std::vector<Real> >& phi = fe->get_phi();

  std::vector<Point> points(1);
  points[0] = pinfo->_closest_point;

  //if((*_current_node)(1) < 1.19 && (*_current_node)(1) > 1.13)
  //  std::cout<<(*_current_node)<<pinfo->_closest_point<<std::endl;
          
  std::vector<Point> mapped_points;

  libMesh::FEInterface::inverse_map(_moose_system.getDim(), fe_type, elem, points, mapped_points);
          
  fe->reinit(elem, &mapped_points);

  Real master_displacement = 0;

  computeQpSolution(master_displacement, _moose_system._serialized_solution, dof_indices, 0, phi);
*/
//  std::cout<<_u[_qp]<<std::endl;
  
//  return 0;

//  if(_current_node->id() == 441)
//    std::cout<<pinfo->_distance<<std::endl;

//  std::cout<<(_moose_system.getMesh()->node(_current_node->id())(0)+_u[_qp])-pinfo->_closest_point(0)<<std::endl;
//  if(_current_node->id() == 893)
//    std::cout<<(_moose_system.getMesh()->node(_current_node->id())(0)+_u[_qp])-pinfo->_closest_point(0)<<std::endl;
  
  return (_moose_system.getMesh()->node(_current_node->id())(0)+_u[_qp])-pinfo->_closest_point(0);
}
