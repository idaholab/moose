//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel.h"

#include "FunctorInterface.h"
#include "ScalarCoupleable.h"
#include "MooseMesh.h"
#include "libmesh/point_locator_base.h"

registerMooseObject("ThermalHydraulicsApp",
                    ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel);

InputParameters
ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel::validParams()
{
  InputParameters params = Base::validParams();
  params.addClassDescription(
      "Solves for the temperature of the outer-surface radial node of a pipe wall, "
      "convectively coupled to a secondary-side flowing fluid, such as the shell side of a "
      "heat exchanger.");
  params.addCoupledVar("mass_flow_rate",
                       {},
                       "Mass flow rate of the secondary-side fluid. Takes a "
                       "scalar variable name");
  params.addCoupledVar("inner_wall_temperature",
                       {},
                       "Temperature of the inner-surface wall node, on the other side of this "
                       "layer's radial conduction path. Takes a scalar variable name");
  params.addCoupledVar("fluid_temperature",
                       {},
                       "Secondary-side fluid temperature adjacent to this wall node. Takes a "
                       "scalar variable name");
  params.addCoupledVar(
      "upstream_fluid_temperature",
      {},
      "Secondary-side fluid temperature adjacent to the upstream wall node. Takes a scalar "
      "variable name");
  params.addCoupledVar(
      "downstream_fluid_temperature",
      {},
      "Secondary-side fluid temperature adjacent to the downstream wall node. Takes a scalar "
      "variable name");
  params.addRequiredParam<MooseFunctorName>(
      "wetted_perimeter",
      "Wetted perimeter of the secondary-side flow channel adjacent to this node [m]");
  params.addRequiredParam<MooseFunctorName>(
      "heat_transfer_coefficient",
      "Heat transfer coefficient of the secondary-side fluid [W/(m^2-K)]");

  return params;
}

ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel::
    ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel(const InputParameters & parameters)
  : Base(parameters),
    _m(ScalarCoupleable::adCoupledScalarValue("mass_flow_rate")),
    _Tin(ScalarCoupleable::adCoupledScalarValue("inner_wall_temperature")),
    _Tf(ScalarCoupleable::adCoupledScalarValue("fluid_temperature")),
    _Tfup(ScalarCoupleable::adCoupledScalarValue("upstream_fluid_temperature")),
    _Tfdown(ScalarCoupleable::adCoupledScalarValue("downstream_fluid_temperature")),
    _wetted_perimeter(this->template getFunctor<ADReal>("wetted_perimeter")),
    _h(this->template getFunctor<ADReal>("heat_transfer_coefficient"))
{
}

ADReal
ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel::convectiveLinearHeatRate() const
{
  const auto elem_arg = functorArg();
  const int i = 0;
  const auto state = Base::_is_implicit ? Moose::currentState() : Moose::oldState();
  // Upwind the segment inlet temperature on the flow direction
  const auto Tin = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tfdown[i] +
                   1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tfup[i];

  // Convective heat transfer with the secondary-side fluid
  return _h(elem_arg, state) * _wetted_perimeter(elem_arg, state) / 2.0 *
         (_Tf[i] + Tin - 2 * Base::_u[i]);
}

Real
ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel::convectiveJacobian() const
{
  mooseError("convectiveJacobian() should not be called in AD mode");
  return 0;
}
