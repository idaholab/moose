//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeVariableEigenstrain.h"

registerMooseObject("SolidMechanicsApp", ComputeVariableEigenstrain);
registerMooseObject("SolidMechanicsApp", ADComputeVariableEigenstrain);

template <bool is_ad>
InputParameters
ComputeVariableEigenstrainTempl<is_ad>::validParams()
{
  InputParameters params = ComputeEigenstrainTempl<is_ad>::validParams();
  params.addClassDescription(
      "Computes an eigenstrain whose scalar prefactor is supplied by a material property. The AD "
      "specialization obtains derivatives through the AD material property; the non-AD "
      "specialization uses derivative material properties to publish first and second derivatives "
      "of elastic_strain.");
  params.addRequiredCoupledVar("args",
                               "Variables on which the prefactor material property depends");
  return params;
}

template <bool is_ad>
ComputeVariableEigenstrainTempl<is_ad>::ComputeVariableEigenstrainTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainTempl<is_ad>>(parameters),
    _num_args(this->coupledComponents("args")),
    _arg_names(_num_args),
    _dprefactor(_num_args),
    _d2prefactor(_num_args),
    _delastic_strain(_num_args),
    _d2elastic_strain(_num_args)
{
  for (unsigned int i = 0; i < _num_args; ++i)
  {
    _arg_names[i] = this->coupledName("args", i);

    if constexpr (!is_ad)
    {
      _dprefactor[i] =
          &this->template getMaterialPropertyDerivative<Real>("prefactor", _arg_names[i]);
      _delastic_strain[i] = &this->template declarePropertyDerivative<RankTwoTensor>(
          this->_base_name + "elastic_strain", _arg_names[i]);

      _d2prefactor[i].resize(_num_args);
      _d2elastic_strain[i].resize(_num_args);

      for (unsigned int j = i; j < _num_args; ++j)
      {
        _d2prefactor[i][j] = &this->template getMaterialPropertyDerivative<Real>(
            "prefactor", _arg_names[i], _arg_names[j]);
        _d2elastic_strain[i][j] = &this->template declarePropertyDerivative<RankTwoTensor>(
            this->_base_name + "elastic_strain", _arg_names[i], _arg_names[j]);
      }
    }
  }
}

template <bool is_ad>
void
ComputeVariableEigenstrainTempl<is_ad>::computeQpEigenstrain()
{
  // Compute eigenstrain = eigen_base * prefactor. ComputeEigenstrainTempl retrieves prefactor as
  // GenericMaterialProperty<Real, is_ad>, so the AD specialization retains its derivative data.
  ComputeEigenstrainTempl<is_ad>::computeQpEigenstrain();

  if constexpr (!is_ad)
    for (unsigned int i = 0; i < _num_args; ++i)
    {
      // elastic_strain = mechanical_strain - eigenstrain
      (*_delastic_strain[i])[this->_qp] = -this->_eigen_base_tensor * (*_dprefactor[i])[this->_qp];

      for (unsigned int j = i; j < _num_args; ++j)
        (*_d2elastic_strain[i][j])[this->_qp] =
            -this->_eigen_base_tensor * (*_d2prefactor[i][j])[this->_qp];
    }
}

template class ComputeVariableEigenstrainTempl<false>;
template class ComputeVariableEigenstrainTempl<true>;
