//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncompressibleMomentumSPScalarKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", IncompressibleMomentumSPScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADIncompressibleMomentumSPScalarKernel);

template <bool is_ad>
InputParameters
IncompressibleMomentumSPScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = is_ad ? ADIncompressibleMomentumBase::validParams()
                                 : IncompressibleMomentumBase::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Implements a generic momentum solve over a 1D flow path, acting on the mass flow rate.");
  // Lots of inputs so we need to be clear what is what
  // This block defines coupled state variables the kernel relies on
  params.addCoupledVar("reference_pressure_drop",
                       {},
                       "Reference system pressure drop (IE lower to upper plenum). Takes a "
                       "scalar variable name");
  return params;
}

template <bool is_ad>
IncompressibleMomentumSPScalarKernelTempl<is_ad>::IncompressibleMomentumSPScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters),
    // Lots of inputs so we need to be clear what is what
    // This block defines coupled state variables the kernel relies on
    _dPc(ScalarCoupleable::coupledScalarValue("reference_pressure_drop"))
{
}

template <bool is_ad>
GenericReal<is_ad>
IncompressibleMomentumSPScalarKernelTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> momentum_residual = 0;
  const Moose::ElemArg _qp = Moose::ElemArg();
  const int _i = 0;
  const auto _state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting global fluid properties
  auto _mu = Base::_fp.mu_from_p_T(Base::_Pref(_qp, _state), (*(Base::_T[0]))[_i]);
  auto _rhog = Base::_fp.rho_from_p_T(Base::_Pref(_qp, _state), (*(Base::_T[0]))[_i]);
  // loop over segments
  for (size_t i = 0; i < Base::_n_segments; ++i)
  {
    // Decide flow regime for friction factor
    auto _Dh = 4.0 * (*(Base::_areas[i]))(_qp, _state) / (*(Base::_perimeters[i]))(_qp, _state);
    auto _G = Base::_u[_i] / (*(Base::_areas[i]))(_qp, _state);
    auto _Re = abs(_G) * _Dh / _mu;
    auto _lam = 64.0 / _Re;
    auto _turb =
        0.25 /
        pow((log10((*(Base::_roughnesses[i]))(_qp, _state) / (_Dh * 3.7) + 5.74 / pow(_Re, 0.9))),
            2);
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
    momentum_residual +=
        _fd * (*(Base::_lengths[i]))(_qp, _state) / _Dh * _G * abs(_G) / 2.0 / _rhog;
    // Forms
    momentum_residual += (*(Base::_forms_losses[i]))(_qp, _state) * _G * abs(_G) / 2.0 / _rhog;
    // Gravity
    // get local density for natural circulation aspect
    auto _rhol = Base::_fp.rho_from_p_T(Base::_Pref(_qp, _state), (*(Base::_T[i]))[_i]);
    momentum_residual += _rhol * Base::_gravity(_qp, _state) * (*(Base::_lengths[i]))(_qp, _state) *
                         sin((*(Base::_alphas[i]))(_qp, _state));
    // Pump pressure
    momentum_residual -= (*(Base::_dPps[i]))(_qp, _state);
    // Transient term
    momentum_residual +=
        (*(Base::_lengths[i]))(_qp, _state) / (*(Base::_areas[i]))(_qp, _state) * Base::_u_dot[_i];
  }
  // Pressure drop
  momentum_residual += _dPc[_i];

  return momentum_residual;
}

template <bool is_ad>
Real
IncompressibleMomentumSPScalarKernelTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    Real momentum_residual = 0;
    const Moose::ElemArg _qp = Moose::ElemArg();
    const int _i = 0;
    const auto _state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
    // start by getting global fluid properties
    auto _mu = Base::_fp.mu_from_p_T(Base::_Pref(_qp, _state), (*(Base::_T[0]))[_i]);
    auto _rhog = Base::_fp.rho_from_p_T(Base::_Pref(_qp, _state), (*(Base::_T[0]))[_i]);
    // loop over segments
    for (size_t i = 0; i < Base::_n_segments; ++i)
    {
      // Decide flow regime for friction factor
      auto _Dh = 4.0 * (*(Base::_areas[i]))(_qp, _state) / (*(Base::_perimeters[i]))(_qp, _state);
      auto _G = Base::_u[_i] / (*(Base::_areas[i]))(_qp, _state);
      auto _Re = abs(_G) * _Dh / _mu;
      auto _lam = 64.0 / _Re;
      auto _turb =
          0.25 /
          pow((log10((*(Base::_roughnesses[i]))(_qp, _state) / (_Dh * 3.7) + 5.74 / pow(_Re, 0.9))),
              2);
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
      momentum_residual += _fd * (*(Base::_lengths[i]))(_qp, _state) / _Dh * _G / _rhog /
                           (*(Base::_areas[i]))(_qp, _state);
      // Forms
      momentum_residual +=
          (*(Base::_forms_losses[i]))(_qp, _state) * _G / _rhog / (*(Base::_areas[i]))(_qp, _state);
      // Transient term
      momentum_residual += (*(Base::_lengths[i]))(_qp, _state) / (*(Base::_areas[i]))(_qp, _state) *
                           Base::_du_dot_du[_i];
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
IncompressibleMomentumSPScalarKernelTempl<true>::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}

template class IncompressibleMomentumSPScalarKernelTempl<false>;
template class IncompressibleMomentumSPScalarKernelTempl<true>;
