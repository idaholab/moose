//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NUMLINEARITERATIONS_H
#define NUMLINEARITERATIONS_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class NumLinearIterations;

template <>
InputParameters validParams<NumLinearIterations>();

class NumLinearIterations : public GeneralPostprocessor
{
public:
  NumLinearIterations(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  virtual Real getValue() override;
};

#endif // NUMLINEARITERATIONS_H
