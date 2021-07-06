//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeTrussResultants.h"

class ComputePlasticTrussResultants : public ComputeTrussResultants
{
public:
  static InputParameters validParams();

  ComputePlasticTrussResultants(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();

  virtual Real computeHardeningValue(Real scalar);
  virtual Real computeHardeningDerivative(Real scalar);

  /// Base name of the material system
  const std::string _base_name;

  /// Input the yield stress
  Real _yield_stress;

  /// Input the hardening constant
  const Real _hardening_constant;

  /// Input the hardening function
  const Function * const _hardening_function;

  /// Coupled variable for the beam cross-sectional area
  const VariableValue & _area;

  /// Material stiffness vector that relates displacement strain increment to force increment
  const MaterialProperty<Real> & _material_stiffness;

  /// Current total displacement strain integrated over the cross-section in global coordinate system.
  const MaterialProperty<RealVectorValue> & _total_disp_strain;

  /// Old total displacement strain integrated over the cross-section in global coordinate system.
  const MaterialProperty<RealVectorValue> & _total_disp_strain_old;

  /// convergence tolerance
  Real _absolute_tolerance;
  Real _relative_tolerance;

  /// Current and old plastic strain
  MaterialProperty<Real> & _plastic_strain;
  const MaterialProperty<Real> & _plastic_strain_old;

  /// Current and old axial stress
  MaterialProperty<Real> & _axial_stress;
  const MaterialProperty<Real> & _axial_stress_old;

  /// Current force vector in global coordinate system
  MaterialProperty<Real> & _force;

  /// Old force vector in global coordinate system
  const MaterialProperty<Real> & _force_old;

  /// Current and old hardening variable
  MaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;

  /// maximum no. of iterations
  const unsigned int _max_its;
};
