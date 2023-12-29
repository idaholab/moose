//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrain.h"

registerMooseObject("TensorMechanicsApp", ComputeEigenstrain);
registerMooseObject("TensorMechanicsApp", ADComputeEigenstrain);

template <bool is_ad>
InputParameters
ComputeEigenstrainTempl<is_ad>::validParams()
{
  InputParameters params = ComputeEigenstrainBase::validParams();
  params.addClassDescription("Computes a constant Eigenstrain");
  params.addRequiredParam<std::vector<Real>>(
      "eigen_base", "Vector of values defining the constant base tensor for the Eigenstrain");
  params.addParam<MaterialPropertyName>(
      "prefactor", 1.0, "Name of material property defining the variable dependence");
  return params;
}

template <bool is_ad>
ComputeEigenstrainTempl<is_ad>::ComputeEigenstrainTempl(const InputParameters & parameters)
  : ComputeEigenstrainBaseTempl<is_ad>(parameters),
    _prefactor(this->template getGenericMaterialProperty<Real, is_ad>("prefactor"))
{
  _eigen_base_tensor.fillFromInputVector(this->template getParam<std::vector<Real>>("eigen_base"));
}

template <bool is_ad>
void
ComputeEigenstrainTempl<is_ad>::computeQpEigenstrain()
{
  // Define Eigenstrain
  _eigenstrain[_qp] = _eigen_base_tensor * _prefactor[_qp];
}
