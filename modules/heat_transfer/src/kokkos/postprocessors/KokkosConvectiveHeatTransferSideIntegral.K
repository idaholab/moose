//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConvectiveHeatTransferSideIntegral.h"

registerKokkosUserObject("HeatTransferApp", KokkosConvectiveHeatTransferSideIntegral);

InputParameters
KokkosConvectiveHeatTransferSideIntegral::validParams()
{
  InputParameters params = KokkosSideIntegralPostprocessor::validParams();
  params.addClassDescription("Computes the total convective heat transfer across a boundary.");
  params.addRequiredCoupledVar("T_solid", "The solid temperature.");
  params.addCoupledVar("T_fluid_var", "The fluid temperature.");
  params.addCoupledVar("htc_var", "HTC variable");
  params.addParam<MaterialPropertyName>("T_fluid",
                                        "Name of the fluid temperature material property");
  params.addParam<MaterialPropertyName>("htc", "Name of alpha_wall material property");
  return params;
}

KokkosConvectiveHeatTransferSideIntegral::KokkosConvectiveHeatTransferSideIntegral(
    const InputParameters & parameters)
  : KokkosSideIntegralPostprocessor(parameters),
    _T_wall(kokkosCoupledValue("T_solid")),
    _T_fluid_is_var(isCoupled("T_fluid_var")),
    _T_fluid(_T_fluid_is_var ? kokkosCoupledValue("T_fluid_var") : kokkosZeroValue()),
    _T_fluid_mat(isParamValid("T_fluid") ? getKokkosMaterialProperty<Real>("T_fluid")
                                         : Moose::Kokkos::MaterialProperty<Real>()),
    _hw_is_var(isCoupled("htc_var")),
    _hw(_hw_is_var ? kokkosCoupledValue("htc_var") : kokkosZeroValue()),
    _hw_mat(isParamValid("htc") ? getKokkosMaterialProperty<Real>("htc")
                                : Moose::Kokkos::MaterialProperty<Real>())
{
  if (isCoupled("htc_var") == isParamValid("htc"))
    paramError("htc", "Either htc_var OR htc must be provided (exactly one, not both).");
  if (isCoupled("T_fluid_var") == isParamValid("T_fluid"))
    paramError("T_fluid",
               "Either ",
               "T_fluid",
               " OR ",
               "T_fluid_var",
               " must be provided (exactly one, not both).");
}
