//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFV2PInterfacialMassTransferFunctorMaterial.h"
#include "NS.h"
#include "NavierStokesMethods.h"
#include "HeatTransferUtils.h"

registerMooseObject("NavierStokesApp", WCNSFV2PInterfacialMassTransferFunctorMaterial);

InputParameters
WCNSFV2PInterfacialMassTransferFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Computes the interfacial mass transfer rate of the two-phase mixture model from the "
      "transported interfacial area concentration and the departure of the mixture temperature "
      "from saturation, rather than taking that rate as prescribed.");
  params.addRequiredParam<MooseFunctorName>(
      "interfacial_area", "Interfacial area concentration of the dispersed phase.");
  params.addRequiredParam<MooseFunctorName>("fraction_dispersed",
                                            "Volume fraction of the dispersed phase.");
  params.addRequiredParam<MooseFunctorName>(NS::T_fluid, "Temperature of the mixture.");
  params.addRequiredParam<MooseFunctorName>(
      "T_saturation", "Saturation temperature of the transition the transfer represents.");
  params.addRequiredParam<MooseFunctorName>("latent_heat",
                                            "Latent heat of that transition, per unit mass.");
  params.addRequiredParam<MooseFunctorName>("rho_c", "Continuous phase density.");
  params.addRequiredParam<MooseFunctorName>("mu_c", "Continuous phase dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>("k_c", "Continuous phase thermal conductivity.");
  params.addRequiredParam<MooseFunctorName>("u_slip", "The slip velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The slip velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The slip velocity in the z direction.");
  params.addParam<Real>(
      "shape_factor",
      6.0,
      "Shape factor psi of the averaged particle size, 6 for spheres. This must match the value "
      "the interfacial area source and sink is given, since both invert the same area to recover "
      "the same particle size.");
  params.addParam<MooseFunctorName>("interfacial_mass_transfer_name",
                                    "interfacial_mass_transfer",
                                    "Name to give the computed functor property.");
  return params;
}

WCNSFV2PInterfacialMassTransferFunctorMaterial::WCNSFV2PInterfacialMassTransferFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _dim(_subproblem.mesh().dimension()),
    _shape_factor(getParam<Real>("shape_factor")),
    _interfacial_area(getFunctor<Real>("interfacial_area")),
    _fd(getFunctor<Real>("fraction_dispersed")),
    _temperature(getFunctor<Real>(NS::T_fluid)),
    _T_saturation(getFunctor<Real>("T_saturation")),
    _latent_heat(getFunctor<Real>("latent_heat")),
    _rho_c(getFunctor<Real>("rho_c")),
    _mu_c(getFunctor<Real>("mu_c")),
    _k_c(getFunctor<Real>("k_c")),
    _u_slip(getFunctor<Real>("u_slip")),
    _v_slip(isParamValid("v_slip") ? &getFunctor<Real>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<Real>("w_slip") : nullptr)
{
  NS::checkSlipVelocityComponents(*this, _dim, _v_slip, _w_slip);

  const auto mass_transfer = [this](const auto & r, const auto & t) -> Real
  {
    const auto area = _interfacial_area(r, t);
    const auto fd = _fd(r, t);

    // With no interface, or no dispersed phase for it to bound, there is nothing for the transfer
    // to act on. The floor matches the one LinearWCNSFV2PInterfaceAreaSourceSink applies to the
    // term this rate feeds.
    constexpr Real minimum_transfer_fraction = 1e-10;
    if (area <= 0.0 || fd <= minimum_transfer_fraction)
      return 0.0;

    // The averaged particle size, which the interfacial area concentration inverts. This is the
    // same relation the interfacial area source and sink uses, so that the Reynolds number the
    // Nusselt correlation is evaluated at describes the same particle population.
    const auto d_b = _shape_factor * fd / area;

    RealVectorValue slip(_u_slip(r, t));
    if (_dim > 1)
      slip(1) = (*_v_slip)(r, t);
    if (_dim > 2)
      slip(2) = (*_w_slip)(r, t);

    const auto reynolds = HeatTransferUtils::reynolds(_rho_c(r, t), slip.norm(), d_b, _mu_c(r, t));

    // RELAP5/MOD3 modified Lee-Ryley, NUREG/CR-5535 Volume 4 Section 4.1.1.1.1, with the Prandtl
    // dependence dropped as that reference drops it for bubbly flow
    const auto nusselt = 2.0 + 0.74 * std::sqrt(reynolds);
    const auto interfacial_coefficient = _k_c(r, t) * nusselt / d_b;

    const auto latent_heat = _latent_heat(r, t);
    if (latent_heat <= 0.0)
      mooseException("A non-positive latent heat of ",
                     latent_heat,
                     " leaves the interfacial mass transfer rate undefined");

    return area * interfacial_coefficient * (_temperature(r, t) - _T_saturation(r, t)) /
           latent_heat;
  };

  addFunctorProperty<Real>(getParam<MooseFunctorName>("interfacial_mass_transfer_name"),
                           mass_transfer);
}
