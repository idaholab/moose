//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFlowJunctionUserObject.h"
#include "MooseMesh.h"

InputParameters
ADFlowJunctionUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();

  params.addRequiredParam<std::vector<Real>>(
      "normals", "Flow channel outward normals or junction inward normals");
  params.addParam<std::vector<processor_id_type>>(
      "processor_ids", "Processor IDs owning each connected flow channel element");

  params.addClassDescription("Provides common interfaces for flow junction user objects");

  return params;
}

ADFlowJunctionUserObject::ADFlowJunctionUserObject(const InputParameters & parameters)
  : SideUserObject(parameters),

    _bnd_ids_vector(_mesh.getBoundaryIDs(boundaryNames(), false)),
    _n_bnd_ids(_bnd_ids_vector.size()),
    _normal(getParam<std::vector<Real>>("normals")),
    _dir(getMaterialProperty<RealVectorValue>("direction")),
    _n_connections(_normal.size()),
    _processor_ids(getParam<std::vector<processor_id_type>>("processor_ids"))
{
  if (comm().size() > 1)
  {
    if (_processor_ids.size() != _n_connections)
      mooseError("The number of entries in the 'processor_ids' parameter must equal the number of "
                 "connections (",
                 _n_connections,
                 ").");
  }
  else
    _processor_ids.resize(_n_connections, 0.);
}

void
ADFlowJunctionUserObject::finalize()
{
}

unsigned int
ADFlowJunctionUserObject::getBoundaryIDIndex()
{
  auto elem_side = std::make_pair(_current_elem, _current_side);
  auto it = _elem_side_to_bnd_id_index.find(elem_side);
  if (it == _elem_side_to_bnd_id_index.end())
  {
    // Get the boundary IDs associated with this (elem,side). In general, there
    // may be more than one boundary ID associated with an (elem,side), but
    // there should be exactly one of these boundary IDs that is seen by this
    // user object.
    const std::vector<BoundaryID> elem_side_bnd_ids =
        _mesh.getBoundaryIDs(_current_elem, _current_side);

    // Loop over the boundary IDs for this (elem,side) pair and over the
    // boundary IDs for this side user object; there should be exactly one match.
    bool found_matching_boundary_id = false;
    unsigned int boundary_id_index = 0;
    for (unsigned int i = 0; i < elem_side_bnd_ids.size(); i++)
      for (unsigned int j = 0; j < _n_bnd_ids; j++)
        if (elem_side_bnd_ids[i] == _bnd_ids_vector[j])
        {
          if (found_matching_boundary_id)
            mooseError(name(), ": Multiple matches for boundary ID were found");
          else
          {
            found_matching_boundary_id = true;
            boundary_id_index = j;
          }
        }

    if (!found_matching_boundary_id)
    {
      std::stringstream ss;
      ss << name() << ": No matching boundary ID was found for (elem,side) = ("
         << _current_elem->id() << "," << _current_side << ").";
      mooseError(ss.str());
    }

    // Check that this boundary ID index was not already found by an earlier (elem,side)
    for (auto it_other = _elem_side_to_bnd_id_index.begin();
         it_other != _elem_side_to_bnd_id_index.end();
         it_other++)
    {
      if (it_other->second == boundary_id_index)
        mooseError(name(), ": Multiple (elem,side) pairs had the same boundary ID index");
    }

    // Store boundary index for future use
    _elem_side_to_bnd_id_index[elem_side] = boundary_id_index;

    return boundary_id_index;
  }
  else
  {
    // get the boundary ID that is already stored in the map
    return _elem_side_to_bnd_id_index[elem_side];
  }
}

void
ADFlowJunctionUserObject::checkValidConnectionIndex(const unsigned int & connection_index) const
{
  if (connection_index >= _n_connections)
    mooseError(name(),
               ": The connection index '",
               connection_index,
               "' is invalid; the range of valid indices is (0, ",
               _n_connections - 1,
               ").");
}
