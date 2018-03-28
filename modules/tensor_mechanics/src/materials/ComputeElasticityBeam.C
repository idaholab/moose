/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeElasticityBeam.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", ComputeElasticityBeam);

template <>
InputParameters
validParams<ComputeElasticityBeam>()
{
  InputParameters params = validParams<Material>();
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
      "shear_modulus",
      "Shear modulus of the material. Can be supplied as either a number or a variable name.");
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
                                                             : NULL),
    _youngs_modulus(coupledValue("youngs_modulus")),
    _shear_modulus(coupledValue("shear_modulus")),
    _shear_coefficient(coupledValue("shear_coefficient"))
{
}

void
ComputeElasticityBeam::initQpStatefulProperties()
{
  // material_stiffness relates the translational strains to forces
  _material_stiffness[_qp](0) = _youngs_modulus[_qp];
  _material_stiffness[_qp](1) = _shear_coefficient[_qp] * _shear_modulus[_qp];
  _material_stiffness[_qp](2) = _material_stiffness[_qp](1);

  // material flexure relates the rotational strains to moments
  _material_flexure[_qp](0) = _shear_coefficient[_qp] * _shear_modulus[_qp];
  _material_flexure[_qp](1) = _youngs_modulus[_qp];
  _material_flexure[_qp](2) = _material_flexure[_qp](1);
}

void
ComputeElasticityBeam::computeQpProperties()
{
  // Multiply by prefactor
  if (_prefactor_function)
  {
    _material_stiffness[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);
    _material_flexure[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);
  }
}
