//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "Material.h"

/** Copies an old material property into a new material property. */
class OldMaterialProperty : public Material
{
public:
  static InputParameters validParams();

  OldMaterialProperty(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  MaterialProperty<Real> & _property;
  const MaterialProperty<Real> & _old_property;
};
