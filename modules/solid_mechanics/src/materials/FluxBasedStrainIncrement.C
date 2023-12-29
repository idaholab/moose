//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxBasedStrainIncrement.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", FluxBasedStrainIncrement);

InputParameters
FluxBasedStrainIncrement::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute strain increment based on flux");
  params.addRequiredCoupledVar("xflux", "x or 0-direction component of flux");
  params.addCoupledVar("yflux", "y or 1-direction component of flux");
  params.addCoupledVar("zflux", "z or 2-direction component of flux");
  params.addCoupledVar("gb", "Grain boundary order parameter");
  params.addRequiredParam<MaterialPropertyName>("property_name",
                                                "Name of diffusive strain increment property");
  return params;
}

FluxBasedStrainIncrement::FluxBasedStrainIncrement(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _grad_jx(&coupledGradient("xflux")),
    _has_yflux(isCoupled("yflux")),
    _has_zflux(isCoupled("zflux")),
    _grad_jy(_has_yflux ? &coupledGradient("yflux") : nullptr),
    _grad_jz(_has_zflux ? &coupledGradient("zflux") : nullptr),
    _gb(isCoupled("gb") ? coupledValue("gb") : _zero),
    _strain_increment(
        declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("property_name")))
{
}

void
FluxBasedStrainIncrement::initQpStatefulProperties()
{
  _strain_increment[_qp].zero();
}

void
FluxBasedStrainIncrement::computeQpProperties()
{
  computeFluxGradTensor();

  _strain_increment[_qp] = -0.5 * (_flux_grad_tensor + _flux_grad_tensor.transpose());
  _strain_increment[_qp] *= (1.0 - _gb[_qp]) * _dt;
}

void
FluxBasedStrainIncrement::computeFluxGradTensor()
{
  _flux_grad_tensor.zero();

  _flux_grad_tensor.fillRow(0, (*_grad_jx)[_qp]);

  if (_has_yflux)
    _flux_grad_tensor.fillRow(1, (*_grad_jy)[_qp]);

  if (_has_zflux)
    _flux_grad_tensor.fillRow(2, (*_grad_jz)[_qp]);
}
