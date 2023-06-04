/*
AuxKernel of Passing Variable Time Derivative
*/

#include "ComputeValueRate.h"

registerMooseObject("TensorMechanicsApp", ComputeValueRate);

InputParameters
ComputeValueRate::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredCoupledVar("coupled",
                               "Nonlinear Variable that needed to be taken time derivative of");

  return params;
}

ComputeValueRate::ComputeValueRate(const InputParameters & parameters)
  : AuxKernel(parameters),

    // Compute the time derivative of the given variable using "coupledDot"
    _coupled_val(coupledDot("coupled"))

{
}

Real
ComputeValueRate::computeValue()
{
  return _coupled_val[_qp];
}
