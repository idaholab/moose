/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
