#ifndef ONEDMOMENTUMFRICTION_H
#define ONEDMOMENTUMFRICTION_H

#include "Kernel.h"


// Forward Declarations
class OneDMomentumFriction;

template<>
InputParameters validParams<OneDMomentumFriction>();

/**
 * The spatial part of the 1D momentum conservation for the variable-area Euler equations.
 */
class OneDMomentumFriction : public Kernel
{
public:

  OneDMomentumFriction(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Coupled variables
  VariableValue & _u_vel;
  VariableValue & _rhouA;
  VariableValue & _hydraulic_diameter;

  // IDs of coupled variables (for computing Jacobians)
  unsigned _rhoA_var_number;

  // Material properties
  MaterialProperty<Real> & _friction;
};

#endif
