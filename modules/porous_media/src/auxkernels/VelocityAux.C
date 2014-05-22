/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "VelocityAux.h"

template<>
InputParameters validParams<VelocityAux>()
{
     InputParameters params = validParams<AuxKernel>();
     params.addParam<int>("component",0,"Direction/component of the velocity vector (0=x, 1=y, 2=z)");
     MooseEnum fluid_phase("steam, water", "water");
     params.addParam<MooseEnum>("phase", fluid_phase, "Sets the phase of interest (water, steam)");
     return params;
}

VelocityAux::VelocityAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _darcy_flux_water(getMaterialProperty<RealGradient>("darcy_flux_water")),
   _darcy_flux_steam(getMaterialProperty<RealGradient>("darcy_flux_steam")),
   _porosity(getMaterialProperty<Real>("porosity")),
   _phase(getParam<MooseEnum>("phase")),
   _i(getParam<int>("component"))

{}

Real
VelocityAux::computeValue()
{
  if (_phase == "steam")
  {
    return _darcy_flux_steam[_qp](_i)/_porosity[_qp];
  }
  else //the phase is water
  {
    return _darcy_flux_water[_qp](_i)/_porosity[_qp];
  }

}

