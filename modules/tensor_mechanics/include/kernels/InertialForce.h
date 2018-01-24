//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
