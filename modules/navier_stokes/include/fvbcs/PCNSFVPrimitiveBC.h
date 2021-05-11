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
#include "MooseVariableFV.h"

class SinglePhaseFluidProperties;

class PCNSFVPrimitiveBC : public FVFluxBC
{
public:
  static InputParameters validParams();
  PCNSFVPrimitiveBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;
  void computeAValues();

  const SinglePhaseFluidProperties & _fluid;
  const unsigned int _dim;
  const MaterialProperty<Real> & _eps_elem;
  const MaterialProperty<Real> & _eps_neighbor;
  const MooseEnum _eqn;
  const unsigned int _index;

  const bool _svel_provided;
  const bool _pressure_provided;
  const bool _T_fluid_provided;
  const Function * const _svel_function;
  const Function * const _pressure_function;
  const Function * const _T_fluid_function;
  const bool _velocity_function_includes_rho;
  const MooseVariableFVReal * const _pressure_var;
  const MooseVariableFVReal * const _T_fluid_var;
  const MooseVariableFVReal * const _sup_vel_x_var;
  const MooseVariableFVReal * const _sup_vel_y_var;
  const MooseVariableFVReal * const _sup_vel_z_var;
};
