//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFV2PSlipVelocityFunctorMaterial.h"
#include "INSFVVelocityVariable.h"
#include "Function.h"
#include "NS.h"
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
  params.addRequiredParam<MooseFunctorName>(NS::density, "Continuous phase density.");
  params.addRequiredParam<MooseFunctorName>("rho_d", "Dispersed phase density.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Mixture Density");
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
      [](const InputParameters & obj_params, InputParameters & rm_params) {
        rm_params.set<unsigned short>("layers") = obj_params.get<unsigned short>("ghost_layers");
      });
  return params;
}

WCNSFV2PSlipVelocityFunctorMaterial::WCNSFV2PSlipVelocityFunctorMaterial(
    const InputParameters & params)
  : FunctorMaterial(params),
    _dim(_subproblem.mesh().dimension()),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
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
    _particle_diameter(getFunctor<ADReal>("particle_diameter")),
    _index(getParam<MooseEnum>("momentum_component"))
{
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

  addFunctorProperty<ADReal>(
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
        if (is_transient)
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
        const ADReal flux_residual =
            density_scaling * (-term_transient - term_advection + _gravity + term_force)(_index);

        const ADReal relaxation_time =
            _rho_d(r, t) * Utility::pow<2>(_particle_diameter(r, t)) / (18.0 * _mu_mixture(r, t));

        const ADReal linear_friction_factor = _linear_friction(r, t) + offset;

        return relaxation_time / linear_friction_factor * flux_residual;
      });
}
