//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMGenericFunctorVectorMaterial.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGenericFunctorVectorMaterial);

/// Handle any numerical vector values, which should be enclosed in curly braces
std::vector<MFEMVectorCoefficientName> processLiteralVectors(const std::vector<MFEMVectorCoefficientName> & input)
{
  std::vector<MFEMVectorCoefficientName> result;
  bool in_literal = false;
  MFEMVectorCoefficientName literal;
  for (const auto & item : input)
  {
    if (in_literal)
    {
      if (item.front() == '{')
      {
        mooseError("Nested numeric vector values are not permitted in "
                   "MFEMGenericFunctorVectoMaterial prop_values.");
      }
      else if (item.back() == '}')
      {
        in_literal = false;
        literal += " " + item.substr(0, item.size() - 1);
        result.push_back(literal);
      }
      else
      {
        literal += " " + item;
      }
    }
    else if(item.front() == '{')
    {
      if(item.back() == '}')
      {
        result.push_back(item.substr(1, item.size() - 2));
      }
      else
      {
        in_literal = true;
        literal = item.substr(1);
      }
    }
    else
    {
      result.push_back(item);
    }
  }
  if (in_literal)
  {
    mooseError("No closing curly brace for vector value in "
               "MFEMGenericFunctorVectorMaterial prop_values: '{" + literal + "'");
  }
  return result;
}

InputParameters
MFEMGenericFunctorVectorMaterial::validParams()
{
  InputParameters params = MFEMFunctorMaterial::validParams();
  params.addClassDescription("Declares material vector properties based on names and coefficients "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<MFEMVectorCoefficientName>>(
      "prop_values",
      "The corresponding names of coefficients associated with the named properties. A coefficient "
      "can be any of the following: a variable, an MFEM material property, a function, "
      "a post-processor, or a numeric vector value (enclosed in curly braces).");

  return params;
}

MFEMGenericFunctorVectorMaterial::MFEMGenericFunctorVectorMaterial(
    const InputParameters & parameters)
  : MFEMFunctorMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(processLiteralVectors(getParam<std::vector<MFEMVectorCoefficientName>>("prop_values")))
{
  if (_prop_names.size() != _prop_values.size())
    paramError("prop_names", "Must match the size of prop_values");

  for (const auto i : index_range(_prop_names))
    _properties.declareVectorProperty(
        _prop_names[i], subdomainsToStrings(_block_ids), _prop_values[i]);
}

MFEMGenericFunctorVectorMaterial::~MFEMGenericFunctorVectorMaterial() {}

#endif
