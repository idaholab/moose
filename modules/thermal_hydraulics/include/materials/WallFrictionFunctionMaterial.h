//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "DerivativeMaterialInterfaceTHM.h"

class Function;

/**
 * Converts Darcy friction factor function into material property
 */
class WallFrictionFunctionMaterial : public DerivativeMaterialInterfaceTHM<Material>
{
public:
  WallFrictionFunctionMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  const Function & _function;

  const MaterialPropertyName _f_D_name;
  MaterialProperty<Real> & _f_D;
  MaterialProperty<Real> * const _df_D_dbeta;
  MaterialProperty<Real> & _df_D_darhoA;
  MaterialProperty<Real> & _df_D_darhouA;
  MaterialProperty<Real> & _df_D_darhoEA;

public:
  static InputParameters validParams();
};
