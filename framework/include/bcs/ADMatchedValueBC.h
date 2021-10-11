//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalBC.h"

/**
 * Implements a simple coupled boundary condition where u=v on the boundary.
 */
class ADMatchedValueBC : public ADNodalBC
{
public:
  static InputParameters validParams();

  ADMatchedValueBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableValue & _v;

private:
  /// Coefficient for primary variable
  const Real _u_coeff;
  /// Coefficient for coupled variable
  const Real _v_coeff;
};
