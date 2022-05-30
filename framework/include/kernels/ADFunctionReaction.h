//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 *  Kernel representing the contribution of the PDE term $fu$, where $f$ is a
 *  function coefficient, and $u$ is a scalar field variable.
 */
class ADFunctionReaction : public ADKernel
{
public:
  static InputParameters validParams();

  ADFunctionReaction(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Function coefficient
  const Function & _func;
};
