/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeElasticityBeam.h"
#include "Function.h"

template <>
InputParameters
validParams<ComputeElasticityBeam>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<FunctionName>(
      "elasticity_prefactor",
      "Optional function to use as a scalar prefactor on the elasticity vector for the beam.");
  params.addCoupledVar("youngs_modulus", "Variable containing Youngs modulus");
  params.addCoupledVar("shear_modulus", "Variable contiaining shear modulus");
  params.addCoupledVar("shear_coefficient", "Reduction factor for the shear modulus");
  return params;
}

ComputeElasticityBeam::ComputeElasticityBeam(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _material_stiffness(declareProperty<RealVectorValue>(_base_name + "material_stiffness")),
    _material_flexure(declareProperty<RealVectorValue>(_base_name + "material_flexure")),
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
