//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "SamplerInterface.h"
#include "SurrogateModel.h"

class SurrogateTester;

template <>
InputParameters validParams<SurrogateTester>();

/**
 * A tool for output Sampler data.
 */
class SurrogateTester : public GeneralVectorPostprocessor, SamplerInterface
{
public:
  static InputParameters validParams();

  SurrogateTester(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// Sampler for evaluating surrogate model
  Sampler & _sampler;
  /// Where or not to output all the samples used
  const bool _output_samples;
  /// Reference to surrogate model
  const SurrogateModel & _model;
  /// Vector containing results of sampling PCE model
  VectorPostprocessorValue & _value_vector;
  /// Vector containing all the sample points for each parameter
  std::vector<VectorPostprocessorValue *> _sample_vector;
};
