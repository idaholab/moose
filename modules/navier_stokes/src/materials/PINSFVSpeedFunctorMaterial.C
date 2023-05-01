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
  params.addParam<MooseFunctorName>(NS::T_fluid, "The fluid temperature variable.");

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
  addFunctorProperty<ADRealVectorValue>(NS::velocity,
                                        [this](const auto & r, const auto & t) -> ADRealVectorValue
                                        {
                                          return ADRealVectorValue(_superficial_vel_x(r, t),
                                                                   _superficial_vel_y(r, t),
                                                                   _superficial_vel_z(r, t)) /
                                                 _eps(r, t);
                                        });

  // Speed is normal of regular interstitial velocity
  // This is needed to compute the Reynolds number
  addFunctorProperty<ADReal>(NS::speed,
                             [this](const auto & r, const auto & t) -> ADReal
                             {
                               // if the velocity is zero, then the norm function call fails because
                               // AD tries to calculate the derivatives which causes a divide by
                               // zero - because d/dx(sqrt(f(x))) = 1/2/sqrt(f(x))*df/dx. So add a
                               // bit of noise to avoid this failure mode.
                               if ((MooseUtils::absoluteFuzzyEqual(_superficial_vel_x(r, t), 0)) &&
                                   (MooseUtils::absoluteFuzzyEqual(_superficial_vel_y(r, t), 0)) &&
                                   (MooseUtils::absoluteFuzzyEqual(_superficial_vel_z(r, t), 0)))
                                 return 1e-42;

                               return ADRealVectorValue(_superficial_vel_x(r, t),
                                                        _superficial_vel_y(r, t),
                                                        _superficial_vel_z(r, t))
                                          .norm() /
                                      _eps(r, t);
                             });
}
