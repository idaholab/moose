//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedExtraElementIDGenerator.h"

#include "Conversion.h"
#include "MooseMeshUtils.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", ParsedExtraElementIDGenerator);

InputParameters
ParsedExtraElementIDGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::string>(
      "expression", "Function expression to return the extra element ID based on element centroid");
  params.addRequiredParam<std::string>(
      "extra_elem_integer_name", "Name of the extra element integer to be added by this generator");
  params.addParam<std::vector<SubdomainName>>("restricted_subdomains",
                                              "Only set ids for elements within these restricted "
                                              "subdomains (empty means the entire domain)");
  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParam<std::vector<ExtraElementIDName>>(
      "extra_element_id_names", {}, "Extra element integers used in the parsed expression");
  params.addClassDescription(
      "Uses a parsed expression to set an extra element id for elements (via their centroids).");

  return params;
}

ParsedExtraElementIDGenerator::ParsedExtraElementIDGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _input(getMesh("input")),
    _function(parameters.get<std::string>("expression")),
    _eeid_names(getParam<std::vector<ExtraElementIDName>>("extra_element_id_names")),
    _elem_id_name(getParam<std::string>("extra_elem_integer_name"))
{
  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string>>("constant_names"),
                      getParam<std::vector<std::string>>("constant_expressions"));

  // add the extra element integers
  std::string symbol_str = "x,y,z";
  for (const auto & eeid_name : _eeid_names)
    symbol_str += "," + eeid_name;

  // parse function
  if (_func_F->Parse(_function, symbol_str) >= 0)
    mooseError("Invalid function\n",
               _function,
               "\nin ParsedExtraElementIDGenerator ",
               name(),
               ".\n",
               _func_F->ErrorMsg());

  _func_params.resize(3 + _eeid_names.size());
}

std::unique_ptr<MeshBase>
ParsedExtraElementIDGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // the extra element ids would not have existed at construction so we only do this now
  for (const auto & eeid_name : _eeid_names)
    _eeid_indices.push_back(mesh->get_elem_integer_index(eeid_name));

  bool has_restriction = isParamValid("restricted_subdomains");
  std::set<SubdomainID> restricted_blocks;
  if (has_restriction)
  {
    auto names = getParam<std::vector<SubdomainName>>("restricted_subdomains");
    for (auto & name : names)
    {
      // check that the subdomain exists in the mesh
      if (!MooseMeshUtils::hasSubdomainName(*mesh, name))
        paramError("restricted_subdomains", "The block '", name, "' was not found in the mesh");

      restricted_blocks.insert(MooseMeshUtils::getSubdomainID(name, *mesh));
    }
  }

  if (!mesh->has_elem_integer(_elem_id_name))
    _extra_elem_id = mesh->add_elem_integer(_elem_id_name);
  else
    _extra_elem_id = mesh->get_elem_integer_index(_elem_id_name);

  // Loop over the elements
  for (const auto & elem : mesh->active_element_ptr_range())
  {
    if (has_restriction && restricted_blocks.count(elem->subdomain_id()) == 0)
      continue;

    const auto centroid = elem->true_centroid();
    _func_params[0] = centroid(0);
    _func_params[1] = centroid(1);
    _func_params[2] = centroid(2);
    for (const auto i : index_range(_eeid_indices))
      _func_params[3 + i] = elem->get_extra_integer(_eeid_indices[i]);
    const auto id = evaluate(_func_F);

    elem->set_extra_integer(_extra_elem_id, id);
  }

  return mesh;
}
