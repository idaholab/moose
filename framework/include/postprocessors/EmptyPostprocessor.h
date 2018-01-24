//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EMPTYPOSTPROCESSOR_H
#define EMPTYPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class EmptyPostprocessor;

template <>
InputParameters validParams<EmptyPostprocessor>();

class EmptyPostprocessor : public GeneralPostprocessor
{
public:
  EmptyPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}
  virtual Real getValue() override { return 0; }
};

#endif
