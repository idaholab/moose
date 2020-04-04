//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeElasticityBeam.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", ComputeElasticityBeam);

InputParameters
ComputeElasticityBeam::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Computes the equivalent of the elasticity tensor for the beam "
                             "element, which are vectors of material translational and flexural "
                             "stiffness.");
  params.addParam<FunctionName>(
      "elasticity_prefactor",
      "Optional function to use as a scalar prefactor on the elasticity vector for the beam.");
  params.addRequiredCoupledVar(
      "youngs_modulus",
      "Young's modulus of the material. Can be supplied as either a number or a variable name.");
  params.addRequiredCoupledVar(
      "poissons_ratio",
      "Poisson's ratio of the material. Can be supplied as either a number or a variable name.");
  params.addCoupledVar(
      "shear_coefficient",
      1.0,
      "Scale factor for the shear modulus. Can be supplied as either a number or a variable name.");
  return params;
}

ComputeElasticityBeam::ComputeElasticityBeam(const InputParameters & parameters)
  : Material(parameters),
    _material_stiffness(declareProperty<RealVectorValue>("material_stiffness")),
    _material_flexure(declareProperty<RealVectorValue>("material_flexure")),
    _prefactor_function(isParamValid("elasticity_prefactor") ? &getFunction("elasticity_prefactor")
                                                             : nullptr),
    _youngs_modulus(coupledValue("youngs_modulus")),
    _poissons_ratio(coupledValue("poissons_ratio")),
    _shear_coefficient(coupledValue("shear_coefficient"))
{
}

void
ComputeElasticityBeam::computeQpProperties()
{
  const Real shear_modulus = _youngs_modulus[_qp] / (2.0 * (1.0 + _poissons_ratio[_qp]));

  // material_stiffness relates the translational strains to forces
  _material_stiffness[_qp](0) = _youngs_modulus[_qp];
  _material_stiffness[_qp](1) = _shear_coefficient[_qp] * shear_modulus;
  _material_stiffness[_qp](2) = _material_stiffness[_qp](1);

  // material_flexure relates the rotational strains to moments
  _material_flexure[_qp](0) = _shear_coefficient[_qp] * shear_modulus;
  _material_flexure[_qp](1) = _youngs_modulus[_qp];
  _material_flexure[_qp](2) = _material_flexure[_qp](1);

  // Multiply by prefactor
  if (_prefactor_function)
  {
    _material_stiffness[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);
    _material_flexure[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);
  }
}
