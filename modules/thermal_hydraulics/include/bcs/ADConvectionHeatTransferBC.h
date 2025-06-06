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

class ADConvectionHeatTransferBC : public ADIntegratedBC
{
public:
  ADConvectionHeatTransferBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Ambient temperature functor
  const Moose::Functor<ADReal> & _T_ambient;
  /// Ambient heat transfer coefficient functor
  const Moose::Functor<ADReal> & _htc_ambient;
  /// Functor by which to scale the boundary condition
  const Moose::Functor<ADReal> & _scale;

public:
  static InputParameters validParams();
};
