//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NUMADAPTIVITYCYCLES_H
#define NUMADAPTIVITYCYCLES_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumAdaptivityCycles;

template <>
InputParameters validParams<NumAdaptivityCycles>();

/**
 * Just returns the number of adaptivity cyles needed.
 */
class NumAdaptivityCycles : public GeneralPostprocessor
{
public:
  NumAdaptivityCycles(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;
};

#endif // NUMADAPTIVITYCYCLES_H
