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
  params.addParam<FunctionName>(
      "elasticity_prefactor",
      "Optional function to use as a scalar prefactor on the elasticity vector for the truss.");
  params.addRequiredCoupledVar(
      "youngs_modulus",
      "Young's modulus of the material. Can be supplied as either a number or a variable name.");
  return params;
}

ComputeElasticityTruss::ComputeElasticityTruss(const InputParameters & parameters)
  : Material(parameters),
    _material_stiffness(declareProperty<RealVectorValue>("material_stiffness")),
    _prefactor_function(isParamValid("elasticity_prefactor") ? &getFunction("elasticity_prefactor")
                                                             : nullptr),
    _youngs_modulus(coupledValue("youngs_modulus"))
{
}

void
ComputeElasticityTruss::computeQpProperties()
{
  // out<<" in ComputeElasticityTruss" << std::endl;

  // material_stiffness relates the translational strains to forces
  _material_stiffness[_qp](0) = _youngs_modulus[_qp];

  // Multiply by prefactor
  if (_prefactor_function)
  {
    _material_stiffness[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);
  }
}
