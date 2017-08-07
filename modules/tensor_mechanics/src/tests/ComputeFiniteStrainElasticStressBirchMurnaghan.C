/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeFiniteStrainElasticStressBirchMurnaghan.h"

template <>
InputParameters
validParams<ComputeFiniteStrainElasticStressBirchMurnaghan>()
{
  InputParameters params = validParams<ComputeBirchMurnaghanEquationOfStress>();
  params.addClassDescription("Compute stress using elasticity for finite strains,"
                             "add bulk viscosity damping and"
                             "substitute the volumetric part of the stress with"
                             "a Murnaghan equation of state");
  return params;
}

ComputeFiniteStrainElasticStressBirchMurnaghan::ComputeFiniteStrainElasticStressBirchMurnaghan(
    const InputParameters & parameters)
  : ComputeBirchMurnaghanEquationOfStress(parameters),
    GuaranteeConsumer(this),
    _strain_increment(getMaterialPropertyByName<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(
        getMaterialPropertyByName<RankTwoTensor>(_base_name + "rotation_increment")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "elastic_strain"))
{
}

void
ComputeFiniteStrainElasticStressBirchMurnaghan::initialSetup()
{
  if (!hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::ISOTROPIC))
    mooseError("ComputeFiniteStrainElasticStressBirchMurnaghan can only be used with elasticity "
               "tensor materials "
               "that guarantee isotropic tensors.");

  _is_elasticity_tensor_guaranteed_constant_in_time =
      hasGuaranteedMaterialProperty(_elasticity_tensor_name, Guarantee::CONSTANT_IN_TIME);
  if ((isParamValid("initial_stress")) && !_is_elasticity_tensor_guaranteed_constant_in_time)
    mooseError("A finite stress material cannot both have an initial stress and an elasticity "
               "tensor with varying values; please use a defined constant elasticity tensor, "
               "such as ComputeIsotropicElasticityTensor, if your model defines an initial "
               "stress, or apply an initial strain instead.");
}

void
ComputeFiniteStrainElasticStressBirchMurnaghan::initQpStatefulProperties()
{
  ComputeBirchMurnaghanEquationOfStress::initQpStatefulProperties();
}

void
ComputeFiniteStrainElasticStressBirchMurnaghan::computeQpStress()
{
  // Calculate the stress in the intermediate configuration
  RankTwoTensor intermediate_stress;

  // Check if the elasticity tensor has changed values
  if (!_is_elasticity_tensor_guaranteed_constant_in_time)
    intermediate_stress =
        _elasticity_tensor[_qp] * (_elastic_strain_old[_qp] + _strain_increment[_qp]);
  else
    intermediate_stress = _stress_old[_qp] + _elasticity_tensor[_qp] * _strain_increment[_qp];

  // Rotate the stress state to the current configuration
  _stress[_qp] =
      _rotation_increment[_qp] * intermediate_stress * _rotation_increment[_qp].transpose();

  // Assign value for elastic strain, which is equal to the mechanical strain
  _elastic_strain[_qp] = _mechanical_strain[_qp];

  // Compute dstress_dstrain
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp]; // This is NOT the exact jacobian
}
