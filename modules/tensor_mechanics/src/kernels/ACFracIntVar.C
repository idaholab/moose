/*
Formulation: Miehe et. al., Int. J. Num. Methods Engg., 2010, 83. 1273-1311
Equation 63-2
Sets up residual for auxiliary variable
Strain energy contribution moved to equation 63-3
Use with ACFracCoupledInterface
*/
#include "ACFracIntVar.h"

template<>
InputParameters validParams<ACFracIntVar>()
{
  InputParameters params = validParams<KernelValue>();
  params.addRequiredCoupledVar("c", "Order Parameter");

  return params;
}

ACFracIntVar::ACFracIntVar(const std::string & name,
                           InputParameters parameters):
    KernelValue(name, parameters),
    _c(coupledValue("c"))
{
}

Real
ACFracIntVar::precomputeQpResidual()
{
  return _u[_qp];
}

Real
ACFracIntVar::precomputeQpJacobian()
{
  Real val=1.0;
  return val * _phi[_j][_qp];
}
