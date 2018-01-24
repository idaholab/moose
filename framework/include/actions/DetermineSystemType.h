//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DETERMINESYSTEMTYPE_H
#define DETERMINESYSTEMTYPE_H

#include "MooseObjectAction.h"

class DetermineSystemType;

template <>
InputParameters validParams<DetermineSystemType>();

class DetermineSystemType : public MooseObjectAction
{
public:
  DetermineSystemType(InputParameters parameters);

  virtual void act() override;
};

#endif /* DETERMINESYSTEMTYPE_H */
