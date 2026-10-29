//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPiecewiseConstantFromCSV.h"

#include "MFEMProblem.h"
#include "SubProblem.h"

#include "mfem.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

registerMooseObject("MooseApp", MFEMPiecewiseConstantFromCSV);

InputParameters
MFEMPiecewiseConstantFromCSV::validParams()
{
  InputParameters params = Function::validParams();

  params.addRequiredParam<FileName>("csv_file", "Delimited numeric input file.");

  params.addParam<std::string>("attribute_column",
                               "Name of the column containing MFEM element attribute IDs.");
  params.addParam<unsigned int>("attribute_column_index",
                                "Zero-based index of the column containing MFEM element attribute IDs.");
  params.addParam<std::vector<unsigned int>>(
      "attributes",
      "MFEM element attribute IDs corresponding, in order, to the selected value column.");

  params.addParam<std::string>("value_column", "Name of the coefficient-value column.");
  params.addParam<unsigned int>("value_column_index",
                                "Zero-based index of the coefficient-value column.");

  params.addParam<Real>("default_value",
                        0.0,
                        "Value assigned to mesh attributes not present in the supplied mapping.");
  params.addParam<bool>("ignore_empty_lines", true, "Whether empty input lines are ignored.");

  MooseEnum header("auto true false", "auto");
  params.addParam<MooseEnum>("header", header, "Header handling for the delimited file.");
  params.addParam<std::string>("delimiter",
                               "Optional delimiter. If omitted, it is detected automatically.");

  params.addClassDescription(
      "Creates an MFEM PWConstCoefficient from attribute-indexed values in a delimited file. "
      "Attributes may be read from a named/indexed column or supplied explicitly.");

  return params;
}

MFEMPiecewiseConstantFromCSV::MFEMPiecewiseConstantFromCSV(const InputParameters & parameters)
  : Function(parameters),
    _mfem_problem(
        cast_ref<MFEMProblem &>(*parameters.getCheckedPointerParam<SubProblem *>("_subproblem"))),
    _coefficient(
        _mfem_problem.getCoefficients().declareScalar<mfem::PWConstCoefficient>(name(), 0)),
    _csv_file(getParam<FileName>("csv_file")),
    _default_value(getParam<Real>("default_value")),
    _ignore_empty_lines(getParam<bool>("ignore_empty_lines"))
{
  const unsigned int attribute_sources = isParamValid("attribute_column") +
                                         isParamValid("attribute_column_index") +
                                         isParamValid("attributes");
  if (attribute_sources != 1)
    paramError("attribute_column",
               "Specify exactly one of 'attribute_column', 'attribute_column_index', or "
               "'attributes'.");

  const unsigned int value_sources =
      isParamValid("value_column") + isParamValid("value_column_index");
  if (value_sources != 1)
    paramError("value_column",
               "Specify exactly one of 'value_column' or 'value_column_index'.");

  if (getParam<MooseEnum>("header") == "false" &&
      (isParamValid("attribute_column") || isParamValid("value_column")))
    paramError("header", "Column-name selection requires a header; use column indices instead.");
}

const std::vector<double> &
MFEMPiecewiseConstantFromCSV::getColumn(const MooseUtils::DelimitedFileReader & reader,
                                        const std::string & name_param,
                                        const std::string & index_param) const
{
  if (isParamValid(name_param))
    return reader.getData(getParam<std::string>(name_param));

  return reader.getData(getParam<unsigned int>(index_param));
}

std::vector<int>
MFEMPiecewiseConstantFromCSV::getAttributes(const MooseUtils::DelimitedFileReader & reader,
                                            const std::size_t number_of_values) const
{
  std::vector<int> attributes;

  if (isParamValid("attributes"))
  {
    const auto & supplied = getParam<std::vector<unsigned int>>("attributes");
    if (supplied.size() != number_of_values)
      paramError("attributes", "The number of attributes must match the number of values.");

    attributes.reserve(supplied.size());
    for (const auto attribute : supplied)
    {
      if (attribute == 0 || attribute > static_cast<unsigned int>(std::numeric_limits<int>::max()))
        paramError("attributes", "MFEM attributes must be positive integer IDs.");
      attributes.push_back(static_cast<int>(attribute));
    }
  }
  else
  {
    const auto & data = getColumn(reader, "attribute_column", "attribute_column_index");
    if (data.size() != number_of_values)
      paramError("attribute_column", "The number of attributes must match the number of values.");

    attributes.reserve(data.size());
    for (const auto raw : data)
    {
      if (!std::isfinite(raw) || raw <= 0.0 || raw > std::numeric_limits<int>::max() ||
          std::trunc(raw) != raw)
        paramError("attribute_column", "MFEM attributes must be positive integer IDs.");
      attributes.push_back(static_cast<int>(raw));
    }
  }

  const std::set<int> unique(attributes.begin(), attributes.end());
  if (unique.size() != attributes.size())
    mooseError("MFEM attributes must be unique in MFEMPiecewiseConstantFromCSV.");

  return attributes;
}

void
MFEMPiecewiseConstantFromCSV::initialSetup()
{
  MooseUtils::DelimitedFileReader reader(_csv_file, &_communicator);
  reader.setIgnoreEmptyLines(_ignore_empty_lines);

  const MooseEnum & header = getParam<MooseEnum>("header");
  if (header == "true")
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::ON);
  else if (header == "false")
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::OFF);
  else
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::AUTO);

  if (isParamValid("delimiter"))
    reader.setDelimiter(getParam<std::string>("delimiter"));

  reader.read();

  const auto & values = getColumn(reader, "value_column", "value_column_index");
  if (values.empty())
    paramError("csv_file", "The selected value column is empty.");

  for (const auto value : values)
    if (!std::isfinite(value))
      paramError("csv_file", "The selected value column contains a non-finite value.");

  const auto attributes = getAttributes(reader, values.size());

  int max_attribute = 0;
  const auto & mesh = _mfem_problem.mesh().getMFEMParMesh();
  if (mesh.attributes.Size())
    max_attribute = mesh.attributes.Max();

  max_attribute = std::max(max_attribute, *std::max_element(attributes.begin(), attributes.end()));
  _communicator.max(max_attribute);

  mfem::Vector constants(max_attribute);
  constants = _default_value;

  for (std::size_t i = 0; i < attributes.size(); ++i)
    constants(attributes[i] - 1) = values[i];

  _coefficient.UpdateConstants(constants);
}

Real
MFEMPiecewiseConstantFromCSV::value(Real, const Point &) const
{
  mooseError("MFEMPiecewiseConstantFromCSV must be evaluated through the MFEM coefficient interface.");
  return 0.0;
}

#endif
