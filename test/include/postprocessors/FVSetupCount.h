//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class FVSetupCounter;

/**
 * Reports how many times a named finite volume object has received one of the setup methods,
 * so that setup dispatch can be tested with a CSVDiff on the count rather than on the mere
 * presence of a call.
 */
class FVSetupCount : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  FVSetupCount(const InputParameters & params);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual PostprocessorValue getValue() const override;

protected:
  /// Name of the object whose count is reported
  const std::string _object_name;

  /// Which setup method to report the count of
  const MooseEnum _count_type;

  /// With count_type = CUSTOM, restricts the count to this execution flag
  const std::string _exec_flag;

private:
  /// Find the named object, which is either a finite volume residual object held in TheWarehouse
  /// or a finite volume initial condition, which lives in its own warehouse instead
  const FVSetupCounter & getCounter() const;
};
