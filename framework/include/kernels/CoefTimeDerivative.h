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

// Forward Declarations
class CoefTimeDerivative;

/**
 * validParams returns the parameters that this Kernel accepts / needs
 * The actual body of the function MUST be in the .C file.
 */
template <>
InputParameters validParams<CoefTimeDerivative>();

class CoefTimeDerivative : public TimeDerivative
{
public:
  CoefTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /**
   * This MooseArray will hold the reference we need to our
   * material property from the Material class
   */
  Real _coef;
};
#endif // COEFTIMEDERIVATIVE_H
