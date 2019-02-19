#include "MassBalanceScalarKernel.h"
#include "Assembly.h"
#include "MooseVariableScalar.h"

registerMooseObject("THMApp", MassBalanceScalarKernel);

template <>
InputParameters
validParams<MassBalanceScalarKernel>()
{
  InputParameters params = validParams<NodalScalarKernel>();
  params.addRequiredCoupledVar("rhoA", "Density");
  params.addRequiredCoupledVar("rhouA", "Momentum");
  params.addRequiredParam<std::vector<Real>>("normals", "node normals");
  return params;
}

MassBalanceScalarKernel::MassBalanceScalarKernel(const InputParameters & parameters)
  : NodalScalarKernel(parameters),
    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _rhouA(coupledValue("rhouA")),
    _normals(getParam<std::vector<Real>>("normals"))
{
}

void
MassBalanceScalarKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  for (unsigned int i = 0; i < _rhouA.size(); i++)
    re(0) -= _rhouA[i] * _normals[i];
}

void
MassBalanceScalarKernel::computeJacobian()
{
  DenseMatrix<Number> & kee = _assembly.jacobianBlock(_var.number(), _var.number());
  // d[Res(lamda_0)] / d(lamda_0) = 0.0;
  for (unsigned int i = 0; i < kee.m(); i++)
    kee(i, i) = 0.;

  // Off-diagonal blocks
  // rhoA
  DenseMatrix<Number> & rho_ken = _assembly.jacobianBlock(_var.number(), _rhoA_var_number);
  for (unsigned int i = 0; i < _rhouA.size(); i++)
  {
    // d[Res(lamda_0)] / d(rho_i)
    // It is 0 in this case
    rho_ken(0, i) -= 0.0;
  }
  // rhouA
  DenseMatrix<Number> & rhou_ken = _assembly.jacobianBlock(_var.number(), _rhouA_var_number);
  for (unsigned int i = 0; i < _rhouA.size(); i++)
  {
    // d[Res(lamda_0)] / d(rhou_i)
    //   Res(lamda_0) = rhouA_i * n_i
    //   d[Res(lamda_0)] / d(rhou_i) = n_i;
    rhou_ken(0, i) -= _normals[i];
  }
}
