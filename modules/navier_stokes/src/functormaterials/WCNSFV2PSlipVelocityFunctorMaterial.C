//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFV2PSlipVelocityFunctorMaterial.h"
#include "INSFVVelocityVariable.h"
#include "MooseLinearVariableFV.h"
#include "Function.h"
#include "NS.h"
#include "NavierStokesMethods.h"
#include "NavierStokesMethods.h"
#include "FVKernel.h"

registerMooseObject("NavierStokesApp", WCNSFV2PSlipVelocityFunctorMaterial);

InputParameters
WCNSFV2PSlipVelocityFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the slip velocity for two-phase mixture model.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(
      NS::density,
      "Mixture density, which carries the buoyancy factor (rho_d - rho_m) / rho_d of the closure.");
  params.addRequiredParam<MooseFunctorName>("rho_d", "Dispersed phase density.");
  params.addRequiredParam<MooseFunctorName>(
      NS::mu,
      "Continuous phase dynamic viscosity, which carries the particle relaxation time; Stokes "
      "drag is exerted by the fluid the particle moves through.");
  params.addParam<bool>(
      "use_dispersed_phase_drag_model",
      false,
      "Whether to close the drag with the Schiller and Naumann correlation, solved together with "
      "the force balance so that its particle Reynolds number is formed from the slip velocity. "
      "Replaces 'linear_coef_name'.");
  params.addParam<MooseFunctorName>(
      "rho_c", "Continuous phase density, needed by the drag model for its Reynolds number.");
  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "Gravity acceleration vector");
  params.addParam<Real>("force_value", 0.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("force_function", "0", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "force_postprocessor", 0, "A postprocessor whose value is multiplied by the body force");
  params.addParam<RealVectorValue>(
      "force_direction", RealVectorValue(1, 0, 0), "Gravitational acceleration vector");
  params.addParam<MooseFunctorName>(
      "linear_coef_name", 0.44, "Linear friction coefficient name as a material property");
  params.addParam<MooseFunctorName>(
      "particle_diameter", 1.0, "Diameter of particles in the dispersed phase.");
  params.addParam<MooseFunctorName>("fd", 0.0, "Fraction dispersed phase.");
  params.addParam<MooseFunctorName>(
      "drift_velocity_name",
      "Name of the diffusion (drift) velocity property to declare, the velocity of the dispersed "
      "phase relative to the centre of mass of the mixture, (1 - c_d) u_slip with c_d the "
      "dispersed phase mass fraction. Declared only when supplied.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  params.addRequiredParam<MooseFunctorName>("slip_velocity_name", "the name of the slip velocity");
  params.addParam<unsigned short>("ghost_layers",
                                  3,
                                  "The number of layers of elements to ghost. With Rhie-Chow and "
                                  "the velocity gradient calculation below, we need 3");
  params.addRelationshipManager(
      "ElementSideNeighborLayers",
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING,
      [](const InputParameters & obj_params, InputParameters & rm_params)
      {
        rm_params.set<unsigned short>("layers") = obj_params.get<unsigned short>("ghost_layers");
      });
  return params;
}

WCNSFV2PSlipVelocityFunctorMaterial::WCNSFV2PSlipVelocityFunctorMaterial(
    const InputParameters & params)
  : FunctorMaterial(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<MooseVariableField<Real> *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v") ? dynamic_cast<MooseVariableField<Real> *>(getFieldVar("v", 0))
                                    : nullptr),
    _w_var(params.isParamValid("w") ? dynamic_cast<MooseVariableField<Real> *>(getFieldVar("w", 0))
                                    : nullptr),
    _rho_mixture(getFunctor<ADReal>(NS::density)),
    _rho_d(getFunctor<ADReal>("rho_d")),
    _mu_mixture(getFunctor<ADReal>(NS::mu)),
    _gravity(getParam<RealVectorValue>("gravity")),
    _force_scale(getParam<Real>("force_value")),
    _force_function(getFunction("force_function")),
    _force_postprocessor(getPostprocessorValue("force_postprocessor")),
    _force_direction(getParam<RealVectorValue>("force_direction")),
    _linear_friction(getFunctor<ADReal>("linear_coef_name")),
    _use_drag_model(getParam<bool>("use_dispersed_phase_drag_model")),
    _rho_c(_use_drag_model ? &getFunctor<ADReal>("rho_c") : nullptr),
    _particle_diameter(getFunctor<ADReal>("particle_diameter")),
    _f_d(getFunctor<ADReal>("fd")),
    _index(getParam<MooseEnum>("momentum_component"))
{
  if (_use_drag_model && !isParamValid("rho_c"))
    paramError("rho_c", "The drag model needs the continuous phase density.");
  if (_use_drag_model && isParamSetByUser("linear_coef_name"))
    paramError("linear_coef_name",
               "A prescribed friction factor cannot be combined with the drag model, which is "
               "solved inside this closure rather than supplied to it.");

  if (!dynamic_cast<const INSFVVelocityVariable *>(_u_var) &&
      !dynamic_cast<const MooseLinearVariableFV<Real> *>(_u_var))
    paramError("u",
               "the u velocity must be an INSFVVelocityVariable or a MooseLinearVariableFVReal");

  if (_dim >= 2 && (!dynamic_cast<const INSFVVelocityVariable *>(_v_var) &&
                    !dynamic_cast<const MooseLinearVariableFV<Real> *>(_v_var)))
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable or a MooseLinearVariableFVReal.");

  if (_dim >= 3 && (!dynamic_cast<const INSFVVelocityVariable *>(_w_var) &&
                    !dynamic_cast<const MooseLinearVariableFV<Real> *>(_w_var)))
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable or a MooseLinearVariableFVReal.");

  // Slip velocity advection term requires gradients
  // TODO: this could be set less often, keeping it false until the two phase mixture system is
  // solved
  if (auto u = dynamic_cast<MooseLinearVariableFV<Real> *>(_u_var))
    u->requestCellGradients();
  if (auto v = dynamic_cast<MooseLinearVariableFV<Real> *>(_v_var))
    v->requestCellGradients();
  if (auto w = dynamic_cast<MooseLinearVariableFV<Real> *>(_w_var))
    w->requestCellGradients();

  const auto & slip_velocity = addFunctorProperty<ADReal>(
      getParam<MooseFunctorName>("slip_velocity_name"),
      [this](const auto & r, const auto & t)
      {
        constexpr Real offset = 1e-15;

        const bool is_transient = _subproblem.isTransient();
        ADRealVectorValue term_advection(0, 0, 0);
        ADRealVectorValue term_transient(0, 0, 0);
        const ADRealVectorValue term_force(
            _force_scale * _force_postprocessor *
            _force_function.value(_t, _current_elem->vertex_average()) * _force_direction);

        // Adding transient term
        // TODO: add time derivative term to lienar FV variable
        if (is_transient && !dynamic_cast<MooseLinearVariableFV<Real> *>(_u_var))
        {
          term_transient(0) += _u_var->dot(r, t);
          if (_dim > 1)
            term_transient(1) += _v_var->dot(r, t);
          if (_dim > 2)
            term_transient(2) += _w_var->dot(r, t);
        }

        // Adding advection term
        const auto u_velocity = (*_u_var)(r, t);
        const auto u_grad = _u_var->gradient(r, t);
        term_advection(0) += u_velocity * u_grad(0);
        if (_dim > 1)
        {
          const auto v_velocity = (*_v_var)(r, t);
          const auto v_grad = _v_var->gradient(r, t);
          term_advection(0) += v_velocity * u_grad(1);
          term_advection(1) += u_velocity * v_grad(0) + v_velocity * v_grad(1);
          if (_dim > 2)
          {
            const auto w_velocity = (*_w_var)(r, t);
            const auto w_grad = _w_var->gradient(r, t);
            term_advection(0) += w_velocity * u_grad(2);
            term_advection(1) += w_velocity * v_grad(2);
            term_advection(2) +=
                u_velocity * w_grad(0) + v_velocity * w_grad(1) + w_velocity * w_grad(2);
          }
        }

        const ADReal density_scaling = (_rho_d(r, t) - _rho_mixture(r, t)) / _rho_d(r, t);
        const ADRealVectorValue acceleration_vec =
            -term_transient - term_advection + _gravity + term_force;

        const ADReal relaxation_time =
            _rho_d(r, t) * Utility::pow<2>(_particle_diameter(r, t)) / (18.0 * _mu_mixture(r, t));

        // The slip in the Stokes limit, f_drag = 1, which is also the whole answer when the drag
        // function is prescribed rather than computed
        const ADReal stokes_prefactor = relaxation_time * density_scaling;

        if (!_use_drag_model)
          return stokes_prefactor / (_linear_friction(r, t) + offset) * acceleration_vec(_index);

        // Otherwise solve the force balance and the drag correlation together, so that the
        // particle Reynolds number is formed from the slip velocity as its definition requires.
        // The magnitude comes from the solve, the direction from the acceleration.
        using std::abs;
        const ADReal stokes_speed = abs(stokes_prefactor) * acceleration_vec.norm();
        const ADReal reynolds_per_speed = NS::particleReynoldsNumber(
            (*_rho_c)(r, t), _particle_diameter(r, t), ADReal(1.0), _mu_mixture(r, t));
        const ADReal slip_speed = NS::solveSlipSpeed(stokes_speed, reynolds_per_speed);
        const ADReal f_drag =
            (MetaPhysicL::raw_value(slip_speed) > 0.0) ? stokes_speed / slip_speed : ADReal(1.0);
        return stokes_prefactor / f_drag * acceleration_vec(_index);
      });

  // The diffusion (drift) velocity, the velocity of the dispersed phase relative to the centre of
  // mass of the mixture. This, not the slip velocity, is the velocity that appears in the phase
  // conservation equations, see VTT Publications 288 equation (28). The factor is written as
  // (rho_m - alpha rho_d) / rho_m so that only quantities this object already has are needed.
  if (isParamValid("drift_velocity_name"))
    addFunctorProperty<ADReal>(
        getParam<MooseFunctorName>("drift_velocity_name"),
        [this, &slip_velocity](const auto & r, const auto & t) -> ADReal
        {
          const auto rho_m = _rho_mixture(r, t);
          if (MetaPhysicL::raw_value(rho_m) <= 0.0)
            return slip_velocity(r, t);
          const auto fd_raw = _f_d(r, t);
          const ADReal fd = (fd_raw < 0.0) ? ADReal(0.0) : (fd_raw > 1.0) ? ADReal(1.0) : fd_raw;
          return (rho_m - fd * _rho_d(r, t)) / rho_m * slip_velocity(r, t);
        });
}
