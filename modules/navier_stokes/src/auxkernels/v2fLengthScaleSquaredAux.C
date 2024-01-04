//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "v2fLengthScaleSquaredAux.h"
#include "NavierStokesMethods.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", v2fLengthScaleSquaredAux);

InputParameters
v2fLengthScaleSquaredAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates the turbulet length scale for the v2f model.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addParam<Real>("C_L", 0.23, "Mixing length cllosure parameter.");
  params.addParam<Real>("C_eta", 70.0, "Kolmogorov scale limiter parameter.");
  return params;
}

v2fLengthScaleSquaredAux::v2fLengthScaleSquaredAux(const InputParameters & params)
  : AuxKernel(params),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _C_L(getParam<Real>("C_L")),
    _C_eta(getParam<Real>("C_eta"))
{
}

Real
v2fLengthScaleSquaredAux::computeValue()
{
  // Convenient Arguments
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  const auto TKE = _k(elem_arg, state);
  const auto TKED = _epsilon(elem_arg, state);
  const auto rho = _rho(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  const auto nu = mu / rho;

  const auto bulk_scale = std::pow(TKE, 1.5) / (TKED + 1e-15);
  const auto kolmogorov_scale = _C_eta * std::pow(std::pow(nu, 3) / (TKED + 1e-15), 0.25);
  return raw_value(Utility::pow<2>(_C_L * std::max(bulk_scale, kolmogorov_scale)));
}
