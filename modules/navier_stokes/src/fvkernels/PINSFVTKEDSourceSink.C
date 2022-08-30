//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVTKEDSourceSink.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", PINSFVTKEDSourceSink);

InputParameters
PINSFVTKEDSourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription(
      "Elental kernel to compute the production and destruction "
      " terms of turbulent kinetic energy (TKE).");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("k", "Coupled turbulent kinetic energy.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>("mu_t", "Turbulent viscosity.");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "Porosity");
  params.addParam<bool>(
      "effective_balance",
      false,
      "Whether the TKE balance should be multiplied by porosity, or whether the provided "
      "diffusivity is an effective diffusivity taking porosity effects into account");
  params.addRequiredParam<MooseFunctorName>("C1_eps", "First epsilon coefficient");
  params.addRequiredParam<MooseFunctorName>("C2_eps", "Second epsilon coefficient");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVTKEDSourceSink::PINSFVTKEDSourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _k(getFunctor<ADReal>("k")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _porosity(getFunctor<ADReal>(NS::porosity)),
    _porosity_factored_in(getParam<bool>("effective_balance")),
    _C1_eps(getFunctor<ADReal>("C1_eps")),
    _C2_eps(getFunctor<ADReal>("C2_eps"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif

  if (!_u_var)
    paramError("u", "the u velocity must be an INSFVVelocityVariable.");

  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");
}

ADReal
PINSFVTKEDSourceSink::computeQpResidual()
{
#ifdef MOOSE_GLOBAL_AD_INDEXING

  ADReal residual = 0.0;

  constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity

  const auto & grad_u = _u_var->adGradSln(_current_elem);
  ADReal symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(grad_u(0));
  if (_dim >= 2)
  {
    const auto & grad_v = _v_var->adGradSln(_current_elem);
    symmetric_strain_tensor_norm +=
        2.0 * Utility::pow<2>(grad_v(1)) + Utility::pow<2>(grad_v(0) + grad_u(1));
    if (_dim >= 3)
    {
      const auto & grad_w = _w_var->adGradSln(_current_elem);
      symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(grad_w(2)) +
                                      Utility::pow<2>(grad_u(2) + grad_w(0)) +
                                      Utility::pow<2>(grad_v(2) + grad_w(1));
    }
  }

  symmetric_strain_tensor_norm = std::sqrt(symmetric_strain_tensor_norm + offset);

  constexpr Real protection_k = 0.01;

  auto production = _C1_eps(makeElemArg(_current_elem))
                    * _mu_t(makeElemArg(_current_elem))
                    * symmetric_strain_tensor_norm
                    * _var(makeElemArg(_current_elem))
                    / (_k(makeElemArg(_current_elem)) + protection_k);

  auto destruction = _C2_eps(makeElemArg(_current_elem))
                     * _rho(makeElemArg(_current_elem))
                     * Utility::pow<2>(_var(makeElemArg(_current_elem)))
                     / (_k(makeElemArg(_current_elem)) + protection_k);

  // Limiting - mostly for sanity
  production = (production > 0) ? production : 0.0;
  destruction = (destruction > 0) ? destruction : 0.0;
  production = (production/destruction < 5) ? production : 5*destruction;
  production = (destruction/production < 5) ? destruction : 5*production;


  residual += production - destruction;
  residual = (residual > 100) ? residual : 100;
  residual = (residual < .01) ? residual : .01;

  if (_porosity_factored_in)
    residual *= _porosity(makeElemArg(_current_elem));

  return residual;
  #else
    return 0;

  #endif
}