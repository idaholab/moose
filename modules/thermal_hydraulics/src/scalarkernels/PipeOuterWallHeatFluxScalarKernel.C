//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeOuterWallHeatFluxScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"

registerMooseObject("ThermalHydraulicsApp", PipeOuterWallHeatFluxScalarKernel);
registerMooseObject("ThermalHydraulicsApp", ADPipeOuterWallHeatFluxScalarKernel);

template <bool is_ad>
InputParameters
PipeOuterWallHeatFluxScalarKernelTempl<is_ad>::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "Solves for the temperature of the outer-surface radial node of a pipe wall, with a "
      "prescribed heat flux applied to its non-radial side.");
  params.addCoupledVar("inner_wall_temperature",
                       {},
                       "Temperature of the inner-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addRequiredParam<MooseFunctorName>("heat_flux", "Applied heat flux functor [W/m^2]");
  params.addRequiredParam<MooseFunctorName>(
      "heated_perimeter", "Perimeter of this wall node over which the heat flux is applied [m]");

  return params;
}

template <bool is_ad>
PipeOuterWallHeatFluxScalarKernelTempl<is_ad>::PipeOuterWallHeatFluxScalarKernelTempl(
    const InputParameters & parameters)
  : Base(parameters),
    _Tin(ScalarCoupleable::coupledScalarValue("inner_wall_temperature")),
    _heat_flux(this->template getFunctor<GenericReal<is_ad>>("heat_flux")),
    _heated_perimeter(this->template getFunctor<GenericReal<is_ad>>("heated_perimeter"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PipeOuterWallHeatFluxScalarKernelTempl<is_ad>::convectiveResidual() const
{
  const Moose::ElemArg qp = Moose::ElemArg();
  const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();

  // Prescribed heat flux applied to this node, positive into the wall
  return _heat_flux(qp, state) * _heated_perimeter(qp, state);
}

template <bool is_ad>
Real
PipeOuterWallHeatFluxScalarKernelTempl<is_ad>::convectiveJacobian() const
{
  if constexpr (!is_ad)
    // The applied heat flux does not depend on this node's own temperature
    return 0.0;
  else
  {
    mooseError("convectiveJacobian() should not be called in AD mode");
    return 0;
  }
}

template <>
Real
PipeOuterWallHeatFluxScalarKernelTempl<true>::convectiveJacobian() const
{
  mooseError("Internal error, calling convectiveJacobian in AD class.");
  return 0.0;
}

template class PipeOuterWallHeatFluxScalarKernelTempl<false>;
template class PipeOuterWallHeatFluxScalarKernelTempl<true>;
