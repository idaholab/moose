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

// Forward Declarations

/// The couple, SplitCHBase and SplitCHWRes, splits the CH equation by replacing chemical potential with 'w'.
class SplitCHBase : public Kernel
{
public:
  static InputParameters validParams();

  SplitCHBase(const InputParameters & parameters);

protected:
  enum PFFunctionType
  {
    Jacobian,
    Residual
  };
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeDFDC(PFFunctionType type);
  virtual Real computeDEDC(PFFunctionType type);
};
