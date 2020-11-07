//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantReporter.h"

registerMooseObject("MooseApp", ConstantReporter);

InputParameters
ConstantReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  params.addRequiredParam<std::vector<ReporterValueName>>(
      "names", "List of names for each reporter value created.");
  MultiMooseEnum types("integer real vector string");
  params.addRequiredParam<MultiMooseEnum>(
      "value_types", types, "List of types for each reporter value created.");

  params.addParam<std::vector<int>>("integer_values", "Values for integers.");
  params.addParam<std::vector<Real>>("real_values", "Values for reals.");
  params.addParam<std::vector<std::vector<Real>>>("vector_values", "Values for vectors.");
  params.addParam<std::vector<std::string>>("string_values", "Values for strings.");
  return params;
}

ConstantReporter::ConstantReporter(const InputParameters & parameters) : GeneralReporter(parameters)
{
  const auto & names = getParam<std::vector<ReporterValueName>>("names");
  const MultiMooseEnum & types(getParam<MultiMooseEnum>("value_types"));
  if (names.size() != types.size())
    paramError("value_types", "names and value_types must be the same size.");

  if (!isParamValid("integer_values") && types.contains("integer"))
    paramError("value_types", "Must specify integer_values when creating integer reporter.");
  if (!isParamValid("real_values") && types.contains("real"))
    paramError("value_types", "Must specify real_values when creating real reporter.");
  if (!isParamValid("vector_values") && types.contains("vector"))
    paramError("value_types", "Must specify vector_values when creating vector reporter.");
  if (!isParamValid("string_values") && types.contains("string"))
    paramError("value_types", "Must specify string_values when creating string reporter.");

  std::vector<int> int_val = getParam<std::vector<int>>("integer_values");
  std::vector<Real> real_val = getParam<std::vector<Real>>("real_values");
  std::vector<std::vector<Real>> vector_val =
      getParam<std::vector<std::vector<Real>>>("vector_values");
  std::vector<std::string> string_val = getParam<std::vector<std::string>>("string_values");

  unsigned int ii = 0;
  unsigned int ir = 0;
  unsigned int iv = 0;
  unsigned int is = 0;
  for (unsigned int i = 0; i < names.size(); ++i)
  {
    if (types[i] == "integer")
    {
      if (ii >= int_val.size())
        paramError("integer_values", "Not enough values provided.");
      _int.push_back(&declareValueByName<int>(names[i], int_val[ii++]));
    }

    else if (types[i] == "real")
    {
      if (ir >= real_val.size())
        paramError("real_values", "Not enough values provided.");
      _real.push_back(&declareValueByName<Real>(names[i], real_val[ir++]));
    }

    else if (types[i] == "vector")
    {
      if (iv >= vector_val.size())
        paramError("vector_values", "Not enough values provided.");
      _vector.push_back(&declareValueByName<std::vector<Real>>(names[i], vector_val[iv++]));
    }

    else if (types[i] == "string")
    {
      if (is >= string_val.size())
        paramError("string_values", "Not enough values provided.");
      _string.push_back(&declareValueByName<std::string>(names[i], string_val[is++]));
    }
  }

  if (ii < int_val.size())
    paramError("integer_values", "Unused values.");
  if (ir < real_val.size())
    paramError("real_values", "Unused values.");
  if (iv < vector_val.size())
    paramError("vector_values", "Unused values.");
  if (is < string_val.size())
    paramError("string_values", "Unused values.");
}
