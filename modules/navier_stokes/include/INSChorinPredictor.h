#ifndef INSCHORINPREDICTOR_H
#define INSCHORINPREDICTOR_H

#include "Kernel.h"

// Forward Declarations
class INSChorinPredictor;

template<>
InputParameters validParams<INSChorinPredictor>();

/**
 * This class computes the "Chorin" Predictor equation in fully-discrete
 * (both time and space) form.
 */
class INSChorinPredictor : public Kernel
{
public:
  INSChorinPredictor(const std::string & name, InputParameters parameters);

  virtual ~INSChorinPredictor(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Velocity (current time)
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;

  // Velocity (old time)
  VariableValue& _u_vel_old;
  VariableValue& _v_vel_old;
  VariableValue& _w_vel_old;

  // Velocity Gradients
  VariableGradient& _grad_u_vel;
  VariableGradient& _grad_v_vel;
  VariableGradient& _grad_w_vel;

  // Old Velocity Gradients
  VariableGradient& _grad_u_vel_old;
  VariableGradient& _grad_v_vel_old;
  VariableGradient& _grad_w_vel_old;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;

  // Material properties
  Real _mu;
  Real _rho;

  // Parameters
  unsigned _component;

  // Flag which controls the time level the convection and diffusion
  // terms are evaluated at.
  bool _chorin_implicit;
};


#endif // INSCHORINPREDICTOR_H
