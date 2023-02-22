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
#include "StochasticReporter.h"

class StochasticMatrix : public StochasticReporter
{
public:
  static InputParameters validParams();

  StochasticMatrix(const InputParameters & parameters);
  virtual void execute() override;

protected:
  // Overriding this to make sure sampler matches
  virtual ReporterName declareStochasticReporterClone(const Sampler & sampler,
                                                      const ReporterData & from_data,
                                                      const ReporterName & from_reporter,
                                                      std::string prefix = "") override;

  /// The sampler to extract data
  Sampler & _sampler;

private:
  /// Storage for declared vectors, one for each column
  std::vector<std::vector<Real> *> _sample_vectors;
};
