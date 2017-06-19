#include "HydraulicDiameterCircularAux.h"

template <>
InputParameters
validParams<HydraulicDiameterCircularAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("A", "Cross-sectional area");
  return params;
}

HydraulicDiameterCircularAux::HydraulicDiameterCircularAux(const InputParameters & parameters)
  : AuxKernel(parameters), _area(coupledValue("A"))
{
}

Real
HydraulicDiameterCircularAux::computeValue()
{
  return std::sqrt(4. * _area[_qp] / libMesh::pi);
}
