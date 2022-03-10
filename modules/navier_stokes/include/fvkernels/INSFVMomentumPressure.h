//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSFVElementalKernel.h"
#include "INSFVMomentumResidualObject.h"

class INSFVMomentumPressure : public INSFVElementalKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumPressure(const InputParameters & params);

  using INSFVElementalKernel::gatherRCData;
  void gatherRCData(const Elem &) override;

protected:
  /// The pressure variable
  const MooseVariableFVReal * const _p_var;
};
