//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledPressureFlowPathMomentumSCSPScalarKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", CoupledPressureFlowPathMomentumSCSPScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADCoupledPressureFlowPathMomentumSCSPScalarKernel);

template <bool is_ad>
InputParameters
CoupledPressureFlowPathMomentumSCSPScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params =
      is_ad ? ADScalarTimeDerivative::validParams() : ODETimeDerivative::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Implements a generic momentum solve over a 1D flow path, acting on the reference pressure "
      "drop."
      "This kernel should only be used if the system is a closed loop, and this component should "
      "close "
      "one or more FlowPathMomentumSCSPScalarKernels. This is because this component flips the "
      "pressure "
      "gradient term sign."
      "Note: Using this kernel also requires using a CoupledODETimeDerivativeScalarKernel, at "
      "least until "
      "we get around to making a suitable ADCoupledTimeDerivativeScalarKernel base class for this."
      "Flow path may be represented by N segments, must provide unique geometric, pump pressure,"
      " and epsilon information for each segment."
      "Assumes incompressibility locally."
      "This version utilizes the Single Component, Single Phase fluid properties base class.");
  // Lots of inputs so we need to be clear what is what
  // This block defines coupled state variables the kernel relies on
  params.addCoupledVar("coupled_mass_flow_rate",
                       {},
                       "coupled_mass_flow_rate. Takes a "
                       "scalar variable name");
  params.addCoupledVar("temperatures",
                       {},
                       "Fluid temperature in each segment of this component. Takes a "
                       "list of scalar variable names");
  // This block grabs boolean parameters that control the solve type
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  // This block characterizes the geometry and fluid type
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
  // This block characterizes material properties not included in fluid properties object
  params.addParam<std::vector<MooseFunctorName>>(
      "roughnesses",
      std::vector<MooseFunctorName>({}),
      "Component wall roughnesses per segment [m]. Takes a vector of functors.");
  params.addParam<MooseFunctorName>("g", 9.81, "Gravitational acceleration [m/s]");

  return params;
}

template <bool is_ad>
CoupledPressureFlowPathMomentumSCSPScalarKernelTempl<
    is_ad>::CoupledPressureFlowPathMomentumSCSPScalarKernelTempl(const InputParameters & parameters)
  : Base(parameters),
    FunctorInterface(this),
    // Lots of inputs so we need to be clear what is what
    // This block defines coupled state variables the kernel relies on
    _mc(ScalarCoupleable::coupledScalarValue("coupled_mass_flow_rate")),
    _n_temps(ScalarCoupleable::coupledScalarComponents("temperatures")),
    _T(_n_temps),
    // This block grabs boolean parameters that control the solve type
    _is_implicit(this->template getParam<bool>("is_implicit")),
    // This block characterizes the geometry and fluid type
    _Pref(this->template getFunctor<GenericReal<is_ad>>("reference_pressure")),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    _n_segments(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _areas(this->template getParam<std::vector<MooseFunctorName>>("areas").size()),
    _perimeters(this->template getParam<std::vector<MooseFunctorName>>("perimeters").size()),
    _lengths(this->template getParam<std::vector<MooseFunctorName>>("lengths").size()),
    _alphas(this->template getParam<std::vector<MooseFunctorName>>("alphas").size()),
    _forms_losses(this->template getParam<std::vector<MooseFunctorName>>("forms_losses").size()),
    _dPps(this->template getParam<std::vector<MooseFunctorName>>("pump_pressures").size()),
    // This block characterizes material properties not included in fluid properties object
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
  for (size_t i = 0; i < _n_segments; ++i)
  {
    _T[i] = &(ScalarCoupleable::coupledScalarValue("temperatures", i));
    _areas[i] = &(this->template getFunctor<GenericReal<is_ad>>(area_names[i]));
    _perimeters[i] = &(this->template getFunctor<GenericReal<is_ad>>(perimeter_names[i]));
    _lengths[i] = &(this->template getFunctor<GenericReal<is_ad>>(length_names[i]));
    _alphas[i] = &(this->template getFunctor<GenericReal<is_ad>>(alpha_names[i]));
    _forms_losses[i] = &(this->template getFunctor<GenericReal<is_ad>>(forms_loss_names[i]));
    _dPps[i] = &(this->template getFunctor<GenericReal<is_ad>>(dPp_names[i]));
    _roughnesses[i] = &(this->template getFunctor<GenericReal<is_ad>>(roughness_names[i]));
  }
}

template <bool is_ad>
GenericReal<is_ad>
CoupledPressureFlowPathMomentumSCSPScalarKernelTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> momentum_residual = 0;
  const Moose::ElemArg _qp = Moose::ElemArg();
  const int _i = 0;
  const auto _state = _is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting global fluid properties
  auto _mu = _fp.mu_from_p_T(_Pref(_qp, _state), (*(_T[0]))[_i]);
  auto _rhog = _fp.rho_from_p_T(_Pref(_qp, _state), (*(_T[0]))[_i]);
  // loop over segments
  for (size_t i = 0; i < _n_segments; ++i)
  {
    // Decide flow regime for friction factor
    auto _Dh = 4.0 * (*(_areas[i]))(_qp, _state) / (*(_perimeters[i]))(_qp, _state);
    auto _G = _mc[_i] / (*(_areas[i]))(_qp, _state);
    auto _Re = abs(_G) * _Dh / _mu;
    auto _lam = 64.0 / _Re;
    auto _turb =
        0.25 /
        pow((log10((*(_roughnesses[i]))(_qp, _state) / (_Dh * 3.7) + 5.74 / pow(_Re, 0.9))), 2);
    auto _fd = 64.0 / _Re;
    auto _pfd = &_fd;
    if (_Re < 2300.0) // laminar
    {
      *_pfd = _lam;
    }
    else if (_Re > 4000.0) // turbulent using Swamee-Jain approx. of Colebrook-White eq.
    {
      *_pfd = _turb;
    }
    else // transition, conservative interpolation between the two
    {
      *_pfd = (_turb - _lam) / 1700 * _Re + _lam;
      *_pfd = std::max(*_pfd, std::max(_lam, _turb));
    }
    // Friction
    momentum_residual += _fd / _Dh * _G * abs(_G) / 2.0 / _rhog * (*(_areas[i]))(_qp, _state);
    // Forms
    momentum_residual += (*(_forms_losses[i]))(_qp, _state) * _G * abs(_G) / 2.0 / _rhog *
                         (*(_areas[i]))(_qp, _state) / (*(_lengths[i]))(_qp, _state);
    // Gravity
    // get local density for natural circulation aspect
    auto _rhol = _fp.rho_from_p_T(_Pref(_qp, _state), (*(_T[i]))[_i]);
    momentum_residual += _rhol * _gravity(_qp, _state) * sin((*(_alphas[i]))(_qp, _state)) *
                         (*(_areas[i]))(_qp, _state);
    // Pump pressure
    momentum_residual -=
        (*(_dPps[i]))(_qp, _state) * (*(_areas[i]))(_qp, _state) / (*(_lengths[i]))(_qp, _state);
    // reference pressure drop
    momentum_residual -= Base::_u[_i] * (*(_areas[i]))(_qp, _state) / (*(_lengths[i]))(_qp, _state);
  }

  return momentum_residual;
}

template <bool is_ad>
Real
CoupledPressureFlowPathMomentumSCSPScalarKernelTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    Real momentum_residual = 0;
    const Moose::ElemArg _qp = Moose::ElemArg();
    const auto _state = _is_implicit ? Moose::currentState() : Moose::oldState();
    // loop over segments
    for (size_t i = 0; i < _n_segments; ++i)
    {
      // reference pressure drop
      momentum_residual -= (*(_areas[i]))(_qp, _state) / (*(_lengths[i]))(_qp, _state);
    }

    return momentum_residual;
  }
  else
  {
    mooseError("computeQpJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
CoupledPressureFlowPathMomentumSCSPScalarKernelTempl<true>::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

template class CoupledPressureFlowPathMomentumSCSPScalarKernelTempl<false>;
template class CoupledPressureFlowPathMomentumSCSPScalarKernelTempl<true>;
