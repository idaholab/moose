//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADInterfaceKernel.h"

/// AD equivalent of CZMInterfaceKernelBase
class ADCZMInterfaceKernelBase : public ADInterfaceKernel
{
public:
  static InputParameters validParams();
  ADCZMInterfaceKernelBase(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::DGResidualType type) override;

  /// Base name of the material system that this kernel applies to
  const std::string _base_name;

  /// the displacement component this kernel is operating on (0=x, 1=y, 2 =z)
  const unsigned int _component;

  /// number of displacement components
  const unsigned int _ndisp;

  // the traction
  const ADMaterialProperty<RealVectorValue> & _traction_global;
};
