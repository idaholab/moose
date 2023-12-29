//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearElasticTruss.h"

class PlasticTruss : public LinearElasticTruss
{
public:
  static InputParameters validParams();

  PlasticTruss(const InputParameters & parameters);

protected:
  virtual void computeQpStrain();
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  virtual Real computeHardeningValue(Real scalar);
  virtual Real computeHardeningDerivative(Real scalar);

  //  yield stress and hardening property input
  Real _yield_stress;
  const Real _hardening_constant;
  const Function * const _hardening_function;

  /// convergence tolerance
  Real _absolute_tolerance;
  Real _relative_tolerance;

  const MaterialProperty<Real> & _total_stretch_old;
  MaterialProperty<Real> & _plastic_strain;
  const MaterialProperty<Real> & _plastic_strain_old;
  const MaterialProperty<Real> & _stress_old;

  MaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;

  /// maximum no. of iterations
  const unsigned int _max_its;
};
