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

/**
 * A tool for output Sampler data.
 */
class SamplerData : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  SamplerData(const InputParameters & parameters);
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;

protected:
  /// Storage for declared vectors, one for each column
  std::vector<VectorPostprocessorValue *> _sample_vectors;

  /// The sampler to extract data
  Sampler & _sampler;

  /// The method of data retrival from the Sample
  const MooseEnum & _sampler_method;
};
