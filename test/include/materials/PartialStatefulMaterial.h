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
 * Material with two stateful properties: "diffusivity" and "conductivity".
 * Used to test partial imports via StatefulMaterialPropertyImporter — only
 * "diffusivity" is present in the import file; "conductivity" is not.
 * After import, conductivity_old must equal initial_conductivity (proving
 * that initStatefulProperties() ran correctly for the non-imported property),
 * while diffusivity_old must equal the remapped imported value (not initial_diffusivity).
 */
class PartialStatefulMaterial : public Material
{
public:
  static InputParameters validParams();
  PartialStatefulMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

private:
  const Real _initial_diffusivity;
  const Real _initial_conductivity;

  MaterialProperty<Real> & _diffusivity;
  const MaterialProperty<Real> & _diffusivity_old;

  MaterialProperty<Real> & _conductivity;
  const MaterialProperty<Real> & _conductivity_old;
};
