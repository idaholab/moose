//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVOutflowTemperatureBC.h"
#include "SystemBase.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVOutflowTemperatureBC);

InputParameters
NSFVOutflowTemperatureBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription("Outflow velocity temperature advection boundary conditions for "
                             "finite volume method allowing for thermal backflow.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "The name of the density");
  params.addRequiredParam<MooseFunctorName>(NS::cp, "The name of the specific heat");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("backflow_T",
                                            "The backflow temperature entering the domain.");
  return params;
}

NSFVOutflowTemperatureBC::NSFVOutflowTemperatureBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _rho(getFunctor<ADReal>(NS::density)),
    _cp(getFunctor<ADReal>(NS::cp)),
    _u(getFunctor<ADReal>("u")),
    _v(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w(isParamValid("w") ? &getFunctor<ADReal>("w") : nullptr),
    _backflow_T(getFunctor<ADReal>("backflow_T")),
    _dim(_subproblem.mesh().dimension())
{
  if (_dim >= 2 && !_v)
    mooseError(
        "In two or more dimensions, the v velocity must be supplied using the 'v' parameter");
  if (_dim >= 3 && !_w)
    mooseError("In threedimensions, the w velocity must be supplied using the 'w' parameter");
}

ADReal
NSFVOutflowTemperatureBC::computeQpResidual()
{
  const auto boundary_face = singleSidedFaceArg();
  const auto state = determineState();

  ADRealVectorValue v(_u(boundary_face, state));
  if (_v)
    v(1) = (*_v)(boundary_face, state);
  if (_w)
    v(2) = (*_w)(boundary_face, state);

  const auto vol_flux = v * _normal;
  const auto rho_cp_face = _rho(boundary_face, state) * _cp(boundary_face, state);

  if (vol_flux > 0)
    return rho_cp_face * _var(boundary_face, state) * vol_flux;
  else
  {
    auto backflow_T = _backflow_T(boundary_face, state);
    return rho_cp_face * backflow_T * vol_flux;
  }
}
