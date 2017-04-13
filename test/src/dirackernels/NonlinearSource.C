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

#include "NonlinearSource.h"

template <>
InputParameters
validParams<NonlinearSource>()
{
  InputParameters params = validParams<DiracKernel>();
  params.addRequiredCoupledVar("coupled_var",
                               "The variable whose value is coupled into the source term.");
  params.addRequiredParam<Real>("scale_factor", "Strength of the source term");
  params.addRequiredParam<Point>("point", "The x,y,z coordinates of the point");
  return params;
}

NonlinearSource::NonlinearSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _coupled_var(coupledValue("coupled_var")),
    _coupled_var_num(coupled("coupled_var")),
    _scale_factor(parameters.get<Real>("scale_factor")),
    _p(getParam<Point>("point"))
{
}

void
NonlinearSource::addPoints()
{
  // The point where the source term is applied.  Pass an ID of zero
  // to use the point caching stuff and speed up addPoints().
  addPoint(_p, 0);
}

Real
NonlinearSource::computeQpResidual()
{
  // c * u * v
  // return -_scale_factor * _u[_qp] * _coupled_var[_qp] * _test[_i][_qp];

  // c * u^2 * v^2
  // Real
  //   u2 = _u[_qp] * _u[_qp],
  //   v2 = _coupled_var[_qp] * _coupled_var[_qp];
  // return -_scale_factor * u2 * v2 * _test[_i][_qp];

  // c
  // return -_scale_factor * _test[_i][_qp];

  // c * v (decoupled from u case, the on-diagonal Jacobians are zero!)
  return -_scale_factor * _coupled_var[_qp] * _test[_i][_qp];
}

Real
NonlinearSource::computeQpJacobian()
{
  // c * u * v
  // return -_scale_factor * _phi[_j][_qp] * _coupled_var[_qp] * _test[_i][_qp];

  // c * u^2 * v^2
  // Real v2 = _coupled_var[_qp] * _coupled_var[_qp];
  // return -_scale_factor * 2. * _u[_qp] * _phi[_j][_qp] * v2 * _test[_i][_qp];

  // c
  // return 0.;

  // c * v (decoupled from u case, the on-diagonal Jacobians are zero!)
  return 0.;
}

Real
NonlinearSource::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _coupled_var_num)
  {
    // c * u * v
    // return -_scale_factor * _u[_qp] * _phi[_j][_qp] * _test[_i][_qp];

    // c * u^2 * v^2
    // Real u2 = _u[_qp] * _u[_qp];
    // return -_scale_factor * u2 * 2. * _coupled_var[_qp] * _phi[_j][_qp] * _test[_i][_qp];

    // c
    // return 0.;

    // c * v (decoupled from u case, the on-diagonal Jacobians are zero!)
    return -_scale_factor * _phi[_j][_qp] * _test[_i][_qp];
  }
  else
    return 0.;
}
