//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeOuterWallAmbientTemperatureScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"

registerMooseObject("ThermalHydraulicsApp", PipeOuterWallAmbientTemperatureScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallAmbientTemperatureScalarKernel);

template <bool is_ad>
InputParameters
PipeOuterWallAmbientTemperatureScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "Solves for the temperature of the outer-surface radial node of a pipe wall.");
  params.addCoupledVar("inner_wall_temperature",
                       {},
                       "Temperature of the inner-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addRequiredParam<MooseFunctorName>("T_ambient", "Ambient temperature functor [K]");
  params.addRequiredParam<MooseFunctorName>(
      "htc_ambient", "Ambient heat transfer coefficient functor [W/(m^2*K)]");
  params.addRequiredParam<MooseFunctorName>(
      "ambient_perimeter", "Perimeter of this wall node exposed to the ambient environment [m]");

  return params;
}

template <bool is_ad>
PipeOuterWallAmbientTemperatureScalarKernelTempl<
    is_ad>::PipeOuterWallAmbientTemperatureScalarKernelTempl(const InputParameters & parameters)
  : Base(parameters),
    _Tin(ScalarCoupleable::coupledScalarValue("inner_wall_temperature")),
    _T_ambient(this->template getFunctor<GenericReal<is_ad>>("T_ambient")),
    _htc_ambient(this->template getFunctor<GenericReal<is_ad>>("htc_ambient")),
    _ambient_perimeter(this->template getFunctor<GenericReal<is_ad>>("ambient_perimeter"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PipeOuterWallAmbientTemperatureScalarKernelTempl<is_ad>::convectiveResidual() const
{
  const Moose::ElemArg qp = Moose::ElemArg();
  const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();

  // Convective heat transfer with the ambient environment
  return _htc_ambient(qp, state) * _ambient_perimeter(qp, state) *
         (_T_ambient(qp, state) - Base::_u[0]);
}

template <bool is_ad>
Real
PipeOuterWallAmbientTemperatureScalarKernelTempl<is_ad>::convectiveJacobian() const
{
  if constexpr (!is_ad)
  {
    const Moose::ElemArg qp = Moose::ElemArg();
    const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();

    // Convective heat transfer with the ambient environment
    return -_htc_ambient(qp, state) * _ambient_perimeter(qp, state);
  }
  else
  {
    mooseError("convectiveJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
PipeOuterWallAmbientTemperatureScalarKernelTempl<true>::convectiveJacobian() const
{
  mooseError("Internal error, calling convectiveJacobian in AD class.");
  return 0.0;
}

template class PipeOuterWallAmbientTemperatureScalarKernelTempl<false>;
template class PipeOuterWallAmbientTemperatureScalarKernelTempl<true>;
