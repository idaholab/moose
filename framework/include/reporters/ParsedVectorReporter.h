//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "ParsedReporterBase.h"

/**
 * Reporter containing operation between vectors from another Reporter
 */
class ParsedVectorReporter : public ParsedReporterBase
{
public:
  static InputParameters validParams();

  ParsedVectorReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  /// input reporter vectors
  std::vector<const std::vector<Real> *> _vector_reporter_data;
  /// output vector
  std::vector<double> & _output_reporter;
};
