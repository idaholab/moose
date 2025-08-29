//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeviatoricFluxBasedStrainIncrement.h"
#include "libmesh/quadrature.h"

registerMooseObject("SolidMechanicsApp", DeviatoricFluxBasedStrainIncrement);

InputParameters
DeviatoricFluxBasedStrainIncrement::validParams()
{
  InputParameters params = FluxBasedStrainIncrement::validParams();
  params.addClassDescription("Compute deviatoric strain increment based on flux");
  params.addRequiredParam<Real>("dimension", "Dimensionality of the problem");
  return params;
}

DeviatoricFluxBasedStrainIncrement::DeviatoricFluxBasedStrainIncrement(
    const InputParameters & parameters)
  : FluxBasedStrainIncrement(parameters), n(getParam<Real>("dimension"))
{
}

void
DeviatoricFluxBasedStrainIncrement::computeQpProperties()
{
  FluxBasedStrainIncrement::computeQpProperties();

  _strain_increment[_qp] += (1.0 / n) *
                            ((_flux_grad_tensor.trace()) * (1.0 - _gb[_qp]) * _Identity_tensor) *
                            _dt; // This is the hydrostatic part of strain rate tensor
}
