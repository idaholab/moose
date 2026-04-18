//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGenericFunctorVectorMaterial.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", GenericFunctorVectorMaterial);

/// Handle any numerical vector values, which should be enclosed in curly braces
std::vector<Moose::MFEM::VectorCoefficientName>
processLiteralVectors(const std::vector<Moose::MFEM::VectorCoefficientName> & input)
{
  std::vector<Moose::MFEM::VectorCoefficientName> result;
  bool in_literal = false;
  Moose::MFEM::VectorCoefficientName literal;
  for (const auto & item : input)
  {
    if (in_literal)
    {
      if (item.front() == '{')
        mooseError("Nested numeric vector values are not permitted in "
                   "MFEMGenericFunctorVectoMaterial prop_values.");
      else if (item.back() == '}')
      {
        in_literal = false;
        literal += " " + item.substr(0, item.size() - 1);
        result.push_back(literal);
      }
      else
        literal += " " + item;
    }
    else if (item.front() == '{')
    {
      if (item.back() == '}')
        result.push_back(item.substr(1, item.size() - 2));
      else
      {
        in_literal = true;
        literal = item.substr(1);
      }
    }
    else
      result.push_back(item);
  }
  if (in_literal)
    mooseError("No closing curly brace for vector value in "
               "Moose::MFEM::GenericFunctorVectorMaterial prop_values: '{" +
               literal + "'");
  return result;
}

namespace Moose::MFEM
{
InputParameters
GenericFunctorVectorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Declares material vector properties based on names and coefficients "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Moose::MFEM::VectorCoefficientName>>(
      "prop_values",
      "The corresponding names of coefficients associated with the named properties");

  return params;
}

GenericFunctorVectorMaterial::GenericFunctorVectorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(processLiteralVectors(
        getParam<std::vector<Moose::MFEM::VectorCoefficientName>>("prop_values")))
{
  if (_prop_names.size() != _prop_values.size())
    paramError("prop_names", "Must match the size of prop_values");

  for (const auto i : index_range(_prop_names))
    _properties.declareVectorProperty(_prop_names[i],
                                      isBoundaryRestricted() ? boundariesToStrings()
                                                             : subdomainsToStrings(),
                                      _prop_values[i]);
}

GenericFunctorVectorMaterial::~GenericFunctorVectorMaterial() {}

} // namespace Moose::MFEM
#endif
