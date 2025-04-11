//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedSubdomainGeneratorBase.h"
#include "Conversion.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/fparser_ad.hh"
#include "libmesh/elem.h"

InputParameters
ParsedSubdomainGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<SubdomainName>>(
      "excluded_subdomains",
      "A set of subdomain names that will not changed even if "
      "they are inside/outside the combinatorial geometry");
  params.addDeprecatedParam<std::vector<subdomain_id_type>>(
      "excluded_subdomain_ids",
      "A set of subdomain ids that will not changed even if "
      "they are inside/outside the combinatorial geometry",
      "excluded_subdomain_ids is deprecated, use excluded_subdomains (ids or names accepted)");
  params.addParam<std::vector<std::string>>(
      "constant_names", {}, "Vector of constants used in the parsed function");
  params.addParam<std::vector<std::string>>(
      "constant_expressions",
      {},
      "Vector of values for the constants in constant_names (can be an FParser expression)");
  params.addParam<std::vector<ExtraElementIDName>>(
      "extra_element_id_names", {}, "Extra element integers used in the parsed expression");
  params.addClassDescription("A base class for mesh generators that Use a parsed expression to "
                             "assign new subdomain id(s).");

  return params;
}

ParsedSubdomainGeneratorBase::ParsedSubdomainGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _input(getMesh("input")),
    _block_id(isParamValid("block_id") ? parameters.get<subdomain_id_type>("block_id")
                                       : Elem::invalid_subdomain_id),
    _excluded_ids(isParamValid("excluded_subdomain_ids")
                      ? parameters.get<std::vector<subdomain_id_type>>("excluded_subdomain_ids")
                      : std::vector<subdomain_id_type>()),
    _eeid_names(getParam<std::vector<ExtraElementIDName>>("extra_element_id_names"))
{
}

std::unique_ptr<MeshBase>
ParsedSubdomainGeneratorBase::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // the extra element ids would not have existed at construction so we only do this now
  for (const auto & eeid_name : _eeid_names)
    _eeid_indices.push_back(mesh->get_elem_integer_index(eeid_name));

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
    assignElemSubdomainID(elem);

  // Assign block name, if provided
  if (isParamValid("block_name"))
    mesh->subdomain_name(_block_id) = getParam<SubdomainName>("block_name");

  mesh->set_isnt_prepared();
  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
ParsedSubdomainGeneratorBase::functionInitialize(const std::string & function_expression)
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
  if (_func_F->Parse(function_expression, symbol_str) >= 0)
    mooseError("Invalid function\n",
               function_expression,
               "\nin ",
               type(),
               " ",
               name(),
               ".\n",
               _func_F->ErrorMsg());

  _func_params.resize(3 + _eeid_names.size());
}
