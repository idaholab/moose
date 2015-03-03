/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ThermoDiffusion.h"

namespace
{
const std::string temperature_name = "temp";
const std::string heat_of_transport_name = "heat_of_transport";
const std::string mass_diffusivity_name = "mass_diffusivity";
const std::string gas_constant_name = "gas_constant";
} // namespace anon

template<>
InputParameters validParams< ThermoDiffusion >()
{
  InputParameters params = validParams< Kernel >();
  params.addRequiredCoupledVar( temperature_name, "Coupled temperature" );
  params.addParam< Real >( gas_constant_name, 8.3144621, "Gas constant" );
  params.addClassDescription( "Kernel for thermo-diffusion (Soret effect, thermophoresis, etc.)" );
  return params;
}

ThermoDiffusion::ThermoDiffusion( const std::string & name, InputParameters parameters ) :
    Kernel( name, parameters ),
    _temperature( coupledValue( temperature_name ) ),
    _grad_temperature( coupledGradient( temperature_name ) ),
    _mass_diffusivity( getMaterialProperty< Real >( mass_diffusivity_name ) ),
    _heat_of_transport( getMaterialProperty< Real >( heat_of_transport_name ) ),
    _gas_constant( getParam< Real >( gas_constant_name ) ),
    _temperature_index( coupled( temperature_name ) )
{
}

RealGradient
ThermoDiffusion::thermoDiffusionVelocity() const
{
  // The thermo-diffusion term looks like grad( v * C ) where v is like a diffusive
  // velocity. If the concentration C does not couple back into the heat equation,
  // then the one-way coupling of temperature means that thermo-diffusion of C
  // behaves like advection. Then v is the velocity:
  //
  //   v = D Qstar grad(T) / ( R T^2 )
  //
  Real coeff = _mass_diffusivity[_qp] * _heat_of_transport[_qp] /
    ( _gas_constant * _temperature[_qp] * _temperature[_qp] );
  return coeff * _grad_temperature[_qp];
}

Real
ThermoDiffusion::computeQpResidual()
{
  return thermoDiffusionVelocity() * _u[_qp] * _grad_test[_i][_qp];
}

Real
ThermoDiffusion::computeQpJacobian()
{
  return thermoDiffusionVelocity() * _phi[_j][_qp] * _grad_test[_i][_qp];
}

Real
ThermoDiffusion::computeQpOffDiagJacobian( unsigned int jvar )
{
  if ( jvar == _temperature_index )
  {
    Real coeff = _mass_diffusivity[_qp] * _heat_of_transport[_qp] /
      ( _gas_constant * _temperature[_qp] * _temperature[_qp] );
    return coeff * _grad_test[_i][_qp] * _u[_qp] * ( _grad_phi[_j][_qp] -
      2 * _phi[_j][_qp] * _grad_temperature[_qp] / _temperature[_qp] );
  }
  return 0;
}
