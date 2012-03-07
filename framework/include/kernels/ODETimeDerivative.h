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

#ifndef ODETIMEDERIVATIVE_H
#define ODETIMEDERIVATIVE_H

#include "ODEKernel.h"

// Forward Declaration
class ODETimeDerivative;

template<>
InputParameters validParams<ODETimeDerivative>();

class ODETimeDerivative : public ODEKernel
{
public:
  ODETimeDerivative(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

};

#endif //ODETIMEDERIVATIVE_H
