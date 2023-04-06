//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceMaterial.h"

/**
 * AD equivalent of CZMComputeLocalTractionBase
 */
class ADCZMComputeLocalTractionBase : public InterfaceMaterial
{
public:
  static InputParameters validParams();
  ADCZMComputeLocalTractionBase(const InputParameters & parameters);

protected:
  void initQpStatefulProperties() override;
  void computeQpProperties() override;

  /// Compute the local traction
  virtual void computeInterfaceTraction() = 0;

  /// Base name of the material system
  const std::string _base_name;

  /// the value of the traction in local coordinates
  ADMaterialProperty<RealVectorValue> & _interface_traction;

  /// The displacment jump in local coordaintes
  const ADMaterialProperty<RealVectorValue> & _interface_displacement_jump;
};
