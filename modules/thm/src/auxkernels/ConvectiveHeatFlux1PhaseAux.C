#include "ConvectiveHeatFlux1PhaseAux.h"

registerMooseObject("THMApp", ConvectiveHeatFlux1PhaseAux);

template <>
InputParameters
validParams<ConvectiveHeatFlux1PhaseAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Computes convective heat flux for 1-phase flow.");
  params.addRequiredCoupledVar("T_wall", "Wall temperature");
  params.addRequiredParam<MaterialPropertyName>("T", "Material property name of fluid temperature");
  params.addRequiredParam<MaterialPropertyName>(
      "Hw", "Material property name of wall heat transfer coefficient");
  params.addParam<Real>("scaling_factor", 1., "Scaling factor");
  return params;
}

ConvectiveHeatFlux1PhaseAux::ConvectiveHeatFlux1PhaseAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _T_wall(coupledValue("T_wall")),
    _T(getMaterialProperty<Real>("T")),
    _Hw(getMaterialProperty<Real>("Hw")),
    _scaling_factor(getParam<Real>("scaling_factor"))
{
}

Real
ConvectiveHeatFlux1PhaseAux::computeValue()
{
  return _Hw[_qp] * (_T_wall[_qp] - _T[_qp]) * _scaling_factor;
}
