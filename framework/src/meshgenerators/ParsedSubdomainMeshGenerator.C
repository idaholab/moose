//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedSubdomainMeshGenerator.h"

registerMooseObject("MooseApp", ParsedSubdomainMeshGenerator);

InputParameters
ParsedSubdomainMeshGenerator::validParams()
{
  InputParameters params = ParsedSubdomainGeneratorBase::validParams();

  params.addRequiredParam<std::string>("combinatorial_geometry",
                                       "Function expression encoding a combinatorial geometry");
  params.addRequiredParam<subdomain_id_type>("block_id",
                                             "Subdomain id to set for inside of the combinatorial");
  params.addParam<SubdomainName>("block_name",
                                 "Subdomain name to set for inside of the combinatorial");
  params.addClassDescription(
      "Uses a parsed expression (`combinatorial_geometry`) to determine if an "
      "element (via its centroid) is inside the region defined by the expression and "
      "assigns a new block ID.");

  return params;
}

ParsedSubdomainMeshGenerator::ParsedSubdomainMeshGenerator(const InputParameters & parameters)
  : ParsedSubdomainGeneratorBase(parameters),
    _function(parameters.get<std::string>("combinatorial_geometry"))
{
  functionInitialize(_function);
}

void
ParsedSubdomainMeshGenerator::assignElemSubdomainID(Elem * elem)
{
  _func_params[0] = elem->vertex_average()(0);
  _func_params[1] = elem->vertex_average()(1);
  _func_params[2] = elem->vertex_average()(2);
  for (const auto i : index_range(_eeid_indices))
    _func_params[3 + i] = elem->get_extra_integer(_eeid_indices[i]);
  bool contains = evaluate(_func_F);

  if (contains && std::find(_excluded_ids.begin(), _excluded_ids.end(), elem->subdomain_id()) ==
                      _excluded_ids.end())
    elem->subdomain_id() = _block_id;
}
