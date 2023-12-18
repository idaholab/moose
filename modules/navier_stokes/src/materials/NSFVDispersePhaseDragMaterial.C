//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVDispersePhaseDragMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVDispersePhaseDragMaterial);

InputParameters
NSFVDispersePhaseDragMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes drag coefficient for dispersed pahse.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>(NS::density, "Continuous phase density.");
  params.addRequiredParam<MooseFunctorName>(NS::mu, "Mixture Density");
  params.addParam<MooseFunctorName>(
      "particle_diameter", 1.0, "Diameter of particles in the dispersed phase.");
  return params;
}

NSFVDispersePhaseDragMaterial::NSFVDispersePhaseDragMaterial(const InputParameters & parameters)
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

  addFunctorProperty<ADReal>("Darcy_coefficient",
                             [this](const auto & r, const auto & t) -> ADReal
                             {
                               ADReal speed = Utility::pow<2>(_u_var(r, t));
                               if (_dim > 1)
                                 speed += Utility::pow<2>((*_v_var)(r, t));
                               if (_dim > 2)
                                 speed += Utility::pow<2>((*_w_var)(r, t));
                               speed = std::sqrt(speed);

                               ADReal Re_particle = _particle_diameter(r, t) * speed *
                                                    _rho_mixture(r, t) / _mu_mixture(r, t);

                               if (Re_particle <= 1000)
                                 return 1.0 + 0.15 * std::pow(Re_particle, 0.687);
                               else
                                 return 0.0183 * Re_particle;
                             });
}
