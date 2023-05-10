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
 * Convection BC from an external application
 */
class ADExternalAppConvectionHeatTransferBC : public ADIntegratedBC
{
public:
  ADExternalAppConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Temperature from external application
  const ADVariableValue & _T_ext;
  /// Heat transfer coefficient from external application
  const ADVariableValue & _htc_ext;
  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;
  /// Function by which to scale the boundary condition
  const Function & _scale_fn;

public:
  static InputParameters validParams();
};
