//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADIncompressibleEnergySPScalarKernel.h"

#include "ADFunctorInterface.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADIncompressibleEnergySPScalarKernel);

InputParameters
ADIncompressibleEnergySPScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params += ADFunctorInterface::validParams();
  params.addClassDescription("Implements a generic energy solve over a 1D flow path segment.");
  params.addCoupledVar("mass_flow_rate",
                       {},
                       "Mass flow rate in component. Takes a "
                       "scalar variable name");
  params.addCoupledVar("inlet_temperature",
                       {},
                       "Fluid temperature of nominal inlet segment/component (N-1). Takes a "
                       "scalar variable name");
  params.addCoupledVar("outlet_temperature",
                       {},
                       "Fluid temperature of nominal outlet segment/component (N+1). Takes a "
                       "scalar variable name");
  params.addCoupledVar("wall_temperature",
                       {},
                       "Wall temperature adjacent to fluid. Takes a "
                       "scalar variable name");
  params.addParam<bool>(
      "is_implicit",
      true,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addParam<UserObjectName>("reference_fp",
                                  "Liquid-phase fluid properties; see momentum kernel.");
  params.addRequiredParam<MooseFunctorName>("area", "Segment/Component flow area [m^2]");
  params.addRequiredParam<MooseFunctorName>("perimeter", "Segment/Component wetted perimeter [m]");
  params.addRequiredParam<MooseFunctorName>("length", "Segment/Component length [m]");
  params.addRequiredParam<MooseFunctorName>("heat_transfer_coefficient",
                                            "Heat transfer coefficient of the fluid [W/(m^2-K)]");
  params.addParam<Point>("functor_evaluation_point",
                         Point(),
                         "Point at which spatially varying functors (functions, field variables, "
                         "functor material properties) are evaluated");

  return params;
}

ADIncompressibleEnergySPScalarKernel::ADIncompressibleEnergySPScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
    ADFunctorInterface(this),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    _fp_ref(isParamValid("reference_fp") ? &getUserObjectByName<SinglePhaseFluidProperties>(
                                               getParam<UserObjectName>("reference_fp"))
                                         : nullptr),
    _m(ScalarCoupleable::adCoupledScalarValue("mass_flow_rate")),
    _Tup(ScalarCoupleable::adCoupledScalarValue("inlet_temperature")),
    _Tdown(ScalarCoupleable::adCoupledScalarValue("outlet_temperature")),
    _Tw(ScalarCoupleable::adCoupledScalarValue("wall_temperature")),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _Pref(this->template getFunctor<ADReal>("reference_pressure")),
    _area(this->template getFunctor<ADReal>("area")),
    _perimeter(this->template getFunctor<ADReal>("perimeter")),
    _length(this->template getFunctor<ADReal>("length")),
    _h(this->template getFunctor<ADReal>("heat_transfer_coefficient")),
    _functor_point(getParam<Point>("functor_evaluation_point"))
{
}

void
ADIncompressibleEnergySPScalarKernel::initialSetup()
{
  ADScalarKernel::initialSetup();

  // Locate the element that spatially varying functors will be evaluated on
  auto pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();
  _functor_elem = (*pl)(_functor_point);
}

ADReal
ADIncompressibleEnergySPScalarKernel::computeQpResidual()
{
  ADReal energy_residual = 0;
  const auto qp = functorArg();
  const int i = 0;
  const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
  // start by getting fluid properties
  const auto in = 1.0 / 2.0 * (1 - _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _Tdown[i] +
                  1.0 / 2.0 * (1 + _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _Tup[i];
  const auto Tm = (Base::_u[i] + in) / 2;
  const auto rho = _fp.rho_from_p_T(_Pref(qp, state), Tm);
  const auto cp = _fp.cp_from_p_T(_Pref(qp, state), Tm);
  const auto rho_ref = _fp_ref ? _fp_ref->rho_from_p_T(_Pref(qp, state), Tm) : rho;

  // Heat transfer to fluid (Dittus-Boelter)
  const auto q = _h(qp, state) * _perimeter(qp, state) / 2.0 * (2.0 * _Tw[i] - Base::_u[i] - in);
  // Advection component
  energy_residual += (_m[i] / 2.0 * (1 - _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _Tdown[i] -
                      _m[i] / 2.0 * (1 + _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08)) * _Tup[i] +
                      _m[i] * _m[i] / sqrt(_m[i] * _m[i] + 1.0e-08) * Base::_u[i]) /
                     _length(qp, state) / _area(qp, state) / rho_ref;
  // Wall heat transfer
  energy_residual -= q / _area(qp, state) / rho / cp;

  return energy_residual;
}

Real
ADIncompressibleEnergySPScalarKernel::computeQpJacobian()
{
  mooseError("Internal error, calling computeQpJacobian in AD class.");
  return 0.0;
}
