/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

  // Velocity
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;

  // Old Velocity
  VariableValue& _u_vel_old;
  VariableValue& _v_vel_old;
  VariableValue& _w_vel_old;

  // Star Velocity
  VariableValue& _u_vel_star;
  VariableValue& _v_vel_star;
  VariableValue& _w_vel_star;

  // Velocity Gradients
  VariableGradient& _grad_u_vel;
  VariableGradient& _grad_v_vel;
  VariableGradient& _grad_w_vel;

  // Old Velocity Gradients
  VariableGradient& _grad_u_vel_old;
  VariableGradient& _grad_v_vel_old;
  VariableGradient& _grad_w_vel_old;

  // Star Velocity Gradients
  VariableGradient& _grad_u_vel_star;
  VariableGradient& _grad_v_vel_star;
  VariableGradient& _grad_w_vel_star;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;

  // Star velocity numbers
  unsigned _u_vel_star_var_number;
  unsigned _v_vel_star_var_number;
  unsigned _w_vel_star_var_number;

  // Material properties
  Real _mu;
  Real _rho;

  // Parameters
  unsigned _component;

  // This is the string that's actually read in from file and used to set the
  // MooseEnum, below.  The options are:
  // OLD  - Use velocity from the previous timestep, leads to explicit method
  // NEW  - Use velocity from current timestep, this may not be an actual method
  // STAR - Use the "star" velocity.  According to Donea's book, this is the
  //        right way to get an implicit method...
  std::string _predictor_type;

  // An enumeration defining which velocity vector is used on the rhs
  // of the Chorin predictor.
  MooseEnum _predictor_enum;

  // A C++ enumeration corresponding to the predictor enumeration.
  enum PredictorType
  {
    OLD = 0,
    NEW = 1,
    STAR = 2
  };
};


#endif // INSCHORINPREDICTOR_H
