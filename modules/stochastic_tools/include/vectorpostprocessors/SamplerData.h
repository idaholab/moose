//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SAMPLERDATA_H
#define SAMPLERDATA_H

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "SamplerInterface.h"

class SamplerData;

template <>
InputParameters validParams<SamplerData>();

/**
 * A tool for output Sampler data.
 */
class SamplerData : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  SamplerData(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual execute() override;

protected:
  /// Storage for declared vectors
  std::vector<VectorPostprocessorValue *> _sample_vectors;

  /// The sampler to extract data
  Sampler & _sampler;

  /// Whether to output the number of rows and columns in the first two rows of output
  const bool & _output_col_row_sizes;
};

#endif
