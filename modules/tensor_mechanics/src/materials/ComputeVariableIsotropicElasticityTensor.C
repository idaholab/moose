/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeVariableIsotropicElasticityTensor.h"

template <>
InputParameters
validParams<ComputeVariableIsotropicElasticityTensor>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addClassDescription("Compute an isotropic elasticity tensor for elastic constants that "
                             "change as a function of material properties");
  params.addRequiredParam<MaterialPropertyName>("youngs_modulus",
                                                "Name of material defining the Young's Modulus");
  params.addRequiredParam<MaterialPropertyName>("poissons_ratio",
                                                "Name of material defining the Poisson's Ratio");
  params.addRequiredCoupledVar(
      "args", "Variable dependence for the Young's Modulus and Poisson's Ratio materials");
  return params;
}

ComputeVariableIsotropicElasticityTensor::ComputeVariableIsotropicElasticityTensor(
    const InputParameters & parameters)
  : ComputeElasticityTensorBase(parameters),
    _elasticity_tensor_old(declarePropertyOld<RankFourTensor>(_elasticity_tensor_name)),
    _youngs_modulus(getMaterialProperty<Real>("youngs_modulus")),
    _poissons_ratio(getMaterialProperty<Real>("poissons_ratio")),
    _num_args(coupledComponents("args")),
    _dyoungs_modulus(_num_args),
    _d2youngs_modulus(_num_args),
    _dpoissons_ratio(_num_args),
    _d2poissons_ratio(_num_args),
    _delasticity_tensor(_num_args),
    _d2elasticity_tensor(_num_args),
    _isotropic_elastic_constants(2)
{
  // fetch prerequisite derivatives and build elasticity tensor derivatives and cross-derivatives
  for (unsigned int i = 0; i < _num_args; ++i)
  {
    const VariableName & iname = getVar("args", i)->name();
    _dyoungs_modulus[i] = &getMaterialPropertyDerivative<Real>("youngs_modulus", iname);
    _dpoissons_ratio[i] = &getMaterialPropertyDerivative<Real>("poissons_ratio", iname);

    _delasticity_tensor[i] =
        &declarePropertyDerivative<RankFourTensor>(_elasticity_tensor_name, iname);

    _d2youngs_modulus[i].resize(_num_args);
    _d2poissons_ratio[i].resize(_num_args);
    _d2elasticity_tensor[i].resize(_num_args);

    for (unsigned int j = i; j < _num_args; ++j)
    {
      const VariableName & jname = getVar("args", j)->name();
      _d2youngs_modulus[i][j] =
          &getMaterialPropertyDerivative<Real>("youngs_modulus", iname, jname);
      _d2poissons_ratio[i][j] =
          &getMaterialPropertyDerivative<Real>("poissons_ratio", iname, jname);
      _d2elasticity_tensor[i][j] =
          &declarePropertyDerivative<RankFourTensor>(_elasticity_tensor_name, iname, jname);
    }
  }
}

void
ComputeVariableIsotropicElasticityTensor::initialSetup()
{
  validateCoupling<Real>("youngs_modulus");
  validateCoupling<Real>("poissons_ratio");
  for (unsigned int i = 0; i < _num_args; ++i)
  {
    const VariableName & iname = getVar("args", i)->name();
    if (_fe_problem.isMatPropRequested(propertyNameFirst(_elasticity_tensor_name, iname)))
      mooseError("Derivative of elasticity tensor requested, but not yet implemented");
    else
      _delasticity_tensor[i] = nullptr;
    for (unsigned int j = 0; j < _num_args; ++j)
    {
      const VariableName & jname = getVar("args", j)->name();
      if (_fe_problem.isMatPropRequested(propertyNameSecond(_elasticity_tensor_name, iname, jname)))
        mooseError("Second Derivative of elasticity tensor requested, but not yet implemented");
      else
        _d2elasticity_tensor[i][j] = nullptr;
    }
  }
}

void
ComputeVariableIsotropicElasticityTensor::initQpStatefulProperties()
{
}

void
ComputeVariableIsotropicElasticityTensor::computeQpElasticityTensor()
{
  // lambda
  _isotropic_elastic_constants[0] =
      _youngs_modulus[_qp] * _poissons_ratio[_qp] /
      ((1.0 + _poissons_ratio[_qp]) * (1.0 - 2.0 * _poissons_ratio[_qp]));
  // shear modulus
  _isotropic_elastic_constants[1] = _youngs_modulus[_qp] / (2.0 * (1.0 + _poissons_ratio[_qp]));

  _elasticity_tensor[_qp].fillFromInputVector(_isotropic_elastic_constants,
                                              RankFourTensor::symmetric_isotropic);

  // Define derivatives of the elasticity tensor
  for (unsigned int i = 0; i < _num_args; ++i)
  {
    if (_delasticity_tensor[i])
      mooseError("Derivative of elasticity tensor requested, but not yet implemented");

    for (unsigned int j = i; j < _num_args; ++j)
      if (_d2elasticity_tensor[i][j])
        mooseError("Second derivative of elasticity tensor requested, but not yet implemented");
  }
}
