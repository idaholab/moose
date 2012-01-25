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

#include "CoefTimeDerivative.h"

template<>
InputParameters validParams<CoefTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<Real>("Coefficient", 1, "The coefficient for the time derivative kernel");
  return params;
}


CoefTimeDerivative::CoefTimeDerivative(const std::string & name,
                                   InputParameters parameters)
    :TimeDerivative(name,parameters),
     _coef(getParam<Real>("Coefficient"))
{}

Real
CoefTimeDerivative::computeQpResidual()
{
  // We're reusing the TimeDerivative Kernel's residual
  // so that we don't have to recode that.
  return _coef * TimeDerivative::computeQpResidual();
}

Real
CoefTimeDerivative::computeQpJacobian()
{
  return _coef * TimeDerivative::computeQpJacobian();
}
