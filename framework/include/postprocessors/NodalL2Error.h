//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALL2ERROR_H
#define NODALL2ERROR_H

#include "NodalVariablePostprocessor.h"

class NodalL2Error;

template <>
InputParameters validParams<NodalL2Error>();

/**
 *
 */
class NodalL2Error : public NodalVariablePostprocessor
{
public:
  NodalL2Error(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _integral_value;
  Function & _func;
};

#endif /* NODALL2ERROR_H */
