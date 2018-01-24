//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALMAXVALUE_H
#define NODALMAXVALUE_H

#include "NodalVariablePostprocessor.h"

// Forward Declarations
class NodalMaxValue;

template <>
InputParameters validParams<NodalMaxValue>();

/**
 * This class computes a maximum (over all the nodal values) of the
 * coupled variable.
 */
class NodalMaxValue : public NodalVariablePostprocessor
{
public:
  NodalMaxValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _value;
};

#endif
