//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElasticEnergyMaterial.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PhaseFieldApp", ElasticEnergyMaterial);
registerMooseObject("PhaseFieldApp", ADElasticEnergyMaterial);

template <bool is_ad>
InputParameters
ElasticEnergyMaterialTempl<is_ad>::validParams()
{
  InputParameters params = DerivativeFunctionMaterialBaseTempl<is_ad>::validParams();
  params.addClassDescription("Free energy material for the elastic energy contributions.");
  params.addParam<std::string>("base_name", "Material property base name");

  // The non-AD object needs the variables with respect to which it explicitly
  // constructs first- and second-derivative material properties. The AD object
  // obtains its nonlinear-DOF derivatives from the AD tensor properties.
  if constexpr (!is_ad)
    params.addCoupledVar("coupled_variables",
                         "Vector of variable arguments of the free energy function");

  params.addCoupledVar("displacement_gradients",
                       "Vector of displacement gradient variables (see "
                       "Modules/PhaseField/DisplacementGradients "
                       "action)");
  return params;
}

template <bool is_ad>
ElasticEnergyMaterialTempl<is_ad>::ElasticEnergyMaterialTempl(const InputParameters & parameters)
  : DerivativeFunctionMaterialBaseTempl<is_ad>(parameters),
    _base_name(this->isParamValid("base_name")
                   ? this->template getParam<std::string>("base_name") + "_"
                   : ""),
    _stress(this->template getGenericMaterialPropertyByName<RankTwoTensor, is_ad>(_base_name +
                                                                                  "stress")),
    _elasticity_tensor(this->template getGenericMaterialPropertyByName<RankFourTensor, is_ad>(
        _base_name + "elasticity_tensor")),
    _strain(this->template getGenericMaterialPropertyByName<RankTwoTensor, is_ad>(_base_name +
                                                                                  "elastic_strain"))
{
  // Explicit tensor-property derivatives are required only by the non-AD
  // specialization. The AD specialization differentiates _stress and _strain
  // through their AD dependency chains.
  if constexpr (!is_ad)
  {
    _dstrain.resize(_nargs);
    _d2strain.resize(_nargs);
    _delasticity_tensor.resize(_nargs);
    _d2elasticity_tensor.resize(_nargs);

    for (unsigned int i = 0; i < _nargs; ++i)
    {
      _dstrain[i] = &this->template getMaterialPropertyDerivativeByName<RankTwoTensor>(
          _base_name + "elastic_strain", _arg_names[i]);
      _delasticity_tensor[i] = &this->template getMaterialPropertyDerivativeByName<RankFourTensor>(
          _base_name + "elasticity_tensor", _arg_names[i]);

      _d2strain[i].resize(_nargs);
      _d2elasticity_tensor[i].resize(_nargs);

      for (unsigned int j = 0; j < _nargs; ++j)
      {
        _d2strain[i][j] = &this->template getMaterialPropertyDerivativeByName<RankTwoTensor>(
            _base_name + "elastic_strain", _arg_names[i], _arg_names[j]);
        _d2elasticity_tensor[i][j] =
            &this->template getMaterialPropertyDerivativeByName<RankFourTensor>(
                _base_name + "elasticity_tensor", _arg_names[i], _arg_names[j]);
      }
    }
  }
}

template <bool is_ad>
void
ElasticEnergyMaterialTempl<is_ad>::initialSetup()
{
  this->template validateCoupling<RankTwoTensor>(_base_name + "elastic_strain");
  this->template validateCoupling<RankFourTensor>(_base_name + "elasticity_tensor");
}

template <bool is_ad>
GenericReal<is_ad>
ElasticEnergyMaterialTempl<is_ad>::computeF()
{
  return 0.5 * _stress[_qp].doubleContraction(_strain[_qp]);
}

template <bool is_ad>
GenericReal<is_ad>
ElasticEnergyMaterialTempl<is_ad>::computeDF(unsigned int i_var)
{
  if constexpr (is_ad)
  {
    libmesh_ignore(i_var);
    return 0.0;
  }
  else
  {
    const unsigned int i = this->argIndex(i_var);

    return 0.5 * ((*_delasticity_tensor[i])[_qp] * _strain[_qp]).doubleContraction(_strain[_qp]) +
           (_elasticity_tensor[_qp] * (*_dstrain[i])[_qp]).doubleContraction(_strain[_qp]);
  }
}

template <bool is_ad>
GenericReal<is_ad>
ElasticEnergyMaterialTempl<is_ad>::computeD2F(unsigned int i_var, unsigned int j_var)
{
  if constexpr (is_ad)
  {
    libmesh_ignore(i_var);
    libmesh_ignore(j_var);
    return 0.0;
  }
  else
  {
    const unsigned int i = this->argIndex(i_var);
    const unsigned int j = this->argIndex(j_var);
    return 0.5 * (((*_d2elasticity_tensor[i][j])[_qp] * _strain[_qp] +
                   (*_delasticity_tensor[i])[_qp] * (*_dstrain[j])[_qp] +
                   (*_delasticity_tensor[j])[_qp] * (*_dstrain[i])[_qp] +
                   _elasticity_tensor[_qp] * (*_d2strain[i][j])[_qp])
                      .doubleContraction(_strain[_qp]) +
                  ((*_delasticity_tensor[i])[_qp] * _strain[_qp] +
                   _elasticity_tensor[_qp] * (*_dstrain[i])[_qp])
                      .doubleContraction((*_dstrain[j])[_qp]) +
                  ((*_delasticity_tensor[j])[_qp] * _strain[_qp] +
                   _elasticity_tensor[_qp] * (*_dstrain[j])[_qp])
                      .doubleContraction((*_dstrain[i])[_qp]) +
                  _stress[_qp].doubleContraction((*_d2strain[i][j])[_qp]));
  }
}

template class ElasticEnergyMaterialTempl<false>;
template class ElasticEnergyMaterialTempl<true>;
