//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxKernel.h"

class FVMatAdvection : public FVFluxKernel
{
public:
  static InputParameters validParams();
  FVMatAdvection(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const ADMaterialProperty<RealVectorValue> & _vel_elem;
  const ADMaterialProperty<RealVectorValue> & _vel_neighbor;

  /// The advected quantity on the elem
  const MooseArray<ADReal> * const _adv_quant_elem;

  /// The advected quantity on the neighbor
  const MooseArray<ADReal> * const _adv_quant_neighbor;
};
