//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALSUM_H
#define NODALSUM_H

#include "NodalVariablePostprocessor.h"

// Forward Declarations
class NodalSum;

template <>
InputParameters validParams<NodalSum>();

/**
 * Computes a sum of the nodal values of the coupled variable.
 */
class NodalSum : public NodalVariablePostprocessor
{
public:
  NodalSum(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;

  void threadJoin(const UserObject & y) override;

protected:
  Real _sum;
};

#endif // NODALSUM_H
