//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVEnergyAmbientConvection.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVEnergyAmbientConvection);

InputParameters
PINSFVEnergyAmbientConvection::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Implements the solid-fluid ambient convection term in the porous "
                             "media Navier Stokes energy equation.");
  params.addRequiredParam<MooseFunctorName>("h_solid_fluid",
                                            "Name of the convective heat transfer coefficient.");
  params.addRequiredParam<bool>("is_solid", "Whether this kernel acts on the solid temperature");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "Fluid temperature");
  params.addRequiredParam<MooseFunctorName>(NS::T_solid, "Solid temperature");
  return params;
}

PINSFVEnergyAmbientConvection::PINSFVEnergyAmbientConvection(const InputParameters & parameters)
  : FVElementalKernel(parameters),
    _h_solid_fluid(getFunctor<ADReal>("h_solid_fluid")),
    _temp_fluid(getFunctor<ADReal>(NS::T_fluid)),
    _temp_solid(getFunctor<ADReal>(NS::T_solid)),
    _is_solid(getParam<bool>("is_solid"))
{
}

ADReal
PINSFVEnergyAmbientConvection::computeQpResidual()
{
  const auto & elem = makeElemArg(_current_elem);
  const auto state = determineState();

  if (_is_solid)
    return -_h_solid_fluid(elem, state) * (_temp_fluid(elem, state) - _temp_solid(elem, state));
  else
    return _h_solid_fluid(elem, state) * (_temp_fluid(elem, state) - _temp_solid(elem, state));
}
