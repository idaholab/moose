//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * Test material that declares a RealVectorValue stateful property named "diffusivity"
 * for testing type mismatch detection in StatefulMaterialPropertyImporter.
 */
class VectorStatefulMaterial : public Material
{
public:
  static InputParameters validParams();

  VectorStatefulMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  MaterialProperty<RealVectorValue> & _diffusivity;
  const MaterialProperty<RealVectorValue> & _diffusivity_old;
};
