//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatRateExternalAppConvectionRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatRateExternalAppConvectionRZ);

InputParameters
HeatRateExternalAppConvectionRZ::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params += RZSymmetry::validParams();

  params.addRequiredCoupledVar("T", "Temperature");
  params.addRequiredCoupledVar("T_ext", "Temperature from external application");
  params.addRequiredCoupledVar("htc_ext", "Heat transfer coefficient from external application");

  params.addClassDescription("Integrates a cylindrical heat structure boundary convective heat "
                             "flux from an external application");

  return params;
}

HeatRateExternalAppConvectionRZ::HeatRateExternalAppConvectionRZ(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    RZSymmetry(this, parameters),

    _T(coupledValue("T")),
    _T_ext(coupledValue("T_ext")),
    _htc_ext(coupledValue("htc_ext"))
{
}

Real
HeatRateExternalAppConvectionRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * _htc_ext[_qp] * (_T_ext[_qp] - _T[_qp]);
}
