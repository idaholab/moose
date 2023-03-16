//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MixingLengthTurbulentViscosityMaterial.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", MixingLengthTurbulentViscosityMaterial);

InputParameters
MixingLengthTurbulentViscosityMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("Computes the material property corresponding to the total viscosity"
                             "comprising the mixing length model turbulent total_viscosity"
                             "and the molecular viscosity.");
  params.addRequiredParam<MooseFunctorName>("u", "The x-velocity");
  params.addParam<MooseFunctorName>("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addParam<MooseFunctorName>("w", 0, "z-velocity"); // only required in 3D
  params.addRequiredParam<MooseFunctorName>("mixing_length", "Turbulent eddy mixing length.");
  params.addRequiredParam<MooseFunctorName>("mu", "The viscosity");
  params.addRequiredParam<MooseFunctorName>("rho", "The value for the density");
  return params;
}

MixingLengthTurbulentViscosityMaterial::MixingLengthTurbulentViscosityMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _mesh_dimension(_mesh.dimension()),
    _u_vel(getFunctor<ADReal>("u")),
    _v_vel(isParamValid("v") ? &getFunctor<ADReal>("v") : nullptr),
    _w_vel(isParamValid("w") ? &getFunctor<ADReal>("v") : nullptr),
    _mixing_len(getFunctor<ADReal>(NS::mixing_length)),
    _mu(getFunctor<ADReal>("mu")),
    _rho(getFunctor<ADReal>("rho"))
{
  addFunctorProperty<ADReal>(
      NS::total_viscosity,
      [this](const auto & r, const auto & t) -> ADReal
      {
        constexpr Real offset = 1e-15; // prevents explosion of sqrt(x) derivative to infinity

        const auto grad_u = _u_vel.gradient(r, t);

        ADReal symmetric_strain_tensor_norm = 2.0 * Utility::pow<2>(grad_u(0));
        if (_mesh_dimension >= 2)
        {
          const auto grad_v = _v_vel->gradient(r, t);

          symmetric_strain_tensor_norm +=
              2.0 * Utility::pow<2>(grad_v(1)) + Utility::pow<2>(grad_v(0) + grad_u(1));
          if (_mesh_dimension >= 3)
          {
            const auto grad_w = _w_vel->gradient(r, t);

            symmetric_strain_tensor_norm += 2.0 * Utility::pow<2>(grad_w(2)) +
                                            Utility::pow<2>(grad_u(2) + grad_w(0)) +
                                            Utility::pow<2>(grad_v(2) + grad_w(1));
          }
        }
        symmetric_strain_tensor_norm = std::sqrt(symmetric_strain_tensor_norm + offset);

        // Return the sum of turbulent viscosity and dynamic viscosity
        return _mu(r, t) +
               _rho(r, t) * symmetric_strain_tensor_norm * Utility::pow<2>(_mixing_len(r, t));
      });
}
