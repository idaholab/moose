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
 * Reporter containing operation between scalars from another Reporter
 */
class ParsedScalarReporter : public ParsedReporterBase
{
public:
  static InputParameters validParams();

  ParsedScalarReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual void finalize() override;

private:
  /// output scalar
  Real & _output_reporter;
};
