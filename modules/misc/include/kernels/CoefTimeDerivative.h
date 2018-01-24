/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
