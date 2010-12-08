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

#include "MMSImplicitEuler.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<MMSImplicitEuler>()
{
  InputParameters params = validParams<TimeKernel>();
  return params;
}

MMSImplicitEuler::MMSImplicitEuler(const std::string & name, InputParameters parameters)
  :TimeKernel(name, parameters)
{}

Real
MMSImplicitEuler::computeQpResidual()
{
  return _test[_i][_qp]*((_u[_qp]-_u_old[_qp])/_dt);
}

Real
MMSImplicitEuler::computeQpJacobian()
{
  return _test[_i][_qp]*_phi[_j][_qp]/_dt;
}
