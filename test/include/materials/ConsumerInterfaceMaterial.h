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

class ConsumerInterfaceMaterial : public InterfaceMaterial
{
public:
  static InputParameters validParams();

  ConsumerInterfaceMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const ADMaterialProperty<Real> & _prop_consumed;
  ADMaterialProperty<Real> & _prop_produced;
};
