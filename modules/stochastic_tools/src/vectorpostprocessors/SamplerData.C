//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "SamplerData.h"

// MOOSE includes
#include "Sampler.h"

template <>
InputParameters
validParams<SamplerData>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addClassDescription(
      "Tool for extracting Sampler object data and storing in VectorPostprocessor vectors.");
  params += validParams<SamplerInterface>();
  params.addRequiredParam<SamplerName>("sampler",
                                       "The sample from which to extract distribution data.");
  params.addParam<bool>("output_column_and_row_sizes",
                        false,
                        "Whether to output the number of "
                        "columns and rows in the matrix in "
                        "the first two rows of output");
  return params;
}

SamplerData::SamplerData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    _sampler(getSampler("sampler")),
    _output_col_row_sizes(getParam<bool>("output_column_and_row_sizes"))
{
}

void
SamplerData::initialize()
{
  for (auto ptr : _sample_vectors)
    ptr->clear();
}

void
SamplerData::execute()
{
  std::vector<DenseMatrix<Real>> data = _sampler.getSamples();
  auto n = data.size();
  if (_sample_vectors.empty())
  {
    _sample_vectors.resize(n);
    for (auto i = beginIndex(data); i < n; ++i)
    {
      std::string name = "mat_" + std::to_string(i);
      _sample_vectors[i] = &declareVector(name);
    }
  }

  for (auto i = beginIndex(data); i < n; ++i)
  {
    const std::size_t offset = _output_col_row_sizes ? 2 : 0;
    const std::size_t vec_size = data[i].get_values().size() + offset;
    _sample_vectors[i]->resize(vec_size);
    if (_output_col_row_sizes)
    {
      (*_sample_vectors[i])[0] = data[i].n(); // number of columns
      (*_sample_vectors[i])[1] = data[i].m(); // number of rows
    }
    std::copy(data[i].get_values().begin(),
              data[i].get_values().end(),
              _sample_vectors[i]->begin() + offset);
  }
}
