#include "PFFracIntVar.h"

template<>
InputParameters validParams<PFFracIntVar>()
{
  InputParameters params = validParams<KernelValue>();

  return params;
}

PFFracIntVar::PFFracIntVar(const std::string & name,
                           InputParameters parameters):
  KernelValue(name, parameters)
{
}

Real
PFFracIntVar::precomputeQpResidual()
{
  //Residual is the variable value
  return _u[_qp];
}

Real
PFFracIntVar::precomputeQpJacobian()
{
  Real val=1.0;
  return val * _phi[_j][_qp];
}
