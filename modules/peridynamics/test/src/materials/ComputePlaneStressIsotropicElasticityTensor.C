//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePlaneStressIsotropicElasticityTensor.h"
#include "ElasticityTensorTools.h"

registerMooseObject("PeridynamicsTestApp", ComputePlaneStressIsotropicElasticityTensor);

InputParameters
ComputePlaneStressIsotropicElasticityTensor::validParams()
{
  InputParameters params = ComputeIsotropicElasticityTensor::validParams();
  params.addClassDescription(
      "Class for computing a constant isotropic elasticity tensor for strong plane stress");

  return params;
}

ComputePlaneStressIsotropicElasticityTensor::ComputePlaneStressIsotropicElasticityTensor(
    const InputParameters & parameters)
  : ComputeIsotropicElasticityTensor(parameters)
{
}

void
ComputePlaneStressIsotropicElasticityTensor::residualSetup()
{
  ComputeIsotropicElasticityTensor::residualSetup();

  // Retrieve youngs_modulus and poissons_ratio from the general 3D elasticity_tensor
  Real youngs_modulus = ElasticityTensorTools::getIsotropicYoungsModulus(_Cijkl);
  Real poissons_ratio = ElasticityTensorTools::getIsotropicPoissonsRatio(_Cijkl);

  // Reconstructe the elasticity tensor specific for plane stress case
  _Cijkl.zero();
  _Cijkl(0, 0, 0, 0) = youngs_modulus / (1.0 - poissons_ratio * poissons_ratio);
  _Cijkl(1, 1, 1, 1) = _Cijkl(0, 0, 0, 0);
  _Cijkl(0, 0, 1, 1) = _Cijkl(0, 0, 0, 0) * poissons_ratio;
  _Cijkl(1, 1, 0, 0) = _Cijkl(0, 0, 1, 1);
  _Cijkl(0, 1, 0, 1) = youngs_modulus / 2.0 / (1.0 + poissons_ratio);
  _Cijkl(0, 1, 1, 0) = _Cijkl(0, 1, 0, 1);
  _Cijkl(1, 0, 0, 1) = _Cijkl(0, 1, 0, 1);
  _Cijkl(1, 0, 1, 0) = _Cijkl(0, 1, 0, 1);
}
