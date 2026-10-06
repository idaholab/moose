//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeOuterWallRadiationScalarKernel.h"

#include "FunctorInterface.h"
#include "ThermalSolidProperties.h"

registerMooseObject("ThermalHydraulicsApp", PipeOuterWallRadiationScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallRadiationScalarKernel);

template <bool is_ad>
InputParameters
PipeOuterWallRadiationScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = is_ad ? ADScalarKernel::validParams() : ScalarKernel::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Adds a radiative heat transfer term, exchanged with an ambient/surrounding "
      "environment, to the outer-surface radial node of a pipe wall. This kernel is meant to "
      "be added alongside another scalar kernel (such as "
      "PipeOuterWallAmbientTemperatureScalarKernel) acting on the same scalar variable, "
      "analogous to how ODETimeDerivative supplies the time-derivative term.");
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<UserObjectName>("sp", "The name of the user object for solid properties");
  params.addRequiredParam<MooseFunctorName>("area",
                                            "Cross-sectional area of this wall layer [m^2]");
  params.addRequiredParam<MooseFunctorName>(
      "perimeter", "Perimeter of this wall node exposed to the radiative environment [m]");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor [K]");
  params.addRequiredParam<MooseFunctorName>("emissivity", "Emissivity functor");
  params.addParam<MooseFunctorName>("view_factor", 1.0, "View factor functor");
  params.addParam<MooseFunctorName>(
      "scale", 1.0, "Functor by which to scale the radiative heat transfer term");
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "Stefan-Boltzmann constant");

  return params;
}

template <bool is_ad>
PipeOuterWallRadiationScalarKernelTempl<is_ad>::PipeOuterWallRadiationScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    _sp(this->template getUserObject<ThermalSolidProperties>("sp")),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _area(this->template getFunctor<GenericReal<is_ad>>("area")),
    _perimeter(this->template getFunctor<GenericReal<is_ad>>("perimeter")),
    _T_ambient(this->template getFunctor<GenericReal<is_ad>>("T_ambient")),
    _emissivity(this->template getFunctor<GenericReal<is_ad>>("emissivity")),
    _view_factor(this->template getFunctor<GenericReal<is_ad>>("view_factor")),
    _scale(this->template getFunctor<GenericReal<is_ad>>("scale")),
    _sigma(this->template getParam<Real>("stefan_boltzmann_constant"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PipeOuterWallRadiationScalarKernelTempl<is_ad>::computeQpResidual()
{
  const Moose::ElemArg qp = Moose::ElemArg();
  const int i = 0;
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();

  const auto A = _area(qp, state);
  const auto cp = _sp.cp_from_T(Base::_u[i]);
  const auto rho = _sp.rho_from_T(Base::_u[i]);

  const auto scale = _scale(qp, state);
  const auto emissivity = _emissivity(qp, state);
  const auto view_factor = _view_factor(qp, state);
  const auto T4 = pow(Base::_u[i], 4);
  const auto T4inf = pow(_T_ambient(qp, state), 4);

  // Radiative heat transfer with the ambient/surrounding environment
  const auto q_radiation =
      _sigma * scale * emissivity * view_factor * _perimeter(qp, state) * (T4inf - T4);

  return -q_radiation / A / rho / cp;
}

template <bool is_ad>
Real
PipeOuterWallRadiationScalarKernelTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    const Moose::ElemArg qp = Moose::ElemArg();
    const int i = 0;
    const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();

    const auto A = _area(qp, state);
    const auto cp = _sp.cp_from_T(Base::_u[i]);
    const auto rho = _sp.rho_from_T(Base::_u[i]);

    const auto scale = _scale(qp, state);
    const auto emissivity = _emissivity(qp, state);
    const auto view_factor = _view_factor(qp, state);

    // Radiative heat transfer with the ambient/surrounding environment
    const auto q_radiation = -4.0 * _sigma * scale * emissivity * view_factor *
                             _perimeter(qp, state) * pow(Base::_u[i], 3);

    return -q_radiation / A / rho / cp;
  }
  else
  {
    mooseError("computeQpJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
PipeOuterWallRadiationScalarKernelTempl<true>::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

template class PipeOuterWallRadiationScalarKernelTempl<false>;
template class PipeOuterWallRadiationScalarKernelTempl<true>;
