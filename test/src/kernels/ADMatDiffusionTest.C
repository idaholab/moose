//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ADMatDiffusionTest.h"

registerADMooseObject("MooseTestApp", ADMatDiffusionTest);

defineADValidParams(
    ADMatDiffusionTest,
    ADKernel,
    params.addParam<MaterialPropertyName>(
        "ad_mat_prop",
        "ad_diffusivity",
        "the name of the AD material property we are going to use");
    params.addParam<MaterialPropertyName>(
        "regular_mat_prop",
        "regular_diffusivity",
        "the name of the AD material property we are going to use");
    MooseEnum prop_to_use("AdAd AdReg RegAd RegReg", "AdAd");
    params.addParam<MooseEnum>("prop_to_use",
                               prop_to_use,
                               "What type of property to use. The prefix indicates the getter type "
                               "in the kernel; the suffix indicates the declaration type in the "
                               "material."););

template <ComputeStage compute_stage>
ADMatDiffusionTest<compute_stage>::ADMatDiffusionTest(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _ad_diff_from_ad_prop(adGetADMaterialProperty<Real>("ad_mat_prop")),
    _regular_diff_from_ad_prop(adGetMaterialProperty<Real>("ad_mat_prop")),
    _ad_diff_from_regular_prop(adGetADMaterialProperty<Real>("regular_mat_prop")),
    _regular_diff_from_regular_prop(adGetMaterialProperty<Real>("regular_mat_prop")),
    _prop_to_use(adGetParam<MooseEnum>("prop_to_use"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADMatDiffusionTest<compute_stage>::computeQpResidual()
{
  if (_prop_to_use == "AdAd")
    return _ad_diff_from_ad_prop[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
  else if (_prop_to_use == "AdReg")
    return _ad_diff_from_regular_prop[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
  else if (_prop_to_use == "RegAd")
    return _regular_diff_from_ad_prop[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
  else if (_prop_to_use == "RegReg")
    return _regular_diff_from_regular_prop[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
  else
    mooseError("Oops");
}
