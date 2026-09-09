//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFV2PInterfaceAreaSourceSink.h"
#include "NavierStokesMethods.h"
#include "NS.h"

#include <algorithm>

#include "libmesh/utility.h"

registerMooseObject("NavierStokesApp", LinearWCNSFV2PInterfaceAreaSourceSink);

InputParameters
LinearWCNSFV2PInterfaceAreaSourceSink::validParams()
{
  InputParameters params = LinearFVElementalKernel::validParams();
  params.addClassDescription(
      "Sources and sinks of the interfacial area concentration transport equation, in the form of "
      "for the linear finite volume discretization. "
      "Offers the Hibiki and Ishii and the Ishii and Kim closure sets.");

  MooseEnum model("hibiki-ishii ishii-kim", "hibiki-ishii");
  params.addParam<MooseEnum>("model", model, "Which closure set to evaluate.");

  params.addRequiredParam<MooseFunctorName>("u",
                                            "The dispersed phase velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The dispersed phase velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The dispersed phase velocity in the z direction.");

  params.addRequiredParam<MooseFunctorName>("rho_d", "Density of the dispersed phase, rho_g.");
  params.addRequiredParam<MooseFunctorName>("rho_f", "Density of the continuous phase, rho_f.");
  params.addParam<MooseFunctorName>(
      "mu_f",
      "Dynamic viscosity of the continuous phase. Required by the Ishii and Kim model, which "
      "forms a particle Reynolds number for its drag coefficient.");
  params.addParam<MooseFunctorName>("fd", 0.0, "Volume fraction of the dispersed phase, alpha_g.");
  params.renameParam("fd", "fraction_dispersed", "");
  params.addParam<MooseFunctorName>("sigma", 1.0, "Surface tension between the phases.");
  params.addRequiredParam<MooseFunctorName>("epsilon",
                                            "Turbulent dissipation rate of the continuous phase.");
  params.addParam<MooseFunctorName>(
      "mass_transfer_rate",
      0.0,
      "Mass transfer rate into the dispersed phase per unit mixture volume, mdot_g.");

  params.addParam<Real>(
      "shape_factor", 6.0, "Shape factor psi of the averaged particle size, 6 for spheres.");
  params.addParam<Real>("fd_max",
                        0.75,
                        "Maximum volume fraction admitted by the model. The reference states 0.75 "
                        "with the Ishii and Kim coefficients.");
  params.addParam<RealVectorValue>("gravity",
                                   RealVectorValue(0, 0, 0),
                                   "Gravity vector. Required by the Ishii and Kim terminal "
                                   "velocity of the wake entrainment sink.");

  // Hibiki and Ishii coefficients, from the reference
  params.addParam<Real>("gamma_c", 0.188, "Hibiki and Ishii coalescence coefficient Gamma_C.");
  params.addParam<Real>("k_c", 0.129, "Hibiki and Ishii coalescence coefficient K_C.");
  params.addParam<Real>("gamma_b", 0.264, "Hibiki and Ishii breakage coefficient Gamma_B.");
  params.addParam<Real>("k_b", 1.37, "Hibiki and Ishii breakage coefficient K_B.");

  // Ishii and Kim coefficients, from the reference
  params.addParam<Real>("c_rc", 0.004, "Ishii and Kim random collision coefficient C_RC.");
  params.addParam<Real>("c_we", 0.002, "Ishii and Kim wake entrainment coefficient C_WE.");
  params.addParam<Real>("c_ti", 0.085, "Ishii and Kim turbulent impact coefficient C_TI.");
  params.addParam<Real>("c", 3.0, "Ishii and Kim coalescence efficiency coefficient C.");
  params.addParam<Real>("we_cr", 6.0, "Ishii and Kim critical Weber number.");

  return params;
}

LinearWCNSFV2PInterfaceAreaSourceSink::LinearWCNSFV2PInterfaceAreaSourceSink(
    const InputParameters & params)
  : LinearFVElementalKernel(params),
    _model(getParam<MooseEnum>("model").getEnum<ModelEnum>()),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<Real>("u")),
    _v_var(isParamValid("v") ? &getFunctor<Real>("v") : nullptr),
    _w_var(isParamValid("w") ? &getFunctor<Real>("w") : nullptr),
    _rho_d(getFunctor<Real>("rho_d")),
    _rho_f(getFunctor<Real>("rho_f")),
    _mu_f(isParamValid("mu_f") ? &getFunctor<Real>("mu_f") : nullptr),
    _f_d(getFunctor<Real>("fd")),
    _sigma(getFunctor<Real>("sigma")),
    _epsilon(getFunctor<Real>("epsilon")),
    _mass_transfer_rate(getFunctor<Real>("mass_transfer_rate")),
    _shape_factor(getParam<Real>("shape_factor")),
    _f_d_max(getParam<Real>("fd_max")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _gamma_c(getParam<Real>("gamma_c")),
    _kc(getParam<Real>("k_c")),
    _gamma_b(getParam<Real>("gamma_b")),
    _kb(getParam<Real>("k_b")),
    _c_rc(getParam<Real>("c_rc")),
    _c_we(getParam<Real>("c_we")),
    _c_ti(getParam<Real>("c_ti")),
    _c(getParam<Real>("c")),
    _we_cr(getParam<Real>("we_cr")),
    _implicit_coefficient(0.0),
    _lagged_source(0.0)
{
  if (_dim >= 2 && !_v_var)
    paramError("v", "In two or more dimensions, the v velocity must be supplied.");
  if (_dim >= 3 && !_w_var)
    paramError("w", "In three dimensions, the w velocity must be supplied.");
  if (_model == ModelEnum::ISHII_KIM && !_mu_f)
    paramError("mu_f",
               "The Ishii and Kim model needs the continuous phase viscosity to form the particle "
               "Reynolds number of its drag coefficient.");
  if (_model == ModelEnum::ISHII_KIM && _gravity.norm() == 0)
    paramError("gravity",
               "The Ishii and Kim wake entrainment sink is driven by the terminal velocity of "
               "terminal velocity, which vanishes without gravity.");
}

Real
LinearWCNSFV2PInterfaceAreaSourceSink::solveTerminalVelocity(const Real stokes_velocity,
                                                             const Real reynolds_per_velocity)
{
  if (stokes_velocity <= 0.0 || reynolds_per_velocity <= 0.0)
    return stokes_velocity;

  return NS::bracketedNewton(
      [=](const Real u)
      {
        const Real correction = 0.1 * std::pow(reynolds_per_velocity * u, 0.75);
        // d/du [u (1 + 0.1 (B u)^{3/4})] = 1 + 0.175 (B u)^{3/4}
        return std::make_pair(u * (1.0 + correction) - stokes_velocity, 1.0 + 1.75 * correction);
      },
      0.0,
      stokes_velocity,
      stokes_velocity,
      stokes_velocity);
}

void
LinearWCNSFV2PInterfaceAreaSourceSink::computeCoefficients()
{
  using std::cbrt, std::exp, std::pow, std::sqrt;

  const auto elem_arg = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();

  const auto rho_d = _rho_d(elem_arg, state);
  const auto rho_f = _rho_f(elem_arg, state);
  const auto sigma = _sigma(elem_arg, state);
  const auto epsilon = _epsilon(elem_arg, state);
  const auto f_d = std::clamp(_f_d(elem_arg, state), 0.0, _f_d_max);
  // The interfacial area concentration is lagged wherever it enters nonlinearly
  const auto xi = _var.getElemValue(*_current_elem_info, state);

  // Material derivative of the dispersed phase density, which drives the expansion term
  const auto rho_d_grad = _rho_d.gradient(elem_arg, state);
  Real material_time_derivative_rho_d = _u_var(elem_arg, state) * rho_d_grad(0);
  if (_v_var)
    material_time_derivative_rho_d += (*_v_var)(elem_arg, state) * rho_d_grad(1);
  if (_w_var)
    material_time_derivative_rho_d += (*_w_var)(elem_arg, state) * rho_d_grad(2);
  if (_subproblem.isTransient())
    material_time_derivative_rho_d += (rho_d - _rho_d(elem_arg, Moose::oldState())) / _dt;

  // With no dispersed phase, or no interface, every source vanishes. Returning zero rather than
  // flooring the particle size avoids an additive offset on a size that inverts the interfacial
  // area concentration, and avoids evaluating the closures below a db of about 8e-89, where the
  // coalescence prefactor 1/db^(11/3) overflows and the breakage term multiplies it by an
  // exponential that has flushed to zero.
  constexpr Real smallest_evaluable_size = 1e-30;
  const auto mean_size = (xi > 0.0) ? _shape_factor * f_d / xi : 0.0;
  const bool has_interface = (f_d > 0.0) && (xi > 0.0) && (mean_size > smallest_evaluable_size);

  // The averaged size of the particle
  const auto db = has_interface ? mean_size : 0.0;

  // The coalescence sinks are non-positive and the breakage source non-negative, by construction
  Real s_rc = 0.0, s_we = 0.0, s_ti = 0.0;

  if (!has_interface)
  {
    // Nothing to do, the sources stay at zero
  }
  else if (_model == ModelEnum::HIBIKI_ISHII)
  {
    // Hibiki and Ishii. The packing factor divides, so both terms grow without bound
    // as the dispersed phase approaches its maximum.
    const auto packing = std::max(_f_d_max - f_d, libMesh::TOLERANCE);
    const auto prefactor =
        Utility::pow<2>(f_d / xi) * cbrt(epsilon) / (pow(db, 11. / 3.) * packing);

    s_rc = -prefactor * _gamma_c * Utility::pow<2>(f_d) *
           exp(-_kc * pow(db, 5. / 6.) * sqrt(rho_f) * cbrt(epsilon) / sqrt(sigma));
    s_ti = prefactor * _gamma_b * f_d * (1.0 - f_d) *
           exp(-_kb * sigma / (rho_f * pow(db, 5. / 3.) * Utility::pow<2>(cbrt(epsilon))));
    // No wake entrainment model in this formulation
  }
  else
  {
    // The mean bubble fluctuating velocity, u_t = epsilon^(1/3) d_b^(1/3)
    const auto u_t = cbrt(epsilon) * cbrt(db);

    // Random collision. The cube roots of the volume fractions, with the difference floored so
    // that the sink stays finite at the packing limit.
    const auto cbrt_max = cbrt(_f_d_max);
    const auto cbrt_fd = cbrt(f_d);
    const auto cbrt_gap = std::max(cbrt_max - cbrt_fd, libMesh::TOLERANCE);
    s_rc = -1.0 / (3.0 * libMesh::pi) * _c_rc * u_t * Utility::pow<2>(xi) / (cbrt_max * cbrt_gap) *
           (1.0 - exp(-_c * cbrt_max * cbrt_fd / cbrt_gap));

    // Wake entrainment. The terminal velocity and the drag coefficient are
    // mutually implicit and are solved together.
    const auto delta_rho = std::abs(rho_f - rho_d);
    const auto reynolds_per_velocity = rho_f * db * (1.0 - f_d) / (*_mu_f)(elem_arg, state);
    const auto stokes_velocity =
        reynolds_per_velocity / 24.0 * db * _gravity.norm() * delta_rho / (3.0 * rho_f);
    const auto u_r = solveTerminalVelocity(stokes_velocity, reynolds_per_velocity);
    const auto reynolds = reynolds_per_velocity * u_r;
    const auto drag_coefficient =
        (reynolds > libMesh::TOLERANCE) ? 24.0 * (1.0 + 0.1 * pow(reynolds, 0.75)) / reynolds : 0.0;
    s_we = -1.0 / (3.0 * libMesh::pi) * _c_we * u_r * Utility::pow<2>(xi) * cbrt(drag_coefficient);

    // Turbulent impact. Below the critical Weber number the breakage rate is zero.
    const auto weber = rho_f * Utility::pow<2>(u_t) * db / sigma;
    if (weber > _we_cr)
      s_ti = 1.0 / 18.0 * _c_ti * u_t * Utility::pow<2>(xi) / f_d * sqrt(1.0 - _we_cr / weber) *
             exp(-_we_cr / weber);
  }

  // Everything is moved to the left hand side. The coalescence sinks are linearized about the
  // previous iterate, which keeps the diagonal coefficient non-negative and is exact at
  // convergence; the breakage source stays on the right hand side.
  _implicit_coefficient = 0.0;
  _lagged_source = -rho_d * s_ti;

  // The expansion and the phase change both multiply the interfacial area concentration but carry
  // no sign of their own, so each is split: the part that strengthens the diagonal is taken
  // implicitly and the part that would weaken it is evaluated at the previous iterate. The two
  // agree at convergence.
  auto add_signed = [this, xi](const Real coefficient)
  {
    _implicit_coefficient += std::max(coefficient, 0.0);
    // _lagged_source carries the left hand side form, which computeRightHandSideContribution()
    // negates, so the part evaluated at the previous iterate enters it with the sign it has on the
    // left rather than the sign it will end up with on the right
    _lagged_source += std::min(coefficient, 0.0) * xi;
  };

  add_signed(-material_time_derivative_rho_d / 3.0);

  // The phase change term divides by the phase fraction, so it is dropped below this floor: the
  // fraction at which a millimetre-scale particle population is already below one particle per
  // cubic metre.
  constexpr Real minimum_transfer_fraction = 1e-10;
  if (f_d > minimum_transfer_fraction)
    add_signed(-2.0 / 3.0 * _mass_transfer_rate(elem_arg, state) / f_d);

  // The coalescence sinks are non-positive, so dividing by the previous iterate always
  // strengthens the diagonal and needs no such split
  if (has_interface)
    _implicit_coefficient -= rho_d * (s_rc + s_we) / xi;
}

void
LinearWCNSFV2PInterfaceAreaSourceSink::setCurrentElemInfo(const ElemInfo * elem_info)
{
  LinearFVElementalKernel::setCurrentElemInfo(elem_info);
  computeCoefficients();
}

Real
LinearWCNSFV2PInterfaceAreaSourceSink::computeMatrixContribution()
{
  return _implicit_coefficient * _current_elem_volume;
}

Real
LinearWCNSFV2PInterfaceAreaSourceSink::computeRightHandSideContribution()
{
  // The kernel contributes to the left hand side, so the part which does not multiply the solution
  // moves to the right hand side with the opposite sign
  return -_lagged_source * _current_elem_volume;
}
