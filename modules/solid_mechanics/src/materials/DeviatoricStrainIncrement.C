//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeviatoricStrainIncrement.h"
#include "libmesh/quadrature.h"

registerMooseObject("SolidMechanicsApp", DeviatoricStrainIncrement);

InputParameters
DeviatoricStrainIncrement::validParams()
{
  InputParameters params = FluxBasedStrainIncrement::validParams();
  params.addClassDescription("Compute deviatoric strain increment based on flux");
  params.addRequiredParam<Real>("dimension","Dimensionality of the problem"); 
  return params;
}

DeviatoricStrainIncrement::DeviatoricStrainIncrement(const InputParameters & parameters)
  : FluxBasedStrainIncrement(parameters),
    n(getParam<Real>("dimension"))
{
}

void
DeviatoricStrainIncrement::computeQpProperties()
{
  FluxBasedStrainIncrement::computeQpProperties();
  
  computeIdentityTensor();

  _strain_increment[_qp] += (1.0/n) * ((_flux_grad_tensor.trace()) * _Identity_tensor) * (1.0 - _gb[_qp]) * _dt;

}

void
DeviatoricStrainIncrement::computeIdentityTensor()
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
