/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari, O. Heinonen

#include "FiniteStrainElasticMaterial.h"

/**
 * FiniteStrainElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

template<>
InputParameters validParams<FiniteStrainElasticMaterial>()
{
  InputParameters params = validParams<FiniteStrainMaterial>();
  params.addRequiredParam<std::vector<Real> >("C_ijkl", "Stiffness tensor for material");
  params.addParam<FunctionName>("elasticity_tensor_prefactor", "Optional function to use as a scalar prefactor on the elasticity tensor.");
  return params;
}

FiniteStrainElasticMaterial::FiniteStrainElasticMaterial(const InputParameters & parameters) :
  FiniteStrainMaterial(parameters),
  _Cijkl(getParam<std::vector<Real> >("C_ijkl"), (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method")),
  _prefactor_function(isParamValid("elasticity_tensor_prefactor") ? &getFunction("elasticity_tensor_prefactor") : NULL)
{
}

void FiniteStrainElasticMaterial::computeQpStress()
{
  //Update strain in intermediate configuration
  _total_strain[_qp] = _total_strain_old[_qp] + _strain_increment[_qp];

  //Rotate strain to current configuration
  _total_strain[_qp] = _rotation_increment[_qp] * _total_strain[_qp] * _rotation_increment[_qp].transpose();

  //For elastic problems elastic strain = total strain
  _elastic_strain[_qp]=_total_strain[_qp];

  // stress = C * e
  _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp]; //Calculate stress in intermediate configruation

  //Rotate the stress to the current configuration
  _stress[_qp] = _rotation_increment[_qp] * _stress[_qp] * _rotation_increment[_qp].transpose();

}

void FiniteStrainElasticMaterial::computeQpElasticityTensor()
{
  // Fill in the matrix stiffness material property
  RotationTensor R(_Euler_angles); // R type: RealTensorValue
  _elasticity_tensor[_qp] = _Cijkl;

  if (_prefactor_function)
    _elasticity_tensor[_qp] *= _prefactor_function->value(_t, _q_point[_qp]);

  _elasticity_tensor[_qp].rotate(R);
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}
