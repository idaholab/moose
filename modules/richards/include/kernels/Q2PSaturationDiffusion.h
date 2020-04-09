//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "RichardsDensity.h"
#include "RichardsRelPerm.h"
#include "Material.h"
#include "Function.h"

// Forward Declarations

/**
 * Diffusive Kernel that models nonzero capillary pressure in Q2P models
 * The Variable of this Kernel should be the saturation
 */
class Q2PSaturationDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  Q2PSaturationDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// water density
  const RichardsDensity & _density;

  /// water relative permeability
  const RichardsRelPerm & _relperm;

  /// porepressure at the quadpoints
  const VariableValue & _pp;

  /// variable number of the porepressure variable
  unsigned int _pp_var_num;

  /// fluid viscosity
  Real _viscosity;

  /// permeability
  const MaterialProperty<RealTensorValue> & _permeability;

  Real _diffusivity;
};
