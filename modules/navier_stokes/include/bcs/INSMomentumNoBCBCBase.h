/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMNOBCBCBASE_H
#define INSMOMENTUMNOBCBCBASE_H

#include "IntegratedBC.h"

// Forward Declarations
class INSMomentumNoBCBCBase;

template <>
InputParameters validParams<INSMomentumNoBCBCBase>();

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

  Real _mu;
  Real _rho;
  RealVectorValue _gravity;

  unsigned _component;
  bool _integrate_p_by_parts;
};

#endif
