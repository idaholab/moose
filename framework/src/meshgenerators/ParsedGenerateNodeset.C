//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedGenerateNodeset.h"
#include "Conversion.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"
#include "libmesh/fe_base.h"

#include <typeinfo>

registerMooseObject("MooseApp", ParsedGenerateNodeset);

InputParameters
ParsedGenerateNodeset::validParams()
{
  InputParameters params = NodeSetsGeneratorBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<BoundaryName>("new_nodeset_name", "The name of the new nodeset");

  params.addRequiredParam<std::string>("expression",
                                       "Function expression describing the geometric constraints "
                                       "that the node must meet to be included in the nodeset");
  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");

  // This nodeset generator can only handle a single new nodeset name, not a vector of names
  params.suppressParameter<std::vector<BoundaryName>>("new_nodeset");

  params.addClassDescription("A MeshGenerator that adds nodes to a nodeset if the "
                             "node satisfies the `expression` expression.");
  params.addParamNamesToGroup("expression constant_names constant_expressions",
                              "Parsed expression");
  return params;
}

ParsedGenerateNodeset::ParsedGenerateNodeset(const InputParameters & parameters)
  : NodeSetsGeneratorBase(parameters),
    FunctionParserUtils<false>(parameters),
    _function(parameters.get<std::string>("expression"))
{
  _nodeset_names.push_back(getParam<BoundaryName>("new_nodeset_name"));

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // parse function
  if (_func_F->Parse(_function, "x,y,z") >= 0)
    paramError("expression", "Invalid function\n", _function, "\n", _func_F->ErrorMsg());

  _func_params.resize(3);
}

std::unique_ptr<MeshBase>
ParsedGenerateNodeset::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  setup(*mesh);

  if (!mesh->is_replicated())
    mooseError("Not implemented for distributed meshes");

  // Get a reference to our BoundaryInfo object for later use
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  // Get a reference to the node to nodeset map
  const auto & nodeset_map = boundary_info.get_nodeset_map();

  // Get the BoundaryIDs from the mesh
  std::vector<boundary_id_type> nodeset_ids =
      MooseMeshUtils::getBoundaryIDs(*mesh, _nodeset_names, true);
  mooseAssert(nodeset_ids.size() == 1, "Length of nodeset_ids should be one");

  // Loop over nodes
  for (const auto curr_node : as_range(mesh->active_nodes_begin(), mesh->active_nodes_end()))
  {
    // Get all nodesets the node is currently a part of
    const auto & node_nodesets_iters = nodeset_map.equal_range(curr_node);

    // Copy into a vector to accommodate this developer's low skills
    std::vector<BoundaryID> node_nodesets;
    for (auto i = node_nodesets_iters.first; i != node_nodesets_iters.second; ++i)
      node_nodesets.push_back(i->second);

    // Get all the elements the node is a part of
    const auto & node_elems = _node_to_elem_map[curr_node->id()];

    // Check all the constraints specified in base class
    if (!nodeSatisfiesRequirements(curr_node, node_nodesets, node_elems, *mesh))
      continue;

    // Check expression
    _func_params[0] = (*curr_node)(0);
    _func_params[1] = (*curr_node)(1);
    _func_params[2] = (*curr_node)(2);
    if (evaluate(_func_F))
    {
      if (_replace)
      {
        for (const auto nodeset_id : node_nodesets)
          boundary_info.remove_node(curr_node, nodeset_id);
      }
      boundary_info.add_node(curr_node, nodeset_ids[0]);
    }
  }
  boundary_info.nodeset_name(nodeset_ids[0]) = _nodeset_names[0];

  // TODO: consider if a new nodeset actually impacts preparedness
  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}
