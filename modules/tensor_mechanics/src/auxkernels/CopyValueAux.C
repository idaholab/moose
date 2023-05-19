/*
AuxKernel of Passing Variable
*/

#include "CopyValueAux.h"

registerMooseObject("TensorMechanicsApp", CopyValueAux);

InputParameters
CopyValueAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredCoupledVar("coupled","Nonlinear Variable that needed to be passed");

  return params;
}

CopyValueAux::CopyValueAux(const InputParameters & parameters)
  : AuxKernel(parameters),

  _coupled_val(coupledValue("coupled"))

{
}

Real
CopyValueAux::computeValue()
{
  return _coupled_val[_qp];
}
