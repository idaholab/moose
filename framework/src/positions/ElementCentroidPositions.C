//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementCentroidPositions.h"

registerMooseObject("MooseApp", ElementCentroidPositions);

InputParameters
ElementCentroidPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addClassDescription("Positions of element centroids.");
  params += BlockRestrictable::validParams();

  // Element centroids could be sorted by XYZ or by id. Default to not sorting
  params.set<bool>("auto_sort") = false;
  // Gathered locally, should be broadcast on every process
  params.set<bool>("auto_broadcast") = true;

  return params;
}

ElementCentroidPositions::ElementCentroidPositions(const InputParameters & parameters)
  : Positions(parameters), BlockRestrictable(this), _mesh(_fe_problem.mesh())
{
  // Mesh is ready at construction
  initialize();
  // Trigger synchronization as the initialization is distributed
  finalize();
}

void
ElementCentroidPositions::initialize()
{
  clearPositions();

  // By default, initialize should be called on meshChanged()
  // Gathering of positions is local, reporter system makes sure to make it global
  if (blockRestricted())
  {
    _positions_2d.resize(numBlocks());
    unsigned int b_index = 0;
    for (const auto & sub_id : blockIDs())
    {
      for (const auto & elem : _mesh.getMesh().active_local_subdomain_elements_ptr_range(sub_id))
      {
        auto centroid = elem->true_centroid();
        _positions.emplace_back(centroid);
        _positions_2d[b_index].emplace_back(centroid);
      }
      b_index += 1;
    }
  }
  else
  {
    _positions.resize(_mesh.getMesh().n_active_local_elem());
    unsigned int i = 0;
    for (const auto & elem : _mesh.getMesh().active_local_element_ptr_range())
      _positions[i++] = elem->true_centroid();
  }
}
