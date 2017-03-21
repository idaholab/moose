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

#include "MassLumpedTimeDerivative.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<MassLumpedTimeDerivative>()
{
  InputParameters params = validParams<TimeKernel>();
  return params;
}

MassLumpedTimeDerivative::MassLumpedTimeDerivative(const InputParameters & parameters)
  : TimeKernel(parameters), _u_dot_nodal(_var.nodalValueDot())
{
}

Real
MassLumpedTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _u_dot_nodal[_i];
}

Real
MassLumpedTimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp] * _du_dot_du[_qp];
}

void
MassLumpedTimeDerivative::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      ke(_i, _i) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
}
