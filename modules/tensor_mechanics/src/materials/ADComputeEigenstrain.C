//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeEigenstrain.h"

#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ADComputeEigenstrain);

InputParameters
ADComputeEigenstrain::validParams()
{
  InputParameters params = ADComputeEigenstrainBase::validParams();
  params.addClassDescription("Computes a constant Eigenstrain");
  params.addRequiredParam<std::vector<Real>>(
      "eigen_base", "Vector of values defining the constant base tensor for the Eigenstrain");
  params.addParam<MaterialPropertyName>(
      "prefactor", 1.0, "Name of material defining the variable dependence");
  return params;
}

ADComputeEigenstrain::ADComputeEigenstrain(const InputParameters & parameters)
  : ADComputeEigenstrainBase(parameters), _prefactor(getADMaterialProperty<Real>("prefactor"))
{
  _eigen_base_tensor.fillFromInputVector(getParam<std::vector<Real>>("eigen_base"));
}

void
ADComputeEigenstrain::computeQpEigenstrain()
{
  // Define Eigenstrain
  _eigenstrain[_qp] = _eigen_base_tensor * _prefactor[_qp];
}
