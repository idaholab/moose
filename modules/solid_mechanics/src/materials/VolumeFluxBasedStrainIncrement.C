//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeFluxBasedStrainIncrement.h"
#include "libmesh/quadrature.h"

registerMooseObject("SolidMechanicsApp", VolumeFluxBasedStrainIncrement);

InputParameters
VolumeFluxBasedStrainIncrement::validParams()
{
  InputParameters params = FluxBasedStrainIncrement::validParams();
  params.addClassDescription("Compute volumetric strain increment based on flux");
  params.addRequiredParam<MaterialPropertyName>("Lambda_Prefactor_J", "Value of prefactor_J");
  params.addRequiredParam<MaterialPropertyName>("Lambda_Prefactor_P", "Value of prefactor_P");
  params.addRequiredParam<MaterialPropertyName>("Source", "Value of Source");
  return params;
}

VolumeFluxBasedStrainIncrement::VolumeFluxBasedStrainIncrement(const InputParameters & parameters)
  : FluxBasedStrainIncrement(parameters),
    _Lambda_prefactor_J(getMaterialProperty<Real>("Lambda_Prefactor_J")),
    _Lambda_prefactor_P(getMaterialProperty<Real>("Lambda_Prefactor_P")),
    _source(getMaterialProperty<Real>("Source"))
{
}

void
VolumeFluxBasedStrainIncrement::initQpStatefulProperties()
{
  _strain_increment[_qp].zero();
}

void
VolumeFluxBasedStrainIncrement::computeQpProperties()
{
  FluxBasedStrainIncrement::computeFluxGradTensor();
  auto strain_rate = ((_Lambda_prefactor_J[_qp]) * (_flux_grad_tensor.trace()) * _Identity_tensor) +
                     ((_Lambda_prefactor_P[_qp]) * _source[_qp] *
                      _Identity_tensor); // Lambda's are the relaxation factors and trace of flux
                                         // grad tensor gives divergence of flux.
  _strain_increment[_qp] = strain_rate * _dt;
}
