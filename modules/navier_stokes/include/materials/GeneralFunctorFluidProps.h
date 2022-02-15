//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"
#include "DerivativeMaterialPropertyNameInterface.h"
#include "Function.h"

class SinglePhaseFluidProperties;
class Function;

/**
 * Computes fluid properties in (P, T) formulation using functor material properties
 */
class GeneralFunctorFluidProps : public FunctorMaterial, DerivativeMaterialPropertyNameInterface
{
public:
  static InputParameters validParams();
  GeneralFunctorFluidProps(const InputParameters & parameters);

protected:
  const SinglePhaseFluidProperties & _fluid;

  /// Porosity
  const Moose::Functor<ADReal> & _eps;

  /**
   * Characteristic length $d$ used in computing the Reynolds number
   * $Re=\rho_fVd/\mu_f$.
   */
  const Real _d;

  /// variables
  const Moose::Functor<ADReal> & _pressure;
  const Moose::Functor<ADReal> & _T_fluid;
  const Moose::Functor<ADReal> & _speed;

  /// Density as a functor, which could be from the variable set or the property
  const Moose::Functor<ADReal> & _rho;

  /// Function to ramp down the viscosity, useful for relaxation transient
  const Function & _mu_rampdown;

  using DerivativeMaterialPropertyNameInterface::derivativePropertyNameFirst;
  using UserObjectInterface::getUserObject;
};
