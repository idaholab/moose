//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalPatchRecoveryAuxBase.h"

/// Forward declare user object
class NodalPatchRecoveryBase;

class NodalPatchRecoveryAux : public NodalPatchRecoveryAuxBase
{
public:
  static InputParameters validParams();

  NodalPatchRecoveryAux(const InputParameters & parameters);

protected:
  virtual Real nodalPatchRecovery() override;

private:
  const NodalPatchRecoveryBase & _npr;
};
