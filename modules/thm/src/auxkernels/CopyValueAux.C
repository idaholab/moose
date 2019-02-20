#include "CopyValueAux.h"

registerMooseObject("THMApp", CopyValueAux);

template <>
InputParameters
validParams<CopyValueAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("source", "Source variable to be copied.");

  return params;
}

CopyValueAux::CopyValueAux(const InputParameters & parameters)
  : AuxKernel(parameters), _source_var(coupledValue("source"))
{
}

Real
CopyValueAux::computeValue()
{
  return _source_var[_qp];
}
