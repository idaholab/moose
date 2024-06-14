//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVDispersePhaseDragFunctorMaterial.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", NSFVDispersePhaseDragFunctorMaterial);

InputParameters
NSFVDispersePhaseDragFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes drag coefficient for dispersed phase.");
  params.addParam<MooseFunctorName>("drag_coef_name",
                                    "Darcy_coefficient",
                                    "Name of the scalar friction coefficient defined. The vector "
                                    "coefficient is suffixed with _vec");
  params.addRequiredParam<MooseFunctorName>("u", "The velocity in the x direction.");
  params.addParam<MooseFunctorName>("v", "The velocity in the y direction.");
  params.addParam<MooseFunctorName>("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Continuous phase density.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Mixture Density");
  params.addParam<MooseFunctorName>(
      "particle_diameter", 1.0, "Diameter of particles in the dispersed phase.");
  return params;
}

NSFVDispersePhaseDragFunctorMaterial::NSFVDispersePhaseDragFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _dim(_subproblem.mesh().dimension()),
    _u_var(getFunctor<ADReal>("u")),
    _v_var(parameters.isParamValid("v") ? &(getFunctor<ADReal>("v")) : nullptr),
    _w_var(parameters.isParamValid("w") ? &(getFunctor<ADReal>("w")) : nullptr),
    _rho_mixture(getFunctor<ADReal>(NS::density)),
    _mu_mixture(getFunctor<ADReal>(NS::mu)),
    _particle_diameter(getFunctor<ADReal>("particle_diameter"))
{
  if (_dim >= 2 && !_v_var)
    paramError("v",
               "In two or more dimensions, the v velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  if (_dim >= 3 && !_w_var)
    paramError("w",
               "In three-dimensions, the w velocity must be supplied and it must be an "
               "INSFVVelocityVariable.");

  const auto f = [this](const auto & r, const auto & t) -> ADReal
  {
    ADRealVectorValue velocity(_u_var(r, t));
    if (_dim > 1)
      velocity(1) = (*_v_var)(r, t);
    if (_dim > 2)
      velocity(2) = (*_w_var)(r, t);
    const auto speed = NS::computeSpeed(velocity);

    const auto Re_particle =
        _particle_diameter(r, t) * speed * _rho_mixture(r, t) / _mu_mixture(r, t);

    if (Re_particle <= 1000)
    {
      if (MetaPhysicL::raw_value(Re_particle) < 0)
        mooseException("Cannot take a non-integer power of a negative number");
      return 1.0 + 0.15 * std::pow(Re_particle, 0.687);
    }
    else
      return 0.0183 * Re_particle;
  };
  const auto & f_func = addFunctorProperty<ADReal>(getParam<MooseFunctorName>("drag_coef_name"), f);

  // Define the vector friction coefficient
  const auto f_vec = [&f_func](const auto & r, const auto & t) -> ADRealVectorValue
  {
    const auto f_value = f_func(r, t);
    return ADRealVectorValue(f_value, f_value, f_value);
  };
  addFunctorProperty<ADRealVectorValue>(getParam<MooseFunctorName>("drag_coef_name") + "_vec",
                                        f_vec);
}
