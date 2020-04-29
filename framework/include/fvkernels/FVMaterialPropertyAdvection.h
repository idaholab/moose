//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVMatAdvection.h"

class FVMaterialPropertyAdvection : public FVMatAdvection
{
public:
  static InputParameters validParams();
  FVMaterialPropertyAdvection(const InputParameters & params);

protected:
  const ADReal & advQuantity() override;
  const ADReal & advQuantityNeighbor() override;

  /// The advected material property on the elem
  const ADMaterialProperty<Real> & _adv_quant_elem;

  /// The advected material property on the neighbor
  const ADMaterialProperty<Real> & _adv_quant_neighbor;
};
