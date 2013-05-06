#ifndef INSMASS_H
#define INSMASS_H

#include "Kernel.h"

// Forward Declarations
class INSMass;

template<>
InputParameters validParams<INSMass>();

/**
 * This class computes the mass equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
class INSMass : public Kernel
{
public:
  INSMass(const std::string & name, InputParameters parameters);

  virtual ~INSMass(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Coupled Gradients
  VariableGradient& _grad_u_vel;
  VariableGradient& _grad_v_vel;
  VariableGradient& _grad_w_vel;

  // Variable numberings
  unsigned _u_vel_var_number;
  unsigned _v_vel_var_number;
  unsigned _w_vel_var_number;
  unsigned _p_var_number;
};


#endif // INSMASS_H
