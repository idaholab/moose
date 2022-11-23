//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalVariablePostprocessor.h"

/**
 * Computes the "nodal" L2-norm of the coupled variable, which is
 * defined by summing the square of its value at every node and taking
 * the square root.
 */
class NodalL2Norm : public NodalVariablePostprocessor
{
public:
  static InputParameters validParams();

  NodalL2Norm(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _sum_of_squares;
};
