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

class ADConvectionHeatTransferBC : public ADIntegratedBC
{
public:
  ADConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Ambient temperature function
  const Function & _T_ambient_fn;
  /// Ambient heat transfer coefficient function
  const Function & _htc_ambient_fn;
  /// Post-processor by which to scale boundary condition
  const PostprocessorValue & _scale_pp;
  /// Function by which to scale the boundary condition
  const Function & _scale_fn;

public:
  static InputParameters validParams();
};
