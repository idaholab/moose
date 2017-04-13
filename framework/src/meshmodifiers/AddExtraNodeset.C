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

template <>
InputParameters
validParams<AddExtraNodeset>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addRequiredParam<std::vector<BoundaryName>>("new_boundary",
                                                     "The name of the boundary to create");

  params.addParam<std::vector<unsigned int>>("nodes",
                                             "The nodes you want to be in the nodeset "
                                             "(Either this parameter or \"coord\" must be "
                                             "supplied).");
  params.addParam<std::vector<Real>>("coord",
                                     "The nodes with coordinates you want to be in the "
                                     "nodeset (Either this parameter or \"nodes\" must be "
                                     "supplied).");
  params.addParam<Real>(
      "tolerance", TOLERANCE, "The tolerance in which two nodes are considered identical");

  return params;
}

AddExtraNodeset::AddExtraNodeset(const InputParameters & params) : MeshModifier(params) {}

void
AddExtraNodeset::modify()
{

  // make sure the input is not empty
  bool data_valid = false;
  if (_pars.isParamValid("nodes"))
    if (getParam<std::vector<unsigned int>>("nodes").size() != 0)
      data_valid = true;
  if (_pars.isParamValid("coord"))
  {
    unsigned int n_coord = getParam<std::vector<Real>>("coord").size();
    if (n_coord % _mesh_ptr->dimension() != 0)
      mooseError("Size of node coordinates does not match the mesh dimension");
    if (n_coord != 0)
      data_valid = true;
  }
  if (!data_valid)
    mooseError("Node set can not be empty!");

  // Get the BoundaryIDs from the mesh
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("new_boundary");
  std::vector<BoundaryID> boundary_ids = _mesh_ptr->getBoundaryIDs(boundary_names, true);

  // Get a reference to our BoundaryInfo object
  BoundaryInfo & boundary_info = _mesh_ptr->getMesh().get_boundary_info();

  // add nodes with their ids
  const std::vector<unsigned int> & nodes = getParam<std::vector<unsigned int>>("nodes");
  for (const auto & node_id : nodes)
    for (const auto & boundary_id : boundary_ids)
      boundary_info.add_node(node_id, boundary_id);

  // add nodes with their coordinates
  const std::vector<Real> & coord = getParam<std::vector<Real>>("coord");
  unsigned int dim = _mesh_ptr->dimension();
  unsigned int n_nodes = coord.size() / dim;

  UniquePtr<PointLocatorBase> locator = _mesh_ptr->getMesh().sub_point_locator();
  locator->enable_out_of_mesh_mode();

  for (unsigned int i = 0; i < n_nodes; ++i)
  {
    Point p;
    for (unsigned int j = 0; j < dim; ++j)
      p(j) = coord[i * dim + j];

    const Elem * elem = (*locator)(p);
    if (!elem)
      mooseError(
          "Unable to locate the following point within the domain, please check its coordinates:\n",
          p);

    bool on_node = false;
    for (unsigned int j = 0; j < elem->n_nodes(); ++j)
    {
      const Node * node = elem->node_ptr(j);

      Point q;
      for (unsigned int k = 0; k < dim; ++k)
        q(k) = (*node)(k);

      if (p.absolute_fuzzy_equals(q, getParam<Real>("tolerance")))
      {
        for (const auto & boundary_id : boundary_ids)
          boundary_info.add_node(node, boundary_id);

        on_node = true;
        break;
      }
    }
    if (!on_node)
      mooseError("Point can not be located!");
  }

  for (unsigned int i = 0; i < boundary_ids.size(); ++i)
    boundary_info.nodeset_name(boundary_ids[i]) = boundary_names[i];
}
