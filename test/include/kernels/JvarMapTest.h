//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "JvarMapInterface.h"

class JvarMapTest : public JvarMapKernelInterface<Kernel>
{
public:
  static InputParameters validParams();

  JvarMapTest(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() { return 0; }

  void testMap(const std::string & name, const JvarMap & map);

  const JvarMap & _v0_map;
  const JvarMap & _v1_map;
};
