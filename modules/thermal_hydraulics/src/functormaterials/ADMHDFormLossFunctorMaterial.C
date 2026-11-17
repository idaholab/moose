//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMHDFormLossFunctorMaterial.h"
#include "ScalarCoupleable.h"
#include "SinglePhaseFluidProperties.h"
#include "MooseMesh.h"
#include "libmesh/point_locator_base.h"

registerMooseObject("ThermalHydraulicsApp", ADMHDFormLossFunctorMaterial);

InputParameters
ADMHDFormLossFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<std::string>(
      "K_name", "Name to give the loss coefficient functor material property");
  params.addClassDescription("Computes forms loss coefficient using RELAP/ATHENA MHD model.");
  params.addCoupledVar("temperature",
                       {},
                       "Fluid temperature in segment. Takes a "
                       "scalar variable name");
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
  params.addParam<bool>(
      "is_implicit",
      false,
      "Whether an explicit (previous value calculation) or implicit (current value) is used");
  params.addRequiredParam<MooseFunctorName>("reference_pressure", "system reference pressure [Pa]");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addRequiredParam<MooseFunctorName>("area", "Segment/Component flow area [m^2]");
  params.addRequiredParam<MooseFunctorName>("perimeter", "Segment/Component wetted perimeter [m]");
  params.addRequiredParam<MooseFunctorName>("length", "Segment/Component length [m]");
  params.addRequiredParam<MooseFunctorName>("sigma_f", "Fluid conductivity [1/ohm*m]");
  params.addRequiredParam<MooseFunctorName>("sigma_w", "Wall conductivity [1/ohm*m]");
  params.addRequiredParam<MooseFunctorName>("wall_thickness", "Wall thickness [m]");
  params.addRequiredParam<MooseFunctorName>("field_strength", "Magnetic field strength [T]");
  params.addParam<Point>("functor_evaluation_point",
                         Point(),
                         "Point at which spatially varying functors (functions, field variables, "
                         "functor material properties) are evaluated");
  return params;
}

ADMHDFormLossFunctorMaterial::ADMHDFormLossFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fp(this->template getUserObject<SinglePhaseFluidProperties>("fp")),
    _T(ScalarCoupleable::adCoupledScalarValue("temperature")),
    _m(ScalarCoupleable::adCoupledScalarValue("mass_flow_rate")),
    _Tup(ScalarCoupleable::coupledScalarValue("inlet_temperature")),
    _Tdown(ScalarCoupleable::coupledScalarValue("outlet_temperature")),
    _is_implicit(this->template getParam<bool>("is_implicit")),
    _Pref(this->template getFunctor<ADReal>("reference_pressure")),
    _area(this->template getFunctor<ADReal>("area")),
    _perimeter(this->template getFunctor<ADReal>("perimeter")),
    _length(this->template getFunctor<ADReal>("length")),
    _sigf(this->template getFunctor<ADReal>("sigma_f")),
    _sigw(this->template getFunctor<ADReal>("sigma_w")),
    _tw(this->template getFunctor<ADReal>("wall_thickness")),
    _B(this->template getFunctor<ADReal>("field_strength")),
    _functor_point(getParam<Point>("functor_evaluation_point"))
{
  addFunctorProperty<ADReal>(
      getParam<std::string>("K_name"),
      [this](const auto & /*geom_arg*/, const auto & /*state*/) -> ADReal
      {
        const auto qp = functorArg();
        const int i = 0;
        const auto state = _is_implicit ? Moose::currentState() : Moose::oldState();
        // start by getting fluid properties
        const auto in = 1.0 / 2.0 * (1 - abs(_m[i]) / _m[i]) * _Tdown[i] +
                        1.0 / 2.0 * (1 + abs(_m[i]) / _m[i]) * _Tup[i];
        const auto mu = _fp.mu_from_p_T(_Pref(qp, state), (_T[i] + in) / 2);
        // auto sigf = _fp.sig_from_p_T(_Pref(qp, state), (_T[i] + in) / 2);

        const auto Dh = 4.0 * _area(qp, state) / _perimeter(qp, state);
        const auto G = _m[i] / _area(qp, state);

        // Hartmann number and conductance ratio
        const auto M = _B(qp, state) * Dh / 2.0 * sqrt(_sigf(qp, state) / mu);
        const auto phi = 2.0 * _sigw(qp, state) * _tw(qp, state) / _sigf(qp, state) / Dh;

        // Assuming circular geometry and uniform, perpendicular magnetc field for now,
        // since limited information on delta & fconfig is available otherwise.
        const auto delta = 0;
        const auto fconfig = 1;
        const auto FWF = (1 / (M - 1) + phi / (1 + phi + delta)) * fconfig * _sigf(qp, state) *
                         pow(_B(qp, state), 2);

        // Must convert to unitless, since FWF has units of 1/s and forms losses are expected to be
        // unitless
        return 2.0 * FWF * _length(qp, state) / G;
      });
}

void
ADMHDFormLossFunctorMaterial::initialSetup()
{
  FunctorMaterial::initialSetup();

  // Locate the element that spatially varying functors will be evaluated on
  auto pl = _mesh.getPointLocator();
  pl->enable_out_of_mesh_mode();
  _functor_elem = (*pl)(_functor_point);
}
