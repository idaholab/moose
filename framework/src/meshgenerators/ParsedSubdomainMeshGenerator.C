//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedSubdomainMeshGenerator.h"
#include "Conversion.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", ParsedSubdomainMeshGenerator);

InputParameters
ParsedSubdomainMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::string>("combinatorial_geometry",
                                       "Function expression encoding a combinatorial geometry");
  params.addRequiredParam<subdomain_id_type>("block_id",
                                             "Subdomain id to set for inside of the combinatorial");
  params.addParam<SubdomainName>("block_name",
                                 "Subdomain name to set for inside of the combinatorial");
  params.addParam<std::vector<SubdomainName>>(
      "excluded_subdomains",
      "A set of subdomain ids that will not changed even if "
      "they are inside/outside the combinatorial geometry");
  params.addDeprecatedParam<std::vector<subdomain_id_type>>(
      "excluded_subdomain_ids",
      "A set of subdomain ids that will not changed even if "
      "they are inside/outside the combinatorial geometry",
      "excluded_subdomain_ids is deprecated, use excluded_subdomains (ids or names accepted)");
  params.addParam<std::vector<std::string>>("constant_names",
                                            "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addClassDescription(
      "Uses a parsed expression (`combinatorial_geometry`) to determine if an "
      "element (via its centroid) is inside the region defined by the expression and "
      "assigns a new block ID.");

  return params;
}

ParsedSubdomainMeshGenerator::ParsedSubdomainMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _input(getMesh("input")),
    _function(parameters.get<std::string>("combinatorial_geometry")),
    _block_id(parameters.get<SubdomainID>("block_id")),
    _excluded_ids(isParamValid("excluded_subdomain_ids")
                      ? parameters.get<std::vector<subdomain_id_type>>("excluded_subdomain_ids")
                      : std::vector<subdomain_id_type>())
{
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
    mooseError("Invalid function\n",
               _function,
               "\nin ParsedSubdomainMeshGenerator ",
               name(),
               ".\n",
               _func_F->ErrorMsg());

  _func_params.resize(3);
}

std::unique_ptr<MeshBase>
ParsedSubdomainMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  if (isParamValid("excluded_subdomains"))
  {
    auto excluded_subdomains = parameters().get<std::vector<SubdomainName>>("excluded_subdomains");

    // check that the subdomains exist in the mesh
    for (const auto & name : excluded_subdomains)
      if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
        paramError("excluded_subdomains", "The block '", name, "' was not found in the mesh");

    _excluded_ids = MooseMeshUtils::getSubdomainIDs(*mesh, excluded_subdomains);
  }

  // Loop over the elements
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    _func_params[0] = elem->vertex_average()(0);
    _func_params[1] = elem->vertex_average()(1);
    _func_params[2] = elem->vertex_average()(2);
    bool contains = evaluate(_func_F);

    if (contains && std::find(_excluded_ids.begin(), _excluded_ids.end(), elem->subdomain_id()) ==
                        _excluded_ids.end())
      elem->subdomain_id() = _block_id;
  }

  // Assign block name, if provided
  if (isParamValid("block_name"))
    mesh->subdomain_name(_block_id) = getParam<SubdomainName>("block_name");

  return dynamic_pointer_cast<MeshBase>(mesh);
}
