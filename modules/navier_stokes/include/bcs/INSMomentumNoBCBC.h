/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSMOMENTUMNOBCBC_H
#define INSMOMENTUMNOBCBC_H

#include "IntegratedBC.h"

// Forward Declarations
class INSMomentumNoBCBC;

template<>
InputParameters validParams<INSMomentumNoBCBC>();

/**
 * This class implements the "No BC" boundary condition
 * discussed by Griffiths, Papanastiou, and others.
 */
class INSMomentumNoBCBC : public IntegratedBC
{
public:
  INSMomentumNoBCBC(const std::string & name, InputParameters parameters);

  virtual ~INSMomentumNoBCBC(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled variables
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;
  VariableValue& _p;

  // Gradients
  VariableGradient& _grad_u_vel;
  VariableGradient& _grad_v_vel;
  VariableGradient& _grad_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _p_var_number;

  Real _mu;
  Real _rho;
  RealVectorValue _gravity;

  unsigned _component;
};


#endif // INSMOMENTUMNOBCBC_H
