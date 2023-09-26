#include "ConvectiveHeatTransferSideIntegral.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("HeatConductionApp", ConvectiveHeatTransferSideIntegral);
registerMooseObject("HeatConductionApp", ADConvectiveHeatTransferSideIntegral);

template <bool is_ad>
InputParameters
ConvectiveHeatTransferSideIntegralTempl<is_ad>::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription("Computes the total convective heat transfer across a boundary.");

  params.addRequiredCoupledVar("T_solid", "The solid temperature.");
  params.addCoupledVar("T_fluid_var", "The fluid temperature.");
  params.addCoupledVar("htc_var", "HTC variable");
  params.addParam<MaterialPropertyName>("T_fluid",
                                        "Name of the fluid temperature material property");
  params.addParam<MaterialPropertyName>("htc", "Name of alpha_wall material property");
  return params;
}

template <bool is_ad>
ConvectiveHeatTransferSideIntegralTempl<is_ad>::ConvectiveHeatTransferSideIntegralTempl(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _T_wall(coupledValue("T_solid")),
    _T_fluid(isCoupled("T_fluid_var") ? &coupledValue("T_fluid_var") : nullptr),
    _T_fluid_mat(isParamValid("T_fluid") ? &getGenericMaterialProperty<Real, is_ad>("T_fluid")
                                         : nullptr),
    _hw(isCoupled("htc_var") ? &coupledValue("htc_var") : nullptr),
    _hw_mat(isParamValid("htc") ? &getGenericMaterialProperty<Real, is_ad>("htc") : nullptr)
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

template <bool is_ad>
Real
ConvectiveHeatTransferSideIntegralTempl<is_ad>::computeQpIntegral()
{
  Real hw;
  if (_hw)
    hw = (*_hw)[_qp];
  else
    hw = MetaPhysicL::raw_value((*_hw_mat)[_qp]);

  Real Tf;
  if (_T_fluid)
    Tf = (*_T_fluid)[_qp];
  else
    Tf = MetaPhysicL::raw_value((*_T_fluid_mat)[_qp]);

  return hw * (_T_wall[_qp] - Tf);
}

template class ConvectiveHeatTransferSideIntegralTempl<false>;
template class ConvectiveHeatTransferSideIntegralTempl<true>;
