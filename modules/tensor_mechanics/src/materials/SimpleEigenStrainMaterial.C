/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
// Original class author: A.M. Jokisaari, O. Heinonen

#include "SimpleEigenStrainMaterial.h"

template <>
InputParameters
validParams<SimpleEigenStrainMaterial>()
{
  InputParameters params = validParams<EigenStrainBaseMaterial>();
  params.addClassDescription("Compute a concentration dependent isotropic Eigenstrain");
  params.addRequiredParam<Real>("epsilon0", "Initial eigen strain value");
  params.addParam<Real>("c0", 0.0, "Initial concentration value");
  params.addRequiredCoupledVar("c", "Concentration");
  return params;
}

SimpleEigenStrainMaterial::SimpleEigenStrainMaterial(const InputParameters & parameters)
  : EigenStrainBaseMaterial(parameters),
    _epsilon0(getParam<Real>("epsilon0")),
    _c0(getParam<Real>("c0"))
{
  mooseDeprecated("SimpleEigenStrainMaterial is deprecated. Refer to "
                  "http://mooseframework.org/wiki/PhysicsModules/TensorMechanics/Deprecations/"
                  "SimpleEigenStrainMaterial to convert this input file.");
}

void
SimpleEigenStrainMaterial::computeEigenStrain()
{
  _eigenstrain[_qp].zero();
  _eigenstrain[_qp].addIa(_epsilon0 * (_c[_qp] - _c0));

  // first derivative w.r.t. c
  _delastic_strain_dc[_qp].zero();
  _delastic_strain_dc[_qp].addIa(-_epsilon0); // delastic_strain/dc = -deigenstrain/dc

  // second derivative w.r.t. c (vanishes)
  _d2elastic_strain_dc2[_qp].zero();
}

void
SimpleEigenStrainMaterial::computeQpElasticityTensor()
{
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp] = _Cijkl;

  // the elasticity tensor is independent of c
  _delasticity_tensor_dc[_qp].zero();
  _d2elasticity_tensor_dc2[_qp].zero();
}
