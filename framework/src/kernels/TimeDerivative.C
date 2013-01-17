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

#include "TimeDerivative.h"

template<>
InputParameters validParams<TimeDerivative>()
{
  InputParameters params = validParams<TimeKernel>();
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise");
  return params;
}

TimeDerivative::TimeDerivative(const std::string & name, InputParameters parameters) :
    TimeKernel(name, parameters),
    _lumping(getParam<bool>("lumping"))
{
}

Real
TimeDerivative::computeQpResidual()
{
  return _test[_i][_qp]*_u_dot[_qp];
}

Real
TimeDerivative::computeQpJacobian()
{
  return _test[_i][_qp]*_phi[_j][_qp]*_du_dot_du[_qp];
}

void
TimeDerivative::computeJacobian()
{
  if (_lumping)
  {
    DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), _var.index());

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        {
          ke(_i, _i) += _JxW[_qp] * _coord[_qp] * computeQpJacobian();
        }
  }
  else
    TimeKernel::computeJacobian();
}
