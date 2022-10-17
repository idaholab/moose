//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Models creation of the variable at boundaries due to dissociation of a coupled variable, e.g. B
 * -> A
 */
class DissociationFluxBC : public ADIntegratedBC
{
public:
  DissociationFluxBC(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  /// The coupled variable that is dissociating to form the variable this
  /// boundary condition is applied to
  const ADVariableValue & _v;

  /// The dissociation rate coefficient
  const Real & _Kd;
};
