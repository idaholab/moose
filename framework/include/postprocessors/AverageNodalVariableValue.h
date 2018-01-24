//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AVERAGENODALVARIABLEVALUE_H
#define AVERAGENODALVARIABLEVALUE_H

#include "NodalVariablePostprocessor.h"

// Forward Declarations
class AverageNodalVariableValue;

template <>
InputParameters validParams<AverageNodalVariableValue>();

class AverageNodalVariableValue : public NodalVariablePostprocessor
{
public:
  AverageNodalVariableValue(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual Real getValue() override;

  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _avg;
  unsigned int _n;
};

#endif // AVERAGENODALVARIABLEVALUE_H
