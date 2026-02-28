//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StochasticPackedVector.h"

registerMooseObject("StochasticToolsApp", StochasticPackedVector);

InputParameters
StochasticPackedVector::validParams()
{
  InputParameters params = StochasticReporter::validParams();
  params.addClassDescription(
      "Packs selected scalar sampler columns into a single vector<Real> per sample.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler from which to extract rows.");
  params.addRequiredParam<std::vector<unsigned int>>(
      "columns", "Zero-based indices of sampler columns to pack (size >= 2).");
  params.addRequiredParam<ReporterValueName>(
      "output_name", "Name of the packed vector reporter (vector<Real> per sample).");
  return params;
}

StochasticPackedVector::StochasticPackedVector(const InputParameters & parameters)
  : StochasticReporter(parameters),
    _sampler(getSampler("sampler")),
    _cols(getParam<std::vector<unsigned int>>("columns")),
    _packed_vec(nullptr)
{
  if (_cols.size() < 2)
    paramError("columns", "At least two column indices are required for packing.");

  // Bounds check column indices against sampler width
  const auto ncols = _sampler.getNumberOfCols();
  for (auto c : _cols)
    if (c >= ncols)
      paramError(
          "columns", "Column index ", c, " is out of range for sampler with ", ncols, " columns.");

  // Declare the packed reporter (vector<Real> per sample -> vector<vector<Real>> across samples)
  const auto & out_name = getParam<ReporterValueName>("output_name");
  _packed_vec = &declareStochasticReporter<std::vector<Real>>(out_name, _sampler);
}

void
StochasticPackedVector::execute()
{
  // Iterate local rows and pack requested columns into a length-_cols.size() vector
  for (dof_id_type i = 0; i < _sampler.getNumberOfLocalRows(); ++i)
  {
    const std::vector<Real> row = _sampler.getNextLocalRow();

    // Safety: row width must match sampler width
    if (row.size() != static_cast<size_t>(_sampler.getNumberOfCols()))
      mooseError("StochasticPackedVector: sampler row width (",
                 row.size(),
                 ") does not match sampler column count (",
                 _sampler.getNumberOfCols(),
                 ").");

    std::vector<Real> packed;
    packed.reserve(_cols.size());
    for (auto c : _cols)
      packed.push_back(row[c]);

    // Store as the vector for this sample
    (*_packed_vec)[i] = std::move(packed);
  }
}

ReporterName
StochasticPackedVector::declareStochasticReporterClone(const Sampler & sampler,
                                                       const ReporterData & from_data,
                                                       const ReporterName & from_reporter,
                                                       std::string prefix)
{
  if (sampler.name() != _sampler.name())
    paramError("sampler",
               "Attempting to create a stochastic value with a different sampler (",
               sampler.name(),
               ") than the one specified at input (",
               _sampler.name(),
               ").");

  return StochasticReporter::declareStochasticReporterClone(
      sampler, from_data, from_reporter, prefix);
}
