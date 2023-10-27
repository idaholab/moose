//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "INSADTauMaterial.h"
#include "INSAD3Eqn.h"

class INSADStabilized3Eqn : public INSADTauMaterialTempl<INSAD3Eqn>
{
public:
  static InputParameters validParams();

  INSADStabilized3Eqn(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const ADVariableSecond & _second_temperature;

  const ADMaterialProperty<Real> & _k;
  const ADMaterialProperty<RealVectorValue> * const _grad_k;

  ADMaterialProperty<Real> & _tau_energy;
  ADMaterialProperty<Real> & _temperature_strong_residual;
};
