//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FXBoundaryBaseUserObject.h"

/**
 * This boundary FX evaluator calculates the flux
 */
class FXBoundaryFluxUserObject final : public FXBoundaryBaseUserObject
{
public:
  static InputParameters validParams();

  FXBoundaryFluxUserObject(const InputParameters & parameters);

protected:
  // Override from SideIntegralVariableUserObject
  virtual Real computeQpIntegral() final;

  /// Name of the diffusivity property in the local material
  const std::string _diffusivity_name;

  /// Value of the diffusivity
  const MaterialProperty<Real> & _diffusivity;
};
