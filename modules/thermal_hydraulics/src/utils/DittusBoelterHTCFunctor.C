//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DittusBoelterHTCFunctor.h"

#include "Numerics.h"
#include "SinglePhaseFluidProperties.h"

DittusBoelterHTCFunctor::DittusBoelterHTCFunctor(const MooseFunctorName & name,
                                                 const SinglePhaseFluidProperties & fp,
                                                 const ADVariableValue & m,
                                                 const ADVariableValue & T_fluid,
                                                 const ADVariableValue & T_fluid_inlet,
                                                 const ADVariableValue & T_fluid_outlet,
                                                 const Moose::Functor<ADReal> & P_ref,
                                                 const Moose::Functor<ADReal> & flow_area,
                                                 const Moose::Functor<ADReal> & wetted_perimeter)
  : Moose::FunctorBase<ADReal>(name),
    _fp(fp),
    _m(m),
    _T_fluid(T_fluid),
    _T_fluid_inlet(T_fluid_inlet),
    _T_fluid_outlet(T_fluid_outlet),
    _P_ref(P_ref),
    _flow_area(flow_area),
    _wetted_perimeter(wetted_perimeter)
{
}

DittusBoelterHTCFunctor::ValueType
DittusBoelterHTCFunctor::evaluate(const Moose::StateArg & state) const
{
  using std::abs, std::pow;

  // The geometry functors are spatially uniform over the segment, so any spatial argument
  // would give the same result; a null-element one is used because a ScalarKernel consumer
  // has no element to offer.
  const Moose::ElemArg elem_arg = Moose::ElemArg();
  const unsigned int i = 0;

  // Upwind the segment inlet temperature on the flow direction
  const auto T_in = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _T_fluid_outlet[i] +
                    1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _T_fluid_inlet[i];

  // Properties at the bulk film temperature of the segment
  const auto T_film = (_T_fluid[i] + T_in) / 2;
  const auto P_ref = _P_ref(elem_arg, state);
  const auto mu = _fp.mu_from_p_T(P_ref, T_film);
  const auto cp = _fp.cp_from_p_T(P_ref, T_film);
  const auto k = _fp.k_from_p_T(P_ref, T_film);

  const auto A = _flow_area(elem_arg, state);
  const auto D_h = 4.0 * A / _wetted_perimeter(elem_arg, state);
  // Formed from the mass flux G = rho * vel, which the segment's mass flow rate gives
  // directly, so no density evaluation is needed
  const auto G = abs(_m[i]) / A;
  const auto Re = G * D_h / mu;
  const auto Pr = THM::Prandtl(cp, mu, k);
  const auto Nu = 0.023 * pow(Re, 0.8) * pow(Pr, 0.4);

  return THM::wallHeatTransferCoefficient(Nu, k, D_h);
}
