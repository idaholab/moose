//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVSpeedFunctorMaterial.h"
#include "NS.h"
#include "NavierStokesMethods.h"

registerMooseObject("NavierStokesApp", PINSFVSpeedFunctorMaterial);

InputParameters
PINSFVSpeedFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription("This is the material class used to compute the interstitial velocity"
                             " norm for the incompressible and weakly compressible primitive "
                             "superficial finite-volume implementation of porous media equations.");

  params.addRequiredParam<MooseFunctorName>(NS::porosity, "porosity");
  params.addParam<MooseFunctorName>(
      NS::superficial_velocity_x, 0, "The x component of the fluid superficial velocity variable.");
  params.addParam<MooseFunctorName>(
      NS::superficial_velocity_y, 0, "The y component of the fluid superficial velocity variable.");
  params.addParam<MooseFunctorName>(
      NS::superficial_velocity_z, 0, "The z component of the fluid superficial velocity variable.");
  auto add_property = [&params](const auto & property_name)
  {
    params.addParam<MooseFunctorName>(property_name,
                                      property_name,
                                      "The name to give the declared '" + property_name +
                                          "' functor property");
  };
  add_property(NS::velocity);
  add_property(NS::speed);
  add_property(NS::velocity_x);
  add_property(NS::velocity_y);
  add_property(NS::velocity_z);
  params.addParam<bool>("define_interstitial_velocity_components",
                        true,
                        "Whether to define the interstitial velocity functors");

  return params;
}

PINSFVSpeedFunctorMaterial::PINSFVSpeedFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
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

  // Interstitial velocity is needed by certain correlations
  const auto & interstitial_velocity = addFunctorProperty<ADRealVectorValue>(
      getParam<MooseFunctorName>(NS::velocity),
      [this](const auto & r, const auto & t) -> ADRealVectorValue
      {
        return ADRealVectorValue(
                   _superficial_vel_x(r, t), _superficial_vel_y(r, t), _superficial_vel_z(r, t)) /
               _eps(r, t);
      });

  // Speed is normal of regular interstitial velocity
  // This is needed to compute the Reynolds number
  addFunctorProperty<ADReal>(getParam<MooseFunctorName>(NS::speed),
                             [&interstitial_velocity](const auto & r, const auto & t) -> ADReal
                             { return NS::computeSpeed(interstitial_velocity(r, t)); });

  // This is not needed for non-porous media, but they can use the 'speed' functor for some friction
  // models
  if (getParam<bool>("define_interstitial_velocity_components"))
  {
    addFunctorProperty<ADReal>(getParam<MooseFunctorName>(NS::velocity_x),
                               [&interstitial_velocity](const auto & r, const auto & t) -> ADReal
                               { return interstitial_velocity(r, t)(0); });
    addFunctorProperty<ADReal>(getParam<MooseFunctorName>(NS::velocity_y),
                               [&interstitial_velocity](const auto & r, const auto & t) -> ADReal
                               { return interstitial_velocity(r, t)(1); });
    addFunctorProperty<ADReal>(getParam<MooseFunctorName>(NS::velocity_z),
                               [&interstitial_velocity](const auto & r, const auto & t) -> ADReal
                               { return interstitial_velocity(r, t)(2); });
  }
}
