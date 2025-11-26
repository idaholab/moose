//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "StochasticReporter.h"

class StochasticPackedVector : public StochasticReporter
{
public:
  static InputParameters validParams();

  StochasticPackedVector(const InputParameters & parameters);
  virtual void execute() override;

protected:
  // Overriding this to make sure sampler matches
  virtual ReporterName declareStochasticReporterClone(const Sampler & sampler,
                                                      const ReporterData & from_data,
                                                      const ReporterName & from_reporter,
                                                      std::string prefix = "") override;

  /// The sampler to extract data from
  Sampler & _sampler;

  /// Column indices to pack (size >= 2)
  const std::vector<unsigned int> _cols;

private:
  /// The declared packed output: one vector<Real> per sample
  std::vector<std::vector<Real>> * _packed_vec;
};
