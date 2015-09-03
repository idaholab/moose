/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef COEFCOUPLEDTIMEDERIVATIVE_H
#define COEFCOUPLEDTIMEDERIVATIVE_H

#include "CoupledTimeDerivative.h"

// Forward Declaration
class CoefCoupledTimeDerivative;

template<>
InputParameters validParams<CoefCoupledTimeDerivative>();

/**
 * This calculates the time derivative for a coupled variable multiplied by a scalar coedfficient
 **/
class CoefCoupledTimeDerivative : public CoupledTimeDerivative
{
public:
  CoefCoupledTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  Real _coef;
};

#endif //COEFCOUPLEDTIMEDERIVATIVE_H
