//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVFluxBC.h"

class SinglePhaseFluidProperties;
class MfrPostprocessor;

class PCNSFVStrongBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  PCNSFVStrongBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  const SinglePhaseFluidProperties & _fluid;
  const unsigned int _dim;
  const ADMaterialProperty<Real> & _sup_vel_x_elem;
  const ADMaterialProperty<Real> & _sup_vel_x_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_x_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_x_neighbor;
  const ADMaterialProperty<Real> & _sup_vel_y_elem;
  const ADMaterialProperty<Real> & _sup_vel_y_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_y_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_y_neighbor;
  const ADMaterialProperty<Real> & _sup_vel_z_elem;
  const ADMaterialProperty<Real> & _sup_vel_z_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_z_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_sup_vel_z_neighbor;
  const ADMaterialProperty<Real> & _T_fluid_elem;
  const ADMaterialProperty<Real> & _T_fluid_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_T_fluid_neighbor;
  const ADMaterialProperty<Real> & _pressure_elem;
  const ADMaterialProperty<Real> & _pressure_neighbor;
  const ADMaterialProperty<RealVectorValue> & _grad_pressure_elem;
  const ADMaterialProperty<RealVectorValue> & _grad_pressure_neighbor;
  const MaterialProperty<Real> & _eps_elem;
  const MaterialProperty<Real> & _eps_neighbor;
  const MooseEnum _eqn;
  const unsigned int _index;

  const bool _sup_vel_provided;
  const bool _pressure_provided;
  const bool _T_fluid_provided;
  const Function * const _sup_vel_function;
  const Function * const _pressure_function;
  const Function * const _T_fluid_function;
  const ADVariableValue & _scalar_elem;
  const ADVariableValue & _scalar_neighbor;
  const ADVariableGradient * const _grad_scalar_elem;
  const ADVariableGradient * const _grad_scalar_neighbor;
  const bool _scalar_function_provided;
  const Function * const _scalar_function;
  const bool _velocity_function_includes_rho;

  MfrPostprocessor * const _mfr_pp;
};
