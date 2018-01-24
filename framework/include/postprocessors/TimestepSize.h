//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TIMESTEPSIZE_H
#define TIMESTEPSIZE_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class TimestepSize;

template <>
InputParameters validParams<TimestepSize>();

class TimestepSize : public GeneralPostprocessor
{
public:
  TimestepSize(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  /**
   * This will return the current time step size.
   */
  virtual Real getValue() override;

protected:
  FEProblemBase & _feproblem;
};

#endif // TIMESTEPSIZE_H
