#include "HeatConductionBC.h"

template<>
InputParameters validParams<HeatConductionBC>()
{
  InputParameters params = validParams<FluxBC>();

  return params;
}

HeatConductionBC::HeatConductionBC(const std::string & name, InputParameters parameters) :
    FluxBC(name, parameters),
    _k(getMaterialProperty<Real>("thermal_conductivity"))
{
}

HeatConductionBC::~HeatConductionBC()
{
}

RealGradient
HeatConductionBC::computeQpFluxResidual()
{
  return -_k[_qp] * _grad_u[_qp];
}

RealGradient
HeatConductionBC::computeQpFluxJacobian()
{
  return -_k[_qp] * _grad_phi[_j][_qp];
}

