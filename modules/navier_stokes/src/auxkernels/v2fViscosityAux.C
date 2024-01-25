//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "v2fViscosityAux.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", v2fViscosityAux);

InputParameters
v2fViscosityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates the turbulent viscosity according to the v2f model.");
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::TV2, "Coupled turbulent wall normal fluctuations.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addParam<Real>("C_mu_2", 0.22, "Coupled turbulent viscosity closure.");
  params.addParam<Real>("C_mu", 0.09, "Coupled turbulent viscosity closure.");
  return params;
}

v2fViscosityAux::v2fViscosityAux(const InputParameters & params)
  : AuxKernel(params),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _v2(getFunctor<ADReal>(NS::TV2)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _C_mu_2(getParam<Real>("C_mu_2")),
    _C_mu(getParam<Real>("C_mu"))
{
}

Real
v2fViscosityAux::computeValue()
{
  // Convenient Arguments
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  const auto TKE = _k(elem_arg, state);
  const auto TKED = _epsilon(elem_arg, state);
  const auto TV2 = _v2(elem_arg, state);
  const auto rho = _rho(elem_arg, state);
  const auto mu = _mu(elem_arg, state);
  const auto nu = mu / rho;

  const auto time_scale_keps = TKE / TKED;
  const auto time_scale = std::max(time_scale_keps, 6 * std::sqrt(nu / TKED));

  const auto mu_t = rho * std::min(_C_mu * TKE * time_scale_keps, _C_mu_2 * TV2 * time_scale);

  // _console << "--------------------------------" << std::endl;
  // _console << "TKE: " << raw_value(TKE) << std::endl;
  // _console << "TKED: " << raw_value(TKED) << std::endl;
  // _console << "TV2: " << raw_value(TV2) << std::endl;
  // _console << "mu_t: " << raw_value(mu_t) << std::endl;
  // _console << "--------------------------------" << std::endl;

  return raw_value(mu_t);
}
