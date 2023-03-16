//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateConductionRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateConductionRZ);

InputParameters
HeatRateConductionRZ::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params += RZSymmetry::validParams();

  params.addRequiredCoupledVar("temperature", "Temperature");
  params.addRequiredParam<MaterialPropertyName>("thermal_conductivity",
                                                "Thermal conductivity material property");
  params.addRequiredParam<bool>("inward",
                                "Set to true to get the conduction rate into the domain and to "
                                "false to the get the conduction rate out of the domain");

  params.addClassDescription("Integrates a conduction heat flux over an RZ boundary.");

  return params;
}

HeatRateConductionRZ::HeatRateConductionRZ(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    RZSymmetry(this, parameters),

    _grad_T(coupledGradient("temperature")),
    _k(getADMaterialProperty<Real>("thermal_conductivity")),
    _direction_factor(getParam<bool>("inward") ? -1.0 : 1.0)
{
}

Real
HeatRateConductionRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return -_direction_factor * MetaPhysicL::raw_value(_k[_qp]) * _grad_T[_qp] * _normals[_qp] *
         circumference;
}
