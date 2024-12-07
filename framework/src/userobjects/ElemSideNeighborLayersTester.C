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

registerMooseObject("MooseApp", ElemSideNeighborLayersTester);

InputParameters
ElemSideNeighborLayersTester::validParams()
{
  InputParameters params = ElementUOProvider::validParams();
  params.addParam<unsigned int>(
      "rank",
      libMesh::DofObject::invalid_processor_id,
      "The rank for which the ghosted elements are recorded (Default: ALL)");

  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC,

      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        rm_params.set<unsigned short>("layers") =
            obj_params.get<unsigned short>("element_side_neighbor_layers");
      }

  );

  params.addRequiredParam<unsigned short>("element_side_neighbor_layers",
                                          "Number of layers to ghost");

  params.addClassDescription("User object to calculate ghosted elements on a single processor or "
                             "the union across all processors.");
  return params;
}

ElemSideNeighborLayersTester::ElemSideNeighborLayersTester(const InputParameters & parameters)
  : ElementUOProvider(parameters), _rank(getParam<unsigned int>("rank"))
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

  if (_rank == libMesh::DofObject::invalid_processor_id || my_processor_id == _rank)
  {
    for (const auto & current_elem : _fe_problem.getNonlinearEvaluableElementRange())
      _evaluable_data.emplace(current_elem->id());

    const auto & mesh = _subproblem.mesh().getMesh();

    for (const auto & elem : mesh.active_element_ptr_range())
      if (elem->processor_id() != my_processor_id)
        _ghost_data.emplace(elem->id());
  }
}

void
ElemSideNeighborLayersTester::finalize()
{
  _communicator.set_union(_ghost_data);
  _communicator.set_union(_evaluable_data);
}

unsigned long
ElemSideNeighborLayersTester::getElementalValueLong(dof_id_type element_id,
                                                    const std::string & field_name) const
{
  if (field_name == "evaluable")
    return _evaluable_data.find(element_id) != _evaluable_data.end();
  else if (field_name == "ghosted")
    return _ghost_data.find(element_id) != _ghost_data.end();

  return std::numeric_limits<unsigned long>::max();
}
