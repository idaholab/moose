//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeStrainIncrement.h"
#include "libmesh/quadrature.h"

registerMooseObject("SolidMechanicsApp", VolumeStrainIncrement);

InputParameters
VolumeStrainIncrement::validParams()
{
  InputParameters params = FluxBasedStrainIncrement::validParams();
  params.addClassDescription("Compute volume strain increment based on divergence of flux and source of vacancies");
  params.addRequiredParam<MaterialPropertyName>("Lambda_Prefactor_J","Value of prefactor_J");
  params.addRequiredParam<MaterialPropertyName>("Lambda_Prefactor_P","Value of prefactor_P");
  params.addCoupledVar("TimederivativeConc", "Derivative of Concentration");
  params.addRequiredParam<MaterialPropertyName>("Source","Value of Source");
  return params;
}

VolumeStrainIncrement::VolumeStrainIncrement(const InputParameters & parameters)
  : FluxBasedStrainIncrement(parameters),
    _Lambda_prefactor_J(getMaterialProperty<Real>("Lambda_Prefactor_J")),
    _Lambda_prefactor_P(getMaterialProperty<Real>("Lambda_Prefactor_P")),
    _derivative_conc(coupledValue("TimederivativeConc")),
    _source(getMaterialProperty<Real>("Source"))

{
}

void
VolumeStrainIncrement::computeQpProperties()
{
  // FluxBasedStrainIncrement::computeQpProperties();
  computeIdentityTensor();

  // _strain_increment[_qp] = - ((_Lambda_prefactor_J[_qp])*(_flux_grad_tensor.trace())*_Identity_tensor) + ((_Lambda_prefactor_P[_qp])*_source[_qp]*_Identity_tensor);
  _strain_increment[_qp] = - ((_Lambda_prefactor_J[_qp])*(_derivative_conc[_qp] - _source[_qp])*_Identity_tensor) + ((_Lambda_prefactor_P[_qp])*_source[_qp]*_Identity_tensor); 
  _strain_increment[_qp] *= _dt;
}

void
VolumeStrainIncrement::computeIdentityTensor()
{
  RankTwoTensor iden(RankTwoTensor::initIdentity);
  _Identity_tensor.zero();

  _Identity_tensor.fillRow(0, iden.row(0));

  if (_has_yflux)
  {
    _Identity_tensor.fillRow(1, iden.row(1));
  }

  if (_has_zflux)
  {
    _Identity_tensor.fillRow(2, iden.row(2));
  }
}
