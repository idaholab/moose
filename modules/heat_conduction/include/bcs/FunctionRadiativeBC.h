//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadiativeHeatFluxBCBase.h"

#include "Function.h"

/**
 * Boundary condition for radiative heat exchange with a cylinder, the outer
 * surface of the domain is assumed to be cylindrical as well
 */
class FunctionRadiativeBC : public RadiativeHeatFluxBCBase
{
public:
  static InputParameters validParams();

  FunctionRadiativeBC(const InputParameters & parameters);

protected:
  virtual Real coefficient() const override;

  /// emissivity function
  const Function & _emissivity;
};
