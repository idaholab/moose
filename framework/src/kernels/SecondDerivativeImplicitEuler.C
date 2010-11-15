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

#include "SecondDerivativeImplicitEuler.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<SecondDerivativeImplicitEuler>()
{
  InputParameters params = validParams<TimeKernel>();
  return params;
}

SecondDerivativeImplicitEuler::SecondDerivativeImplicitEuler(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :TimeKernel(name, moose_system, parameters)
{}

Real
SecondDerivativeImplicitEuler::computeQpResidual()
{
  return _test[_i][_qp]*((_u[_qp]-2*_u_old[_qp]+_u_older[_qp])/(_moose_system._dt*_moose_system._dt));
}

Real
SecondDerivativeImplicitEuler::computeQpJacobian()
{
  return _test[_i][_qp]*(_phi[_j][_qp]/(_moose_system._dt*_moose_system._dt));
}
