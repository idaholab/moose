//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IncompressibleMomentumSPScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", IncompressibleMomentumSPScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADIncompressibleMomentumSPScalarKernel);

template <bool is_ad>
InputParameters
IncompressibleMomentumSPScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = IncompressibleMomentumSPBaseTempl<is_ad>::validParams();
  params += FunctorInterface::validParams();
  params.addClassDescription(
      "Implements a generic momentum solve over a 1D flow path, acting on the mass flow rate.");
  params.addCoupledVar("reference_pressure_drop",
                       {},
                       "Reference system pressure drop (IE lower to upper plenum). Takes a "
                       "scalar variable name");
  return params;
}

template <bool is_ad>
IncompressibleMomentumSPScalarKernelTempl<is_ad>::IncompressibleMomentumSPScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters), _dPc(ScalarCoupleable::coupledScalarValue("reference_pressure_drop"))
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
  auto _Tave = ((*(Base::_T[0]))[_i] + (*(Base::_T[Base::_n_temps - 1]))[_i]) / 2;
  auto _mu = Base::_fp.mu_from_p_T(Base::_Pref(_qp, _state), _Tave);
  auto _rhog = Base::_fp.rho_from_p_T(Base::_Pref(_qp, _state), _Tave);
  // Global rescale factor so the mass flow rate's transient term has a unit coefficient,
  // matching the coefficient of ODETimeDerivative kernel: the path's lumped inertia is
  // Sum_j(length_j / area_j), so the whole equation is divided through by that single sum.
  GenericReal<is_ad> _inertia = 0;
  for (size_t j = 0; j < Base::_n_segments; ++j)
    _inertia += (*(Base::_lengths[j]))(_qp, _state) / (*(Base::_areas[j]))(_qp, _state);
  auto _invC = 1.0 / _inertia;
  // loop over segments
  for (size_t j = 0; j < Base::_n_segments; ++j)
  {
    // Decide flow regime for friction factor
    auto _Dh = 4.0 * (*(Base::_areas[j]))(_qp, _state) / (*(Base::_perimeters[j]))(_qp, _state);
    auto _G = Base::_u[_i] / (*(Base::_areas[j]))(_qp, _state);
    auto _fd = Base::computeFrictionFactor(_mu, _G, _Dh, j);
    // Friction
    momentum_residual +=
        _fd * (*(Base::_lengths[j]))(_qp, _state) / _Dh * _G * abs(_G) / 2.0 / _rhog * _invC;
    // Form losses
    momentum_residual +=
        (*(Base::_forms_losses[j]))(_qp, _state) * _G * abs(_G) / 2.0 / _rhog * _invC;
    // Gravity
    // get local density for natural circulation aspect
    auto _rhol = Base::_fp.rho_from_p_T(Base::_Pref(_qp, _state), (*(Base::_T[j]))[_i]);
    momentum_residual += _rhol * Base::_gravity(_qp, _state) * (*(Base::_lengths[j]))(_qp, _state) *
                         sin((*(Base::_alphas[j]))(_qp, _state)) * _invC;
    // Pump pressure
    momentum_residual -= (*(Base::_dPps[j]))(_qp, _state) * _invC;
  }
  // Pressure drop (single path-wide unknown, applied once)
  momentum_residual += _dPc[_i] * _invC;

  return momentum_residual;
}

template <bool is_ad>
Real
IncompressibleMomentumSPScalarKernelTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    Real momentum_jacob = 0;
    const Moose::ElemArg _qp = Moose::ElemArg();
    const int _i = 0;
    const auto _state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
    // start by getting global fluid properties
    auto _Tave = ((*(Base::_T[0]))[_i] + (*(Base::_T[Base::_n_temps - 1]))[_i]) / 2;
    auto _mu = Base::_fp.mu_from_p_T(Base::_Pref(_qp, _state), _Tave);
    auto _rhog = Base::_fp.rho_from_p_T(Base::_Pref(_qp, _state), _Tave);
    // Global rescale factor, see computeQpResidual()
    Real _inertia = 0;
    for (size_t j = 0; j < Base::_n_segments; ++j)
      _inertia += (*(Base::_lengths[j]))(_qp, _state) / (*(Base::_areas[j]))(_qp, _state);
    auto _invC = 1.0 / _inertia;
    // loop over segments
    for (size_t j = 0; j < Base::_n_segments; ++j)
    {
      // Decide flow regime for friction factor
      auto _Dh = 4.0 * (*(Base::_areas[j]))(_qp, _state) / (*(Base::_perimeters[j]))(_qp, _state);
      auto _G = Base::_u[_i] / (*(Base::_areas[j]))(_qp, _state);
      auto _fd = Base::computeFrictionFactor(_mu, _G, _Dh, j);
      // Friction
      momentum_jacob += _fd * (*(Base::_lengths[j]))(_qp, _state) / _Dh * _G / _rhog /
                        (*(Base::_areas[j]))(_qp, _state) * _invC;
      // Form losses
      momentum_jacob += (*(Base::_forms_losses[j]))(_qp, _state) * _G / _rhog /
                        (*(Base::_areas[j]))(_qp, _state) * _invC;
    }

    return momentum_jacob;
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
