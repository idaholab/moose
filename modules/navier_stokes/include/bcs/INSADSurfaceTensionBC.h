//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADVectorIntegratedBC.h"

/**
 * A class that imparts a surface tension on the momentum equation
 */
class INSADSurfaceTensionBC : public ADVectorIntegratedBC
{
public:
  static InputParameters validParams();

  INSADSurfaceTensionBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The recoil pressure
  const ADMaterialProperty<RealVectorValue> & _surface_term_curvature;
  const ADMaterialProperty<RealVectorValue> & _surface_term_gradient1;
  const ADMaterialProperty<RealVectorValue> & _surface_term_gradient2;
};
