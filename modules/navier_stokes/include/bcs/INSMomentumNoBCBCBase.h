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

// Forward Declarations

/**
 * Base class for the "No BC" boundary condition.  Subclasses will
 * implement the computeQpXYZ() functions differently based on whether the
 * "traction" or "Laplacian" form of the viscous stress tensor is
 * used.  The idea behind this is discussed by Griffiths, Papanastiou,
 * and others.  Note that this BC, unlike the natural BC, is
 * insufficient to set the value of the pressure in outflow problems,
 * and therefore you will need to implement a pressure pin or similar
 * approach for constraining the null space of constant pressures.
 */
class INSMomentumNoBCBCBase : public IntegratedBC
{
public:
  static InputParameters validParams();

  INSMomentumNoBCBCBase(const InputParameters & parameters);

  virtual ~INSMomentumNoBCBCBase() {}

protected:
  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _p;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _p_var_number;

  RealVectorValue _gravity;
  unsigned _component;
  bool _integrate_p_by_parts;

  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _rho;
};
