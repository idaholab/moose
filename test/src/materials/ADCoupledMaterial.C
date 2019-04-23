//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledMaterial.h"

registerADMooseObject("MooseTestApp", ADCoupledMaterial);

defineADValidParams(
    ADCoupledMaterial, ADMaterial, params.addRequiredCoupledVar("coupled_var", "A coupledvariable");
    params.addRequiredParam<MaterialPropertyName>("ad_mat_prop",
                                                  "Name of the ad property this material defines");
    params.addRequiredParam<MaterialPropertyName>(
        "regular_mat_prop", "Name of the regular property this material defines"););

template <ComputeStage compute_stage>
ADCoupledMaterial<compute_stage>::ADCoupledMaterial(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _ad_mat_prop(adDeclareADProperty<Real>(adGetParam<MaterialPropertyName>("ad_mat_prop"))),
    _regular_mat_prop(
        adDeclareProperty<Real>(adGetParam<MaterialPropertyName>("regular_mat_prop"))),
    _coupled_var(adCoupledValue("coupled_var"))
{
}

// Note that the structure of the two (uncommented) methods below are for testing purposes only;
// e.g. this material demonstrates that you get bad convergence when you drop the derivative
// information from the coupled variable. A production version of this material would look like
// this:
//
// template <ComputeStage compute_stage>
// void
// ADCoupledMaterial<compute_stage>::computeQpProperties()
// {
//   _ad_mat_prop[_qp] = 4.0 * _coupled_var[_qp];
// }

template <ComputeStage compute_stage>
void
ADCoupledMaterial<compute_stage>::computeQpProperties()
{
  _regular_mat_prop[_qp] = 4.0 * _coupled_var[_qp].value();
  _ad_mat_prop[_qp] = 4.0 * _coupled_var[_qp];
}

template <>
void
ADCoupledMaterial<RESIDUAL>::computeQpProperties()
{
  _regular_mat_prop[_qp] = 4.0 * _coupled_var[_qp];
  _ad_mat_prop[_qp] = 4.0 * _coupled_var[_qp];
}
