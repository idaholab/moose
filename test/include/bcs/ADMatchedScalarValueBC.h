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
 * Boundary condition of a Dirichlet type
 *
 * Sets the values of a nodal variable at nodes to values specified by a function
 */
class ADMatchedScalarValueBC : public ADNodalBC
{
public:
  static InputParameters validParams();

  ADMatchedScalarValueBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableValue & _v;
};
