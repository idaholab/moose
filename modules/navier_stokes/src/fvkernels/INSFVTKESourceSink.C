//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTKESourceSink.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", INSFVTKESourceSink);

InputParameters
INSFVTKESourceSink::validParams()
{
  InputParameters params = FVElementalKernel::validParams();
  params.addClassDescription("Elental kernel to compute the production and destruction "
                             " terms of turbulent kinetic energy (TKE).");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("epsilon",
                                            "Coupled turbulent kinetic energy dissipation rate.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "fluid density");
  params.addRequiredParam<MooseFunctorName>("mu_t", "Turbulent viscosity.");
  params.addParam<Real>("max_mixing_length",
                        10.0,
                        "Maximum mixing legth allowed for the domain - adjust for realizable "
                        "k-epsilon to work properly.");
  params.addParam<bool>(
      "linearized_model", false, "Boolean to determine if the problem is linearized.");
  params.addParam<MooseFunctorName>(
      "linear_variable", 1.0, "Linearization coefficient in case the problem has been linearized.");
  params.addParam<bool>(
      "realizable_constraint",
      true,
      "Boolean to determine if the kEpsilon mixing length realizability constrints are applied.");
  params.set<unsigned short>("ghost_layers") = 2;
  params.addParam<Real>("rf", 1.0, "Relaxation factor.");
  return params;
}

INSFVTKESourceSink::INSFVTKESourceSink(const InputParameters & params)
  : FVElementalKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _epsilon(getFunctor<ADReal>("epsilon")),
    _rho(getFunctor<ADReal>(NS::density)),
    _mu_t(getFunctor<ADReal>("mu_t")),
    _max_mixing_length(getParam<Real>("max_mixing_length")),
    _linearized_model(getParam<bool>("linearized_model")),
    _linear_variable(getFunctor<ADReal>("linear_variable")),
    _realizable_constraint(getParam<bool>("realizable_constraint")),
    _rf(getParam<Real>("rf"))
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
INSFVTKESourceSink::computeQpResidual()
{

  ADReal residual = 0.0;

  constexpr Real offset = 0.0; // prevents explosion of sqrt(x) derivative to infinity

  const auto & grad_u = _u_var->adGradSln(_current_elem);
  auto Sij_00 = 0.5 * (grad_u(0) + grad_u(0));
  ADReal symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(Sij_00);
  if (_dim >= 2)
  {
    const auto & grad_v = _v_var->adGradSln(_current_elem);
    auto Sij_01 = 0.5 * (grad_u(1) + grad_v(0));
    auto Sij_10 = Sij_01;
    auto Sij_11 = 0.5 * (grad_v(1) + grad_u(1));
    symmetric_strain_tensor_norm +=
        2.0 * (Utility::pow<2>(Sij_01) + Utility::pow<2>(Sij_10) + Utility::pow<2>(Sij_11));
    if (_dim >= 3)
    {
      const auto & grad_w = _w_var->adGradSln(_current_elem);
      auto Sij_02 = 0.5 * (grad_u(2) + grad_w(0));
      auto Sij_20 = Sij_02;
      auto Sij_12 = 0.5 * (grad_v(2) + grad_w(1));
      auto Sij_21 = Sij_12;
      auto Sij_22 = 0.5 * (grad_w(2) + grad_w(2));
      symmetric_strain_tensor_norm +=
          (Utility::pow<2>(Sij_02) + Utility::pow<2>(Sij_20) + Utility::pow<2>(Sij_12) +
           Utility::pow<2>(Sij_21) + Utility::pow<2>(Sij_22));
    }
  }

  const auto & grad_u_old = _u_var->adGradSln(_current_elem, 1);
  Sij_00 = 0.5 * (grad_u_old(0) + grad_u_old(0));
  ADReal symmetric_strain_tensor_norm_old = 2.0 * Utility::pow<2>(Sij_00);
  if (_dim >= 2)
  {
    const auto & grad_v_old = _v_var->adGradSln(_current_elem);
    auto Sij_01 = 0.5 * (grad_u_old(1) + grad_v_old(0));
    auto Sij_10 = Sij_01;
    auto Sij_11 = 0.5 * (grad_v_old(1) + grad_u_old(1));
    symmetric_strain_tensor_norm_old +=
        2.0 * (Utility::pow<2>(Sij_01) + Utility::pow<2>(Sij_10) + Utility::pow<2>(Sij_11));
    if (_dim >= 3)
    {
      const auto & grad_w_old = _w_var->adGradSln(_current_elem);
      auto Sij_02 = 0.5 * (grad_u_old(2) + grad_w_old(0));
      auto Sij_20 = Sij_02;
      auto Sij_12 = 0.5 * (grad_v_old(2) + grad_w_old(1));
      auto Sij_21 = Sij_12;
      auto Sij_22 = 0.5 * (grad_w_old(2) + grad_w_old(2));
      symmetric_strain_tensor_norm_old +=
          (Utility::pow<2>(Sij_02) + Utility::pow<2>(Sij_20) + Utility::pow<2>(Sij_12) +
           Utility::pow<2>(Sij_21) + Utility::pow<2>(Sij_22));
    }
  }

  symmetric_strain_tensor_norm = 2.0 * symmetric_strain_tensor_norm + offset;
  symmetric_strain_tensor_norm_old = 2.0 * symmetric_strain_tensor_norm_old + offset;

  auto production = _mu_t(makeElemArg(_current_elem)) * symmetric_strain_tensor_norm.value();
  auto production_old =
      _mu_t(makeElemArg(_current_elem), 1) * symmetric_strain_tensor_norm_old.value();

  ADReal destruction, destruction_old;
  if (_linearized_model)
    destruction = _rho(makeElemArg(_current_elem)) * _linear_variable(makeElemArg(_current_elem)) *
                  _var(makeElemArg(_current_elem));
  else
  {
    destruction = _rho(makeElemArg(_current_elem)) * _epsilon(makeElemArg(_current_elem));
    destruction_old = _rho(makeElemArg(_current_elem), 1) * _epsilon(makeElemArg(_current_elem), 1);
  }

  production = _rf * production + (1.0 - _rf) * production_old;
  destruction = _rf * destruction + (1.0 - _rf) * destruction_old;

  // if (_realizable_constraint)
  // {
  //   // Ralizable constraints
  //   production = (production > 0) ? production : 0.0;
  //   destruction = (destruction > 0) ? destruction : 0.0;
  //   destruction = (destruction < production) ? destruction : production;
  // }

  residual = destruction - production;

  // _console << "Production k: " << production << " Destruction k: " << destruction << std::endl;

  return residual;
}
