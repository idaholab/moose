#include "PostprocessorCED.h"

template<>
InputParameters validParams<PostprocessorCED>()
{
  InputParameters params = validParams<ScalarKernel>();
  params.addRequiredParam<PostprocessorName>("pp_name", "");
  params.addRequiredParam<Real>("value", "");

  return params;
}

PostprocessorCED::PostprocessorCED(const std::string & name, InputParameters parameters) :
    ScalarKernel(name, parameters),
    _value(getParam<Real>("value")),
    _pp_value(getPostprocessorValue("pp_name"))
{
}

PostprocessorCED::~PostprocessorCED()
{
}

void
PostprocessorCED::reinit()
{
}

void
PostprocessorCED::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.index());
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
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), _var.index());
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
