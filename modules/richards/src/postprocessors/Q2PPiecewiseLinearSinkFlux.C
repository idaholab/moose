//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the mass due to a flux from the boundary of a volume.
//
#include "Q2PPiecewiseLinearSinkFlux.h"
#include "Function.h"

registerMooseObject("RichardsApp", Q2PPiecewiseLinearSinkFlux);

InputParameters
Q2PPiecewiseLinearSinkFlux::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addParam<UserObjectName>(
      "fluid_density",
      "The fluid density as a RichardsDensity UserObject.  If this and the "
      "fluid_viscosity are given, then fluxes are multiplied by "
      "(density*permeability_nn/viscosity), where the '_nn' indicates the "
      "component normal to the boundary.  In this case bare_flux is measured in "
      "Pa.s^-1.  This can be used in conjunction with fluid_relperm.");
  params.addParam<Real>("fluid_viscosity", "The fluid dynamic viscosity.");
  params.addParam<UserObjectName>(
      "fluid_relperm",
      "The fluid density as a RichardsRelPerm UserObject (eg RichardsRelPermPower "
      "for water, or Q2PRelPermPostGas for gas).  If this and the saturation "
      "variable are defined then the flux will be motiplied by relative "
      "permeability.  This can be used in conjunction with fluid_density");
  params.addCoupledVar("saturation", "The name of the water saturation variable");
  params.addRequiredCoupledVar("porepressure", "The name of the porepressure variable");
  params.addRequiredParam<std::vector<Real>>(
      "pressures", "Tuple of pressure values.  Must be monotonically increasing.");
  params.addRequiredParam<std::vector<Real>>(
      "bare_fluxes",
      "Tuple of flux values (measured in kg.m^-2.s^-1 if not using fluid_density, "
      "otherwise in Pa.s^-1).  This flux is OUT of the medium: hence positive "
      "values of flux means this will be a SINK, while negative values indicate "
      "this flux will be a SOURCE.  A piecewise-linear fit is performed to the "
      "(pressure,bare_fluxes) pairs to obtain the flux at any arbitrary pressure, "
      "and the first or last bare_flux values are used if the quad-point pressure "
      "falls outside this range.");
  params.addParam<FunctionName>("multiplying_fcn",
                                1.0,
                                "The flux will be multiplied by this spatially-and-temporally "
                                "varying function.  This is useful if the boundary is a moving "
                                "boundary controlled by RichardsExcav.");
  params.addClassDescription("Records the fluid flow into a sink (positive values indicate fluid "
                             "is flowing from porespace into the sink).");
  return params;
}

Q2PPiecewiseLinearSinkFlux::Q2PPiecewiseLinearSinkFlux(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _sink_func(getParam<std::vector<Real>>("pressures"),
               getParam<std::vector<Real>>("bare_fluxes")),
    _m_func(getFunction("multiplying_fcn")),
    _pp(coupledValue("porepressure")),
    _use_mobility(isParamValid("fluid_density") && isParamValid("fluid_viscosity")),
    _use_relperm(isParamValid("fluid_relperm") && isCoupled("saturation")),
    _density(isParamValid("fluid_density") ? &getUserObject<RichardsDensity>("fluid_density")
                                           : NULL),
    _viscosity(isParamValid("fluid_viscosity") ? getParam<Real>("fluid_viscosity") : 1),
    _relperm(isParamValid("fluid_relperm") ? &getUserObject<RichardsRelPerm>("fluid_relperm")
                                           : NULL),
    _sat(isCoupled("saturation") ? coupledValue("saturation") : _zero),
    _permeability(getMaterialProperty<RealTensorValue>("permeability"))
{
  if ((isParamValid("fluid_density") && !isParamValid("fluid_viscosity")) ||
      (!isParamValid("fluid_density") && isParamValid("fluid_viscosity")))
    mooseError("Q2PPiecewiseLinearSink: you must supply both of fluid_density and fluid_viscosity "
               "if you wish to multiply by the mobility");
  if ((isParamValid("fluid_relperm") && !isCoupled("saturation")) ||
      (!isParamValid("fluid_relperm") && isCoupled("saturation")))
    mooseError("Q2PPiecewiseLinearSink: you must supply both of fluid_relperm and saturation if "
               "you wish to multiply by the relative permeaility");
}

Real
Q2PPiecewiseLinearSinkFlux::computeQpIntegral()
{
  Real flux = _sink_func.sample(_pp[_qp]);

  flux *= _m_func.value(_t, _q_point[_qp]);

  if (_use_mobility)
  {
    Real k = (_permeability[_qp] * _normals[_qp]) * _normals[_qp];
    flux *= _density->density(_pp[_qp]) * k / _viscosity;
  }
  if (_use_relperm)
    flux *= _relperm->relperm(_sat[_qp]);

  return flux * _dt;
}
