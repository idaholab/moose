/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ThermoDiffusion.h"

template<>
InputParameters validParams< ThermoDiffusion >()
{
  InputParameters params = validParams< Kernel >();
  params.addRequiredCoupledVar( "temp", "Coupled temperature" );
  params.addParam< Real >( "gas_constant", 8.3144621, "Gas constant" );
  params.addParam< std::string >( "heat_of_transport", "heat_of_transport", "Property name for the heat of transport.");
  params.addParam< std::string >( "mass_diffusivity", "mass_diffusivity", "Property name for the diffusivity.");

  params.addClassDescription( "Kernel for thermo-diffusion (Soret effect, thermophoresis, etc.)" );
  return params;
}

ThermoDiffusion::ThermoDiffusion( const std::string & name, InputParameters parameters ) :
    Kernel( name, parameters ),
    _temperature( coupledValue( "temp" ) ),
    _grad_temperature( coupledGradient( "temp" ) ),
    _mass_diffusivity( getMaterialProperty< Real >( getParam< std::string >( "mass_diffusivity" ) ) ),
    _heat_of_transport( getMaterialProperty< Real >( getParam< std::string >( "heat_of_transport" ) ) ),
    _gas_constant( getParam< Real >( "gas_constant" ) ),
    _temperature_index( coupled( "temp" ) )
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
