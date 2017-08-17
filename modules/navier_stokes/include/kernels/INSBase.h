/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INSBASE_H
#define INSBASE_H

#include "Kernel.h"

// Forward Declarations
class INSBase;

template <>
InputParameters validParams<INSBase>();

/**
 * This class computes strong and weak components of the INS governing
 * equations.  These terms can then be assembled in child classes
 */
class INSBase : public Kernel
{
public:
  INSBase(const InputParameters & parameters);

  virtual ~INSBase() {}

protected:
  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian() = 0;
  virtual Real computeQpOffDiagJacobian(unsigned jvar) = 0;
  virtual RealVectorValue computeStrongConvectiveTerm();
  virtual RealVectorValue computeStrongViscousTerm();
  virtual RealVectorValue computeStrongPressureTerm();
  virtual RealVectorValue computeStrongGravityTerm();

  /// second derivatives of the shape function
  const VariablePhiSecond & _second_phi;

  // Coupled variables
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
  const VariableValue & _p;

  // Gradients
  const VariableGradient & _grad_u_vel;
  const VariableGradient & _grad_v_vel;
  const VariableGradient & _grad_w_vel;
  const VariableGradient & _grad_p;

  // Seconds
  const VariableSecond & _second_u_vel;
  const VariableSecond & _second_v_vel;
  const VariableSecond & _second_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _p_var_number;

  // Parameters
  RealVectorValue _gravity;

  // Material properties
  const MaterialProperty<Real> & _mu;
  const MaterialProperty<Real> & _rho;
};

#endif
