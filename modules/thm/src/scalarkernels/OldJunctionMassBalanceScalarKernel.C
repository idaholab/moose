#include "OldJunctionMassBalanceScalarKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"

registerMooseObject("THMApp", OldJunctionMassBalanceScalarKernel);

template <>
InputParameters
validParams<OldJunctionMassBalanceScalarKernel>()
{
  InputParameters params = validParams<JunctionScalarKernel>();
  params.addRequiredCoupledVar("rhoA", "Density");
  params.addRequiredCoupledVar("rhouA", "Momentum");

  return params;
}

OldJunctionMassBalanceScalarKernel::OldJunctionMassBalanceScalarKernel(
    const InputParameters & parameters)
  : JunctionScalarKernel(parameters),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _rhouA(coupledValue("rhouA"))
{
}

void
OldJunctionMassBalanceScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  for (unsigned int i = 0; i < _rhouA.size(); i++)
    re(0) += _rhouA[i] * _normals[i];
}

void
OldJunctionMassBalanceScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & kee = _assembly.jacobianBlock(_var.number(), _var.number());
  // d[Res(lamda_0)] / d(lamda_0) = 0.0;
  for (unsigned int i = 0; i < kee.m(); i++)
    kee(i, i) = 0.;

  // Off-diagonal blocks
  // rho
  DenseMatrix<Number> & rho_ken = _assembly.jacobianBlock(_var.number(), _rhoA_var_number);
  for (unsigned int i = 0; i < _rhouA.size(); i++)
  {
    // d[Res(lamda_0)] / d(rho_i)
    // It is 0 in this case
    rho_ken(0, i) += 0.0;
  }
  // rhou
  DenseMatrix<Number> & rhou_ken = _assembly.jacobianBlock(_var.number(), _rhouA_var_number);
  for (unsigned int i = 0; i < _rhouA.size(); i++)
  {
    // d[Res(lamda_0)] / d(rhou_i)
    //   Res(lamda_0) = rhou_i * A_i * n_i
    //   d[Res(lamda_0)] / d(rhou_i) = A_i * n_i;
    rhou_ken(0, i) += _normals[i];
  }
}
