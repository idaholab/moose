//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMaterial.h"

// Forward Declarations
template <ComputeStage>
class ADStatefulMaterial;

declareADValidParams(ADStatefulMaterial);

/**
 * Stateful material class that defines a few properties.
 */
template <ComputeStage compute_stage>
class ADStatefulMaterial : public ADMaterial<compute_stage>
{
public:
  ADStatefulMaterial(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

private:
  Real _initial_diffusivity;

  /**
   * Create two MooseArray Refs to hold the current
   * and previous material properties respectively
   */
  ADMaterialProperty(Real) & _diffusivity;
  const MaterialProperty<Real> & _diffusivity_old;
  const ADVariableValue & _u;

  usingMaterialMembers;
};

