#ifndef ONEDMOMENTUMFRICTION_H
#define ONEDMOMENTUMFRICTION_H

#include "Kernel.h"

// Forward Declarations
class OneDMomentumFriction;

template<>
InputParameters validParams<OneDMomentumFriction>();

/**
 * Contribution due to wall friction
 */
class OneDMomentumFriction : public Kernel
{
public:
  OneDMomentumFriction(const std::string & name, InputParameters parameters);
  virtual ~OneDMomentumFriction();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  VariableValue & _u_vel;
  VariableValue & _rhouA;
  VariableValue & _hydraulic_diameter;

  unsigned int _rhoA_var_number;

  const MaterialProperty<Real> & _friction;
};

#endif
