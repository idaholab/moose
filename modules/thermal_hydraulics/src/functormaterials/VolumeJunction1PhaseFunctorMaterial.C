//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseFunctorMaterial.h"
#include "THMNames.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseFunctorMaterial);

InputParameters
VolumeJunction1PhaseFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();

  params.addClassDescription("Computes several quantities for VolumeJunction1Phase.");

  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The SinglePhaseFluidProperties object for the fluid");

  return params;
}

VolumeJunction1PhaseFunctorMaterial::VolumeJunction1PhaseFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _rhoV(getFunctor<ADReal>("rhoV")),
    _rhouV(getFunctor<ADReal>("rhouV")),
    _rhovV(getFunctor<ADReal>("rhovV")),
    _rhowV(getFunctor<ADReal>("rhowV")),
    _rhoEV(getFunctor<ADReal>("rhoEV")),
    _V(getParam<Real>("volume")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
  addPressureFunctorProperty();
  addTemperatureFunctorProperty();
  addVelocityComponentFunctorProperty("vel_x", _rhouV);
  addVelocityComponentFunctorProperty("vel_y", _rhovV);
  addVelocityComponentFunctorProperty("vel_z", _rhowV);
}

void
VolumeJunction1PhaseFunctorMaterial::addPressureFunctorProperty()
{
  addFunctorProperty<Real>(THM::functorMaterialPropertyName<false>("p"),
                           [this](const auto & r, const auto & t) -> Real
                           {
                             Real v, e;
                             computeSpecificVolumeAndInternalEnergy(r, t, v, e);
                             return _fp.p_from_v_e(v, e);
                           });
}

void
VolumeJunction1PhaseFunctorMaterial::addTemperatureFunctorProperty()
{
  addFunctorProperty<Real>(THM::functorMaterialPropertyName<false>("T"),
                           [this](const auto & r, const auto & t) -> Real
                           {
                             Real v, e;
                             computeSpecificVolumeAndInternalEnergy(r, t, v, e);
                             return _fp.T_from_v_e(v, e);
                           });
}

void
VolumeJunction1PhaseFunctorMaterial::addVelocityComponentFunctorProperty(
    const std::string & vel_name, const Moose::Functor<ADReal> & rhouV_i)
{
  addFunctorProperty<Real>(THM::functorMaterialPropertyName<false>(vel_name),
                           [this, &rhouV_i](const auto & r, const auto & t) -> Real
                           { return rhouV_i(r, t).value() / _rhoV(r, t).value(); });
}

template <typename SpaceArg, typename StateArg>
void
VolumeJunction1PhaseFunctorMaterial::computeSpecificVolumeAndInternalEnergy(const SpaceArg & r,
                                                                            const StateArg & t,
                                                                            Real & v,
                                                                            Real & e) const
{
  const Real rhoV = _rhoV(r, t).value();
  const Real rhouV = _rhouV(r, t).value();
  const Real rhovV = _rhovV(r, t).value();
  const Real rhowV = _rhowV(r, t).value();
  const Real rhoEV = _rhoEV(r, t).value();

  const Real rho = rhoV / _V;
  v = 1.0 / rho;
  const Real vel_x = rhouV / rhoV;
  const Real vel_y = rhovV / rhoV;
  const Real vel_z = rhowV / rhoV;
  const RealVectorValue vel(vel_x, vel_y, vel_z);
  const Real E = rhoEV / rhoV;
  e = E - 0.5 * vel * vel;
}
