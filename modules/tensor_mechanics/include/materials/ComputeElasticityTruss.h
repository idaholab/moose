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
  MaterialProperty<Real> & _material_stiffness;

  /// Young's modulus of the truss material
  const VariableValue & _youngs_modulus;
};
