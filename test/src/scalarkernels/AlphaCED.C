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

#include "AlphaCED.h"

template<>
InputParameters validParams<AlphaCED>()
{
  InputParameters params = validParams<ScalarKernel>();
  params.addRequiredParam<Real>("value", "The value we are enforcing");

  return params;
}

AlphaCED::AlphaCED(const std::string & name, InputParameters parameters) :
    ScalarKernel(name, parameters),
    _value(getParam<Real>("value"))
{
}

AlphaCED::~AlphaCED()
{
}

void
AlphaCED::reinit()
{
}

void
AlphaCED::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.index());
  for (_i = 0; _i < re.size(); _i++)
    re(_i) += computeQpResidual();
}

Real
AlphaCED::computeQpResidual()
{
  return _u[_i] - _value;
}

void
AlphaCED::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), _var.index());
  for (_i = 0; _i < ke.m(); _i++)
    ke(_i, _i) += computeQpJacobian();
}

Real
AlphaCED::computeQpJacobian()
{
  return 1.;
}

void
AlphaCED::computeOffDiagJacobian(unsigned int /*jvar*/)
{
}

Real
AlphaCED::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
