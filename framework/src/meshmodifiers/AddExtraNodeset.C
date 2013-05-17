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

#include "AddExtraNodeset.h"
#include "MooseApp.h"
#include "MooseMesh.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"

template<>
InputParameters validParams<AddExtraNodeset>()
{
  InputParameters params = validParams<MeshModifier>();

  params.addRequiredParam<std::vector<BoundaryName> >("boundary", "The boundary you want to use.");
  params.addParam<std::vector<unsigned int> >("nodes", "The nodes you want to be in the nodeset (Either this parameter or \"coord\" must be supplied).");
  params.addParam<std::vector<Real> >("coord","The nodes with coordinates you want to be in the nodeset (Either this parameter or \"nodes\" must be supplied).");
  params.addParam<Real>("tolerance", TOLERANCE, "The tolerance in which two nodes are considered identical");

  return params;
}

AddExtraNodeset::AddExtraNodeset(const std::string & name, InputParameters params) :
    MeshModifier(name, params)
{
}

void
AddExtraNodeset::modify()
{
  // make sure the input is not empty
  bool data_valid = false;
  if (_pars.isParamValid("nodes"))
    if (getParam<std::vector<unsigned int> >("nodes").size() != 0)
      data_valid = true;
  if (_pars.isParamValid("coord"))
  {
    unsigned int n_coord = getParam<std::vector<Real> >("coord").size();
    if (n_coord % _mesh_ptr->dimension() != 0)
      mooseError("Size of node coordinates does not match the mesh dimension");
    if (n_coord !=0)
      data_valid = true;
  }
  if (!data_valid)
    mooseError("Node set can not be empty!");

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName> >("boundary");
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);

  // add nodes with their ids
  const std::vector<unsigned int> & nodes = getParam<std::vector<unsigned int> >("nodes");
  for(unsigned int i=0; i<nodes.size(); i++)
    for(unsigned int j=0; j<boundary_ids.size(); ++j)
      _mesh_ptr->getMesh().boundary_info->add_node(nodes[i], boundary_ids[j]);

  // add nodes with their coordinates
  const std::vector<Real> & coord = getParam<std::vector<Real> >("coord");
  unsigned int dim = _mesh_ptr->dimension();
  unsigned int n_nodes = coord.size() / dim;

  for (unsigned int i=0; i<n_nodes; i++)
  {
    Point p;
    for (unsigned int j=0; j<dim; j++)
      p(j) = coord[i*dim+j];

    const Elem* elem = _mesh_ptr->getMesh().point_locator() (p);

    bool on_node = false;
    for (unsigned int j=0; j<elem->n_nodes(); j++)
    {
      const Node* node = elem->get_node(j);

      Point q;
      for (unsigned int k=0; k<dim; k++)
        q(k) = (*node)(k);

      if (p.absolute_fuzzy_equals(q, getParam<Real>("tolerance")))
      {
        for(unsigned int j=0; j<boundary_ids.size(); ++j)
          _mesh_ptr->getMesh().boundary_info->add_node(node, boundary_ids[j]);
        on_node = true;
        break;
      }
    }
    if (!on_node)
      mooseError("Point can not be located!");
  }

  for (unsigned int i=0; i<boundary_ids.size(); ++i)
    _mesh_ptr->_mesh.boundary_info->sideset_name(boundary_ids[i]) = boundary_names[i];
}

