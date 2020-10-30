//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

// Forward Declarations
class Function;

class FunctionArrayAux : public ArrayAuxKernel
{
public:
  static InputParameters validParams();

  FunctionArrayAux(const InputParameters & parameters);

protected:
  virtual RealEigenVector computeValue() override;

  /// Functions being used to compute the value of this kernel
  std::vector<const Function *> _funcs;
};
