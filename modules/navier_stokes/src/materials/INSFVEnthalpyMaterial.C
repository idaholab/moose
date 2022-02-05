//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVEnthalpyMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVEnthalpyMaterial);

InputParameters
INSFVEnthalpyMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("This is the material class used to compute enthalpy for "
                             "the incompressible/weakly-compressible finite-volume implementation "
                             "of the Navier-Stokes equations.");
  params.addRequiredParam<MooseFunctorName>("rho", "The value for the density");
  params.addRequiredParam<MooseFunctorName>("temperature", "the temperature");
  params.addParam<MooseFunctorName>("cp_name", "cp", "the name of the specific heat capacity");
  return params;
}

INSFVEnthalpyMaterial::INSFVEnthalpyMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rho(getFunctor<ADReal>("rho")),
    _temperature(getFunctor<ADReal>("temperature")),
    _cp(getFunctor<ADReal>("cp_name"))
{
  addFunctorProperty<ADReal>("rho_cp_temp",
                             [this](const auto & r, const auto & t) -> ADReal
                             { return _rho(r, t) * _cp(r, t) * _temperature(r, t); });
}
