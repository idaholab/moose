//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeElasticityTruss.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", ComputeElasticityTruss);

InputParameters
ComputeElasticityTruss::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the equivalent of the elasticity tensor for the truss "
                             "element, which are vectors of material axial stiffness.");
  params.addRequiredCoupledVar(
      "youngs_modulus",
      "Young's modulus of the material. Can be supplied as either a number or a variable name.");
  return params;
}

ComputeElasticityTruss::ComputeElasticityTruss(const InputParameters & parameters)
  : Material(parameters),
    _material_stiffness(declareProperty<Real>("material_stiffness")),
    _youngs_modulus(coupledValue("youngs_modulus"))
{
}

void
ComputeElasticityTruss::computeQpProperties()
{
  _material_stiffness[_qp] = _youngs_modulus[_qp];
}
