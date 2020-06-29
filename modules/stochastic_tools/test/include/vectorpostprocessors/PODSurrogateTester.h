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
#include "SurrogateModelInterface.h"
#include "PODReducedBasisSurrogate.h"

/**
 * A tool for output Sampler data.
 */
class PODSurrogateTester : public GeneralVectorPostprocessor, SamplerInterface, SurrogateModelInterface
{
public:
  static InputParameters validParams();

  PODSurrogateTester(const InputParameters & parameters);
  virtual void initialSetup() override;
  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:

  /// Sampler for evaluating surrogate model
  Sampler & _sampler;
  /// Where or not to output all the samples used
  const bool _output_samples;
  /// Reference to surrogate model
  PODReducedBasisSurrogate * _model;
  /// Vector containing results of sampling PCE model
  VectorPostprocessorValue & _value_vector;
  /// Vector containing all the sample points for each parameter
  std::vector<VectorPostprocessorValue *> _sample_vector;

  std::string _variable_name;

  MooseEnum _to_compute;
};
