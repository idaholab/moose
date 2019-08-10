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
#include "DerivativeMaterialInterface.h"

class HasMaterialTest;

template <>
InputParameters validParams<HasMaterialTest>();

class HasMaterialTest : public DerivativeMaterialInterface<Material>
{
public:
  HasMaterialTest(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const bool _before_found;
  const bool _after_found;
  const MaterialProperty<Real> & _before;
  const MaterialProperty<Real> & _after;
};
