//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideSetsFromNodeSetsGenerator.h"

#include "CastUniquePointer.h"
#include "libmesh/boundary_info.h"
#include "libmesh/elem_side_builder.h"

registerMooseObject("MooseApp", SideSetsFromNodeSetsGenerator);

InputParameters
SideSetsFromNodeSetsGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addClassDescription("Mesh generator which constructs side sets from node sets");
  params.addRequiredParam<MeshGeneratorName>("input",
                                             "Input mesh the operation will be applied to");
  params.addParam<std::vector<BoundaryName>>(
      "nodesets_to_convert",
      "If specified, list of nodesets to convert. If not specified, all nodesets are converted");
  return params;
}

SideSetsFromNodeSetsGenerator::SideSetsFromNodeSetsGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
SideSetsFromNodeSetsGenerator::generate()
{
  if (!isParamValid("nodesets_to_convert"))
    _input->get_boundary_info().build_side_list_from_node_list();
  else
  {
    const auto & nodeset_names = getParam<std::vector<BoundaryName>>("nodesets_to_convert");
    auto & binfo = _input->get_boundary_info();

    std::set<BoundaryID> nodeset_ids;
    for (const auto & nodeset_name : nodeset_names)
    {
      // Look through the nodeset map.
      BoundaryID nodeset_id = std::numeric_limits<BoundaryID>::max();
      for (const auto & [id, name] : binfo.get_nodeset_name_map())
        if (name == nodeset_name)
          nodeset_id = id;
      if (MooseUtils::isDigits(nodeset_name))
        nodeset_id = std::stoi(nodeset_name);
      if (nodeset_id == std::numeric_limits<BoundaryID>::max())
        paramError("nodesets_to_convert",
                   "Nodeset '" + nodeset_name + "' does not exist in the input mesh");
      nodeset_ids.insert(nodeset_id);
    }
    _input->get_boundary_info().build_side_list_from_node_list(nodeset_ids);
  }

  return dynamic_pointer_cast<MeshBase>(_input);
}
