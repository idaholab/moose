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

class NodalPatchRecoveryBase;

class NodalPatchRecoveryAux : public AuxKernel
{
public:
  static InputParameters validParams();

  NodalPatchRecoveryAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  const NodalPatchRecoveryBase & _npr;
};
