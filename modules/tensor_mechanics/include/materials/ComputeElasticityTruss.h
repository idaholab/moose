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

/**
 * ComputeElasticityTruss computes the equivalent of the elasticity tensor for the truss element,
 * which are vectors of material translational and flexural stiffness
 */
class ComputeElasticityTruss : public Material
{
public:
  static InputParameters validParams();

  ComputeElasticityTruss(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Material stiffness vector that relates displacement strain increments to force increments
  MaterialProperty<RealVectorValue> & _material_stiffness;

  // /// Material flexure vector that relates rotational strain increments to moment increments
  // MaterialProperty<RealVectorValue> & _material_flexure;

  /// Prefactor function used to modify (i.e., multiply) the material stiffness and flexure vectors
  const Function * const _prefactor_function;

  /// Young's modulus of the truss material
  const VariableValue & _youngs_modulus;

  // /// Poisson's ratio of the truss material
  // const VariableValue & _poissons_ratio;

  // /// Shear coefficient for the truss cross-section
  // const VariableValue & _shear_coefficient;
};
