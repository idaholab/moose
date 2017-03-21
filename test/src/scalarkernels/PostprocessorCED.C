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

#include "PostprocessorCED.h"
#include "Assembly.h"

template <>
InputParameters
validParams<PostprocessorCED>()
{
  InputParameters params = validParams<ScalarKernel>();
  params.addRequiredParam<PostprocessorName>("pp_name", "");
  params.addRequiredParam<Real>("value", "");

  return params;
}

PostprocessorCED::PostprocessorCED(const InputParameters & parameters)
  : ScalarKernel(parameters),
    _value(getParam<Real>("value")),
    _pp_value(getPostprocessorValue("pp_name"))
{
}

PostprocessorCED::~PostprocessorCED() {}

void
PostprocessorCED::reinit()
{
}

void
PostprocessorCED::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());
  for (_i = 0; _i < re.size(); _i++)
    re(_i) += computeQpResidual();
}

Real
PostprocessorCED::computeQpResidual()
{
  return _pp_value - _value;
}

void
PostprocessorCED::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());
  for (_i = 0; _i < ke.m(); _i++)
    ke(_i, _i) += computeQpJacobian();
}

Real
PostprocessorCED::computeQpJacobian()
{
  return 0.;
}

void
PostprocessorCED::computeOffDiagJacobian(unsigned int /*jvar*/)
{
}

Real
PostprocessorCED::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
