//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncompressibleMomentumSPBase.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

template <bool is_ad>
InputParameters
IncompressibleMomentumSPBaseTempl<is_ad>::validParams()
{
  InputParameters params =
      is_ad ? ADScalarTimeDerivative::validParams() : ODETimeDerivative::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription("Base class for path-integrated incompressible momentum kernels.");
  params.addCoupledVar("temperatures",
                       {},
                       "Fluid temperature in each segment of this component. Takes a "
                       "list of scalar variable names");
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addParam<std::vector<MooseFunctorName>>(
      "areas",
      std::vector<MooseFunctorName>({}),
      "Component flow areas per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "perimeters",
      std::vector<MooseFunctorName>({}),
      "Component flow perimeters per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "lengths",
      std::vector<MooseFunctorName>({}),
      "Component flow lengths per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "alphas",
      std::vector<MooseFunctorName>({}),
      "Component flow angles per segment with respect to horizontal (-pi/2 downward to pi/2 "
      "upward). Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "forms_losses",
      std::vector<MooseFunctorName>({}),
      "Forms loss coefficients per segment. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "pump_pressures",
      std::vector<MooseFunctorName>({}),
      "Pump pressure gains per segment [Pa]. Takes a vector of functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "roughnesses",
      std::vector<MooseFunctorName>({}),
      "Component wall roughnesses per segment [m]. Takes a vector of functors.");
  params.addParam<MooseFunctorName>("g", 9.81, "Gravitational acceleration [m/s]");

  return params;
}

template <bool is_ad>
IncompressibleMomentumSPBaseTempl<is_ad>::IncompressibleMomentumSPBaseTempl(
    const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    _n_temps(ScalarCoupleable::coupledScalarComponents("temperatures")),
    _T(_n_temps),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _Pref(this->template getFunctor<GenericReal<is_ad>>("reference_pressure")),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    _n_segments(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _areas(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _perimeters(this->template getParam<std::vector<MooseFunctorName>>("perimeters").size()),
    _lengths(this->template getParam<std::vector<MooseFunctorName>>("lengths").size()),
    _alphas(this->template getParam<std::vector<MooseFunctorName>>("alphas").size()),
    _forms_losses(this->template getParam<std::vector<MooseFunctorName>>("forms_losses").size()),
    _dPps(this->template getParam<std::vector<MooseFunctorName>>("pump_pressures").size()),
    _roughnesses(this->template getParam<std::vector<MooseFunctorName>>("roughnesses").size()),
    _gravity(this->template getFunctor<GenericReal<is_ad>>("g"))
{
  auto & area_names = MooseBase::getParam<std::vector<MooseFunctorName>>("areas");
  auto & perimeter_names = MooseBase::getParam<std::vector<MooseFunctorName>>("perimeters");
  auto & length_names = MooseBase::getParam<std::vector<MooseFunctorName>>("lengths");
  auto & alpha_names = MooseBase::getParam<std::vector<MooseFunctorName>>("alphas");
  auto & forms_loss_names = MooseBase::getParam<std::vector<MooseFunctorName>>("forms_losses");
  auto & dPp_names = MooseBase::getParam<std::vector<MooseFunctorName>>("pump_pressures");
  auto & roughness_names = MooseBase::getParam<std::vector<MooseFunctorName>>("roughnesses");
  if (_n_segments != area_names.size() || _n_segments != perimeter_names.size() ||
      _n_segments != length_names.size() || _n_segments != alpha_names.size() ||
      _n_segments != forms_loss_names.size() || _n_segments != dPp_names.size() ||
      _n_segments != roughness_names.size() || _n_segments != _n_temps)
  {
    mooseError(
        "Must provide consistent number of segments for each parameter! Including temperatures!");
  }
  for (size_t j = 0; j < _n_segments; ++j)
  {
    _T[j] = &(ScalarCoupleable::coupledScalarValue("temperatures", j));
    _areas[j] = &(this->template getFunctor<GenericReal<is_ad>>(area_names[j]));
    _perimeters[j] = &(this->template getFunctor<GenericReal<is_ad>>(perimeter_names[j]));
    _lengths[j] = &(this->template getFunctor<GenericReal<is_ad>>(length_names[j]));
    _alphas[j] = &(this->template getFunctor<GenericReal<is_ad>>(alpha_names[j]));
    _forms_losses[j] = &(this->template getFunctor<GenericReal<is_ad>>(forms_loss_names[j]));
    _dPps[j] = &(this->template getFunctor<GenericReal<is_ad>>(dPp_names[j]));
    _roughnesses[j] = &(this->template getFunctor<GenericReal<is_ad>>(roughness_names[j]));
  }
}

template class IncompressibleMomentumSPBaseTempl<false>;
template class IncompressibleMomentumSPBaseTempl<true>;
