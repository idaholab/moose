//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Test3EqnRDGObjectBase.h"

/**
 * Base class for testing consistency of a numerical flux for the 3-equation model.
 */
class ConsistencyTest3EqnRDGFluxBase : public Test3EqnRDGObjectBase
{
public:
  ConsistencyTest3EqnRDGFluxBase() : Test3EqnRDGObjectBase() {}

protected:
  virtual void test() override;
};
