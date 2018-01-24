//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostUserObject.h"
#include "MooseMesh.h"

// invalid_processor_id
#include "libmesh/dof_object.h"

template <>
InputParameters
validParams<GhostUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<unsigned int>(
      "rank",
      DofObject::invalid_processor_id,
      "The rank for which the ghosted elements are recorded (Default: ALL)");

  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  params.registerRelationshipManagers("ElementSideNeighborLayers");
  params.addRequiredParam<unsigned short>("element_side_neighbor_layers",
                                          "Number of layers to ghost");

  params.addClassDescription("User object to calculate ghosted elements on a single processor or "
                             "the union across all processors.");
  return params;
}

GhostUserObject::GhostUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _rank(getParam<unsigned int>("rank"))
{
}

void
GhostUserObject::initialize()
{
  _ghost_data.clear();
}

void
GhostUserObject::execute()
{
  auto my_processor_id = processor_id();

  if (_rank == DofObject::invalid_processor_id || my_processor_id == _rank)
  {
    const auto & mesh = _subproblem.mesh().getMesh();

    const auto end = mesh.active_elements_end();
    for (auto el = mesh.active_elements_begin(); el != end; ++el)
    {
      const auto & elem = *el;

      if (elem->processor_id() != my_processor_id)
        _ghost_data.emplace(elem->id());
    }
  }
}

void
GhostUserObject::finalize()
{
  _communicator.set_union(_ghost_data);
}

unsigned long
GhostUserObject::getElementalValue(dof_id_type element_id) const
{
  return _ghost_data.find(element_id) != _ghost_data.end();
}
