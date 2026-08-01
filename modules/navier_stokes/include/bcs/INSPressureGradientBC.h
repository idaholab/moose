//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 * Adds the exterior-face pressure contribution for a momentum equation whose pressure gradient is
 * integrated by parts.
 */
class INSPressureGradientBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  INSPressureGradientBC(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual ADReal computeQpResidual() override;

  /// Pressure evaluated on the exterior face.
  const ADVariableValue & _pressure;

  /// Momentum component to which this pressure contribution is added.
  const unsigned _component;
};
