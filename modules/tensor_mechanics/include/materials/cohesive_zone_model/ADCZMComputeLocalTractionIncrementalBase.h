//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADCZMComputeLocalTractionBase.h"

/**
 * AD equivalent of CZMComputeLocalTractionIncrementalBase
 */
class ADCZMComputeLocalTractionIncrementalBase : public ADCZMComputeLocalTractionBase
{
public:
  static InputParameters validParams();
  ADCZMComputeLocalTractionIncrementalBase(const InputParameters & parameters);

protected:
  void computeInterfaceTraction() override;

  /// method used to compute the traction increment
  virtual void computeInterfaceTractionIncrement() = 0;

  /// the value of the interface traction increment
  ADMaterialProperty<RealVectorValue> & _interface_traction_inc;

  /// the old interface traction value
  const MaterialProperty<RealVectorValue> & _interface_traction_old;

  /// The displacment jump  incremenet in local coordinates
  ADMaterialProperty<RealVectorValue> & _interface_displacement_jump_inc;

  /// The old interface displacment jump
  const MaterialProperty<RealVectorValue> & _interface_displacement_jump_old;
};
