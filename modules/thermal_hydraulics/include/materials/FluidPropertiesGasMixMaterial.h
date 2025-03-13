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

class VaporMixtureFluidProperties;

/**
 * Computes various fluid properties for FlowModelGasMix.
 */
class FluidPropertiesGasMixMaterial : public Material
{
public:
  static InputParameters validParams();

  FluidPropertiesGasMixMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const ADVariableValue & _A;
  const ADVariableValue & _xirhoA;
  const ADVariableValue & _rhoA;
  const ADVariableValue & _rhouA;
  const ADVariableValue & _rhoEA;

  ADMaterialProperty<Real> & _xi;
  ADMaterialProperty<Real> & _rho;
  ADMaterialProperty<Real> & _v;
  ADMaterialProperty<Real> & _vel;
  ADMaterialProperty<Real> & _e;
  ADMaterialProperty<Real> & _p;
  ADMaterialProperty<Real> & _T;
  ADMaterialProperty<Real> & _h;
  ADMaterialProperty<Real> & _H;
  ADMaterialProperty<Real> & _c;
  ADMaterialProperty<Real> & _cp;
  ADMaterialProperty<Real> & _cv;
  ADMaterialProperty<Real> & _k;
  ADMaterialProperty<Real> & _mu;

  const VaporMixtureFluidProperties & _fp;
};
