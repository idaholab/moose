//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COEFCOUPLEDTIMEDERIVATIVE_H
#define COEFCOUPLEDTIMEDERIVATIVE_H

#include "CoupledTimeDerivative.h"

// Forward Declaration
class CoefCoupledTimeDerivative;

template <>
InputParameters validParams<CoefCoupledTimeDerivative>();

/**
 * This calculates the time derivative for a coupled variable multiplied by a
 * scalar coefficient
 */
class CoefCoupledTimeDerivative : public CoupledTimeDerivative
{
public:
  CoefCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const Real _coef;
};

#endif // COEFCOUPLEDTIMEDERIVATIVE_H
