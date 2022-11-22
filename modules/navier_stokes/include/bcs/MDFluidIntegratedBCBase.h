//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"
#include "SinglePhaseFluidProperties.h"

/**
 * This class couples together all the variables for the 3D fluid equations to allow them to be used
 * in derived IntegratedBC classes.
 */
class MDFluidIntegratedBCBase : public IntegratedBC
{
public:
  static InputParameters validParams();

  MDFluidIntegratedBCBase(const InputParameters & parameters);
  virtual ~MDFluidIntegratedBCBase() {}

protected:
  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _pressure;
  const VariableValue & _temperature;

  const MaterialProperty<Real> & _rho;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
  const VariableGradient & _grad_pressure;
  const VariableGradient & _grad_temperature;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _pressure_var_number;
  unsigned _temperature_var_number;

  bool _has_porosity;
  const VariableValue & _porosity;

  // Helper function for mapping Moose variable numberings into
  // the "canonical" numbering for the porous medium equations.
  unsigned map_var_number(unsigned var);
  const SinglePhaseFluidProperties & _eos;
};
