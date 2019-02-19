#include "JunctionMassBalanceScalarKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"

registerMooseObject("THMApp", JunctionMassBalanceScalarKernel);

template <>
InputParameters
validParams<JunctionMassBalanceScalarKernel>()
{
  InputParameters params = validParams<JunctionScalarKernel>();
  params.addRequiredCoupledVar("rhouA", "Momentum");

  return params;
}

JunctionMassBalanceScalarKernel::JunctionMassBalanceScalarKernel(const InputParameters & parameters)
  : JunctionScalarKernel(parameters),
    _rhouA_var_number(coupled("rhouA")),
    _rhouA(coupledValue("rhouA"))
{
}

void
JunctionMassBalanceScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  re(0) = 0;
  for (unsigned int i = 0; i < _rhouA.size(); i++)
    re(0) += _rhouA[i] * _normals[i];
}

void
JunctionMassBalanceScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & J_diag = _assembly.jacobianBlock(_var.number(), _var.number());
  for (unsigned int i = 0; i < J_diag.m(); i++)
    J_diag(i, i) = 0;

  DenseMatrix<Number> & J_rhouA = _assembly.jacobianBlock(_var.number(), _rhouA_var_number);
  for (unsigned int i = 0; i < _rhouA.size(); i++)
    J_rhouA(0, i) += _normals[i];
}
