//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFrictionFactorFunctorMaterial.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", LinearFrictionFactorFunctorMaterial);

InputParameters
LinearFrictionFactorFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Material class used to compute a friction factor of the form A * "
      "f(r, t) + B * g(r, t) * |v_I| with A, B vector constants, f(r, t) and g(r, t) "
      "functors of space and time, and |v_I| the interstitial speed");

  params.addRequiredParam<MooseFunctorName>("functor_name",
                                            "The name of functor storing the friction factor");
  params.addRequiredParam<MooseFunctorName>(NS::porosity, "porosity");
  params.addParam<MooseFunctorName>(
      NS::superficial_velocity_x, 0, "The x component of the fluid superficial velocity variable.");
  params.addParam<MooseFunctorName>(
      NS::superficial_velocity_y, 0, "The y component of the fluid superficial velocity variable.");
  params.addParam<MooseFunctorName>(
      NS::superficial_velocity_z, 0, "The z component of the fluid superficial velocity variable.");
  params.addParam<RealVectorValue>(
      "A", RealVectorValue(1, 1, 1), "Coefficient of the A * f(t) term");
  params.addParam<RealVectorValue>(
      "B", RealVectorValue(), "Coefficient of the B * g(t) * |v_I| term");
  params.addParam<MooseFunctorName>("f", "Functor f in the A * f(t) term");
  params.addParam<MooseFunctorName>("g", "Functor g in the B * g(t) * |v_I| term");
  return params;
}

LinearFrictionFactorFunctorMaterial::LinearFrictionFactorFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _functor_name(getParam<MooseFunctorName>("functor_name")),
    _A(getParam<RealVectorValue>("A")),
    _B(getParam<RealVectorValue>("B")),
    _f(getFunctor<ADReal>("f")),
    _g(getFunctor<ADReal>("g")),
    _eps(getFunctor<ADReal>(NS::porosity)),
    _superficial_vel_x(getFunctor<ADReal>(NS::superficial_velocity_x)),
    _superficial_vel_y(getFunctor<ADReal>(NS::superficial_velocity_y)),
    _superficial_vel_z(getFunctor<ADReal>(NS::superficial_velocity_z))
{
  // Check dimension of the mesh
  const unsigned int num_components_specified =
      parameters.isParamSetByUser(NS::superficial_velocity_x) +
      parameters.isParamSetByUser(NS::superficial_velocity_y) +
      parameters.isParamSetByUser(NS::superficial_velocity_z);
  if (num_components_specified != blocksMaxDimension())
    mooseError("Only ",
               num_components_specified,
               " superficial velocity components were provided for a mesh of dimension ",
               blocksMaxDimension());

  addFunctorProperty<ADRealVectorValue>(
      _functor_name,
      [this](const auto & r, const auto & t) -> ADRealVectorValue
      {
        // Compute speed
        // see PINSFVSpeedFunctorMaterial.C for explanation
        const ADRealVectorValue superficial_vel(
            _superficial_vel_x(r, t), _superficial_vel_y(r, t), _superficial_vel_z(r, t));
        const auto speed = NS::computeSpeed(superficial_vel) / _eps(r, t);

        return _A * _f(r, t) + _B * _g(r, t) * speed;
      });
}
