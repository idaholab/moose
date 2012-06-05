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
#include "MooseApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"

template<>
InputParameters validParams<AddExtraNodesetAction>()
{
  InputParameters params = validParams<Action>();

  params.addRequiredParam<unsigned int >("id", "The nodeset number you want to use.");
  params.addParam<std::vector<unsigned int> >("nodes", "The nodes you want to be in the nodeset (Either this parameter or \"coord\" must be supplied).");
  params.addParam<std::vector<Real> >("coord","The nodes with coordinates you want to be in the nodeset (Either this parameter or \"nodes\" must be supplied).");
  params.addParam<Real>("tolerance", TOLERANCE, "The tolerance in which two nodes are considered identical");

  return params;
}

AddExtraNodesetAction::AddExtraNodesetAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
AddExtraNodesetAction::act()
{
  // make sure the input is not empty
  bool data_valid = false;
  if (_pars.isParamValid("nodes"))
    if (getParam<std::vector<unsigned int> >("nodes").size() != 0)
      data_valid = true;
  if (_pars.isParamValid("coord"))
  {
    unsigned int n_coord = getParam<std::vector<Real> >("coord").size();
    if (n_coord % _mesh->dimension() != 0)
      mooseError("Size of node coordinates does not match the mesh dimension");
    if (n_coord !=0)
      data_valid = true;
  }
  if (!data_valid)
    mooseError("Node set can not be empty!");

  unsigned int id = getParam<unsigned int>("id");

  // add nodes with their ids
  const std::vector<unsigned int> & nodes = getParam<std::vector<unsigned int> >("nodes");
  for(unsigned int i=0; i<nodes.size(); i++)
  {
    if(_mesh)
      _mesh->getMesh().boundary_info->add_node(nodes[i], id);
    if(_displaced_mesh)
      _displaced_mesh->getMesh().boundary_info->add_node(nodes[i], id);
  }

  // add nodes with their coordinates
  const std::vector<Real> & coord = getParam<std::vector<Real> >("coord");
  unsigned int dim = _mesh->dimension();
  unsigned int n_nodes = coord.size() / dim;

  for (unsigned int i=0; i<n_nodes; i++)
  {
    Point p;
    for (unsigned int j=0; j<dim; j++)
      p(j) = coord[i*dim+j];

    const Elem* elem = _mesh->getMesh().point_locator() (p);

    bool on_node = false;
    for (unsigned int j=0; j<elem->n_nodes(); j++)
    {
      const Node* node = elem->get_node(j);

      Point q;
      for (unsigned int k=0; k<dim; k++)
        q(k) = (*node)(k);

      if (p.absolute_fuzzy_equals(q, getParam<Real>("tolerance"))) {
        _mesh->getMesh().boundary_info->add_node(node, id);
        on_node = true;
        break;
      }
    }
    if (!on_node)
      mooseError("Point can not be located!");
  }
}

