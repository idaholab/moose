//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADIncompressibleEnergySPScalarKernel.h"

#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", ADIncompressibleEnergySPScalarKernel);

InputParameters
ADIncompressibleEnergySPScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
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
  MooseEnum bulk_T("segment_average node", "segment_average");
  params.addParam<MooseEnum>(
      "heat_transfer_bulk_temperature",
      bulk_T,
      "Fluid temperature used for wall heat transfer (and fluid properties). "
      "'segment_average': (T + T_in)/2 with T_in the upwinded neighbour (legacy). Not monotone "
      "for NTU > 2 and, at zero flow, blind to an odd/even mode of the node temperatures, which "
      "can then grow without bound. 'node': the node temperature T (well-mixed node); "
      "monotone for any NTU and any flow rate. Must match the setting of the coupled "
      "ADPipeInnerWallTemperatureScalarKernel so wall and fluid exchange the same heat.");
  params.addParam<Point>("functor_evaluation_point",
                         Point(),
                         "Point at which spatially varying functors (functions, field variables, "
                         "functor material properties) are evaluated");

  return params;
}

ADIncompressibleEnergySPScalarKernel::ADIncompressibleEnergySPScalarKernel(
    const InputParameters & parameters)
  : Base(parameters),
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
  // Bulk temperature seen by the wall: segment average (legacy) or node temperature
  const bool node_bulk = getParam<MooseEnum>("heat_transfer_bulk_temperature") == "node";
  const auto Tm = node_bulk ? ADReal(Base::_u[i]) : ADReal((Base::_u[i] + in) / 2);
  const auto rho = _fp.rho_from_p_T(_Pref(qp, state), Tm);
  const auto cp = _fp.cp_from_p_T(_Pref(qp, state), Tm);
  const auto rho_ref = _fp_ref ? _fp_ref->rho_from_p_T(_Pref(qp, state), Tm) : rho;

  // Heat transfer to fluid (Dittus-Boelter)
  const auto q = _h(qp, state) * _perimeter(qp, state) * (_Tw[i] - Tm);
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
