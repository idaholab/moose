/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INERTIALFORCE_H
#define INERTIALFORCE_H

#include "Kernel.h"
#include "Material.h"

// Forward Declarations
class InertialForce;

template <>
InputParameters validParams<InertialForce>();

class InertialForce : public Kernel
{
public:
  InertialForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  const MaterialProperty<Real> & _density;
  const VariableValue & _u_old;
  const VariableValue & _vel_old;
  const VariableValue & _accel_old;
  const Real _beta;
  const Real _gamma;
  const MaterialProperty<Real> & _eta;
  const Real _alpha;
};

#endif // INERTIALFORCE_H
