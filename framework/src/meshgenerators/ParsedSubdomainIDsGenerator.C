//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedSubdomainIDsGenerator.h"

registerMooseObject("MooseApp", ParsedSubdomainIDsGenerator);

InputParameters
ParsedSubdomainIDsGenerator::validParams()
{
  InputParameters params = ParsedSubdomainGeneratorBase::validParams();

  params.addClassDescription(
      "Uses a parsed expression to determine the subdomain ids of included elements.");

  return params;
}

ParsedSubdomainIDsGenerator::ParsedSubdomainIDsGenerator(const InputParameters & parameters)
  : ParsedSubdomainGeneratorBase(parameters)
{
}

void
ParsedSubdomainIDsGenerator::assignElemSubdomainID(Elem * elem)
{
  _func_params[0] = elem->vertex_average()(0);
  _func_params[1] = elem->vertex_average()(1);
  _func_params[2] = elem->vertex_average()(2);
  for (const auto i : index_range(_eeid_indices))
    _func_params[3 + i] = elem->get_extra_integer(_eeid_indices[i]);

  if (std::find(_excluded_ids.begin(), _excluded_ids.end(), elem->subdomain_id()) ==
      _excluded_ids.end())
  {
    const Real func_val = evaluate(_func_F);
    if (func_val < 0.0)
      paramError("expression",
                 "the value of the function is negative at the element with ID " +
                     std::to_string(elem->id()) +
                     ". The function must be non-negative. Consider using the absolute value of "
                     "the function.");
    elem->subdomain_id() = std::round(func_val);
  }
}
