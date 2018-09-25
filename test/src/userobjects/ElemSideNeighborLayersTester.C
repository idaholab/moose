//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemSideNeighborLayersTester.h"
#include "MooseMesh.h"

// invalid_processor_id
#include "libmesh/dof_object.h"

registerMooseObject("MooseTestApp", ElemSideNeighborLayersTester);

template <>
InputParameters
validParams<ElemSideNeighborLayersTester>()
{
  InputParameters params = validParams<ElementUOProvider>();
  params.addParam<unsigned int>(
      "rank",
      DofObject::invalid_processor_id,
      "The rank for which the ghosted elements are recorded (Default: ALL)");
  params.addParam<bool>(
      "show_evaluable", false, "Instead of showing ghosts, shows evaluable elements");

  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  params.registerRelationshipManagers("ElementSideNeighborLayers");
  params.addRequiredParam<unsigned short>("element_side_neighbor_layers",
                                          "Number of layers to ghost");

  params.addClassDescription("User object to calculate ghosted elements on a single processor or "
                             "the union across all processors.");
  return params;
}

ElemSideNeighborLayersTester::ElemSideNeighborLayersTester(const InputParameters & parameters)
  : ElementUOProvider(parameters),
    _rank(getParam<unsigned int>("rank")),
    _show_evaluable(getParam<bool>("show_evaluable"))
{
}

void
ElemSideNeighborLayersTester::initialize()
{
  _ghost_data.clear();
}

void
ElemSideNeighborLayersTester::execute()
{
  auto my_processor_id = processor_id();

  if (_rank == DofObject::invalid_processor_id || my_processor_id == _rank)
  {
    if (_show_evaluable)
    {
      for (const auto & current_elem : _fe_problem.getEvaluableElementRange())
        _ghost_data.emplace(current_elem->id());
    }
    else
    {
      const auto & mesh = _subproblem.mesh().getMesh();

      for (const auto & elem : mesh.active_element_ptr_range())
        if (elem->processor_id() != my_processor_id)
          _ghost_data.emplace(elem->id());
    }
  }
}

void
ElemSideNeighborLayersTester::finalize()
{
  _communicator.set_union(_ghost_data);
}

unsigned long
ElemSideNeighborLayersTester::getElementalValueLong(dof_id_type element_id,
                                                    const std::string & /*field_name*/) const
{
  return _ghost_data.find(element_id) != _ghost_data.end();
}
