//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSTEMPERATURETIMEDERIVATIVE_H
#define INSTEMPERATURETIMEDERIVATIVE_H

#include "TimeDerivative.h"

// Forward Declarations
class INSTemperatureTimeDerivative;

template <>
InputParameters validParams<INSTemperatureTimeDerivative>();

/**
 * This class computes the time derivative for the incompressible
 * Navier-Stokes momentum equation.  Could instead use CoefTimeDerivative
 * for this.
 */
class INSTemperatureTimeDerivative : public TimeDerivative
{
public:
  INSTemperatureTimeDerivative(const InputParameters & parameters);

  virtual ~INSTemperatureTimeDerivative() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Parameters
  const MaterialProperty<Real> & _rho;
  const MaterialProperty<Real> & _cp;
};

#endif // INSTEMPERATURETIMEDERIVATIVE_H
