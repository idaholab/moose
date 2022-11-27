//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "Material.h"
#include "SinglePhaseFluidProperties.h"

/* Fluid materials for 3D fluid model */
class MDFluidMaterial : public Material
{
public:
  static InputParameters validParams();

  MDFluidMaterial(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual void computeQpProperties() override;
  const unsigned int _mesh_dimension;

  // Coupled values needed for stabilization...
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _temperature;
  const VariableValue & _pressure;

  const VariableGradient & _grad_u;
  const VariableGradient & _grad_v;
  const VariableGradient & _grad_w;

  // Material properties
  MaterialProperty<RealTensorValue> & _viscous_stress_tensor;
  MaterialProperty<Real> & _dynamic_viscosity;
  MaterialProperty<Real> & _turbulence_viscosity;
  MaterialProperty<Real> & _k;
  MaterialProperty<Real> & _k_turbulence;
  MaterialProperty<Real> & _k_elem;
  MaterialProperty<Real> & _cp;
  MaterialProperty<Real> & _rho;
  MaterialProperty<Real> & _hsupg;
  MaterialProperty<Real> & _tauc;
  MaterialProperty<Real> & _taum;
  MaterialProperty<Real> & _taue;

  bool _compute_visc_turbulenc;
  bool _has_turb_visc_auxvar;
  const Function * _mixing_length;
  const VariableValue & _turb_visc_auxvar;

  bool _has_scale_vel;
  Real _scaling_velocity; /// scaling velocity
  const SinglePhaseFluidProperties & _eos;

  bool _bSteady;
  Real _u_elem;
  Real _v_elem;
  Real _w_elem;
  Real _vel_mag;
  Real _k_elem_val;

private:
  void compute_fluid_properties();
  void compute_hsupg();
  void compute_tau();
};
