//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COEFTIMEDERIVATIVE_H
#define COEFTIMEDERIVATIVE_H

#include "TimeDerivative.h"

class CoefTimeDerivative;

template <>
InputParameters validParams<CoefTimeDerivative>();

/**
 * Time derivative term multiplied by a coefficient
 */
class CoefTimeDerivative : public TimeDerivative
{
public:
  CoefTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// The coefficient the time derivative is multiplied with
  Real _coef;
};

#endif // COEFTIMEDERIVATIVE_H
