//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTFSourceSink.h"
#include "NS.h"
#include "NonlinearSystemBase.h"
#include "NavierStokesMethods.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("NavierStokesApp", INSFVTFSourceSink);

InputParameters
INSFVTFSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  // Description
  params.addClassDescription(
      "Elemental kernel to compute the production and destruction for the v2f function.");

  // Coupled velocity fields
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");

  // Couupled turbulent variables
  params.addRequiredParam<MooseFunctorName>(NS::TKE, "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::TKED,
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::TV2, "Coupled turbulent wall normal fluctuations.");

  // Coupled thermophysical properties
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Dynamic viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::mu_t, "Turbulent viscosity.");

  // Coupled model coefficients
  params.addParam<Real>("C1", 1.4, "First relaxation function coefficient.");
  params.addParam<Real>("C2", 0.3, "Second relaxation function coefficient.");
  params.addParam<Real>("n", 6.0, "Model parameter.");
  params.addParam<Real>("C_eta", 70.0, "Kolmogorov scale limiter parameter.");

  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

INSFVTFSourceSink::INSFVTFSourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(params.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(params.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _k(getFunctor<ADReal>(NS::TKE)),
    _epsilon(getFunctor<ADReal>(NS::TKED)),
    _v2(getFunctor<ADReal>(NS::TV2)),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu(getFunctor<ADReal>(NS::mu)),
    _mu_t(getFunctor<ADReal>(NS::mu_t)),
    _C1(getParam<Real>("C1")),
    _C2(getParam<Real>("C2")),
    _n(getParam<Real>("n")),
    _C_eta(getParam<Real>("C_eta"))
{
}

ADReal
INSFVTFSourceSink::computeQpResidual()
{
  // Convenient variables
  const auto elem_arg = makeElemArg(_current_elem);
  const auto state = determineState();
  const auto mu = _mu(elem_arg, state);
  const auto rho = _rho(elem_arg, state);
  const auto nu = mu / rho;
  const auto TKE = _k(elem_arg, state);
  const auto TKED = _epsilon(elem_arg, state);
  const auto TV2 = _v2(elem_arg, state);

  // Computing turbulent production
  TensorValue<ADReal> Sij;
  const auto & grad_u = _u_var.gradient(elem_arg, state);
  auto trace = grad_u(0) / 3.0;

  if (_dim == 1)
    Sij = TensorValue<ADReal>(grad_u(0) - trace, 0, 0, 0, 0, 0, 0, 0, 0);

  if (_dim >= 2)
  {
    const auto & grad_v = (*_v_var).gradient(elem_arg, state);
    trace += grad_v(1) / 3.0;

    if (_dim == 2)
      Sij = TensorValue<ADReal>(grad_u(0) - trace,
                                0.5 * (grad_u(1) + grad_v(0)),
                                0,
                                0.5 * (grad_v(0) + grad_u(1)),
                                grad_v(1) - trace,
                                0,
                                0,
                                0,
                                0);

    if (_dim >= 3)
    {
      const auto & grad_w = (*_w_var).gradient(elem_arg, state);
      trace += grad_w(2) / 3.0;

      Sij = TensorValue<ADReal>(grad_u(0) - trace,
                                0.5 * (grad_u(1) + grad_v(0)),
                                0.5 * (grad_u(2) + grad_w(0)),
                                0.5 * (grad_v(0) + grad_u(1)),
                                grad_v(1) - trace,
                                0.5 * (grad_v(2) + grad_w(1)),
                                0.5 * (grad_w(0) + grad_u(2)),
                                0.5 * (grad_w(1) + grad_v(2)),
                                grad_w(2) - trace);
    }
  }

  const auto symmetric_strain_tensor_sq_norm = 2.0 * Sij.contract(Sij);
  const auto production = _mu_t(elem_arg, state) * symmetric_strain_tensor_sq_norm / rho;

  // Computing turbulent surrogate variables
  const auto time_scale = std::max(TKE / TKED, 6 * std::sqrt(nu / TKED));
  const auto bulk_scale = std::pow(TKE, 1.5) / TKED;
  const auto kolmogorov_scale = _C_eta * std::pow(std::pow(nu, 3) / TKED, 0.25);
  const auto L2 = Utility::pow<2>(0.23 * std::max(bulk_scale, kolmogorov_scale));

  // Computing production and destruction
  const auto v2fdiffusivity = ((_C1 - _n) * TV2 - 2.0 / 3.0 * TKE * (_C1 - 1.0)) / time_scale;
  const auto fluctuation_function = (v2fdiffusivity - _C2 * production) / TKE;
  const auto implicit_term = _var(elem_arg, state);

  return (implicit_term + fluctuation_function) / L2;
}
