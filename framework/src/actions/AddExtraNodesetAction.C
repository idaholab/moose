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

#include "AddExtraNodesetAction.h"
#include "Parser.h"
#include "MooseMesh.h"
#include "MProblem.h"
#include "ActionWarehouse.h"

template<>
InputParameters validParams<AddExtraNodesetAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<unsigned int >("id", "The nodeset number you want to use.");
  params.addRequiredParam<std::vector<unsigned int> >("nodes", "The nodes you want to be in the nodeset.");
  
  return params;
}

AddExtraNodesetAction::AddExtraNodesetAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}


void
AddExtraNodesetAction::act()
{
  MooseMesh * mesh = _parser_handle._mesh;
  MooseMesh * displaced_mesh = _parser_handle._displaced_mesh;

  unsigned int id = getParam<unsigned int>("id");
  const std::vector<unsigned int> & nodes = getParam<std::vector<unsigned int> >("nodes");

  for(unsigned int i=0; i<nodes.size(); i++)
  {
    if(mesh)
      mesh->getMesh().boundary_info->add_node(nodes[i], id);
    if(displaced_mesh)
      displaced_mesh->getMesh().boundary_info->add_node(nodes[i], id);
  }
}

