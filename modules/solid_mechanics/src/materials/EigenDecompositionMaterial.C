//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EigenDecompositionMaterial.h"
#include "RankTwoTensor.h"

registerMooseObject("SolidMechanicsApp", EigenDecompositionMaterial);
registerMooseObject("SolidMechanicsApp", ADEigenDecompositionMaterial);

template <bool is_ad>
InputParameters
EigenDecompositionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Emits material properties for the eigenvalues and eigenvectors of a "
                             "symmetric rank two tensor.");
  params.addRequiredParam<MaterialPropertyName>(
      "rank_two_tensor",
      "The name of the symmetric rank two tensor to used in eigen decomposition.");
  params.addParam<std::string>(
      "base_name",
      "Optional parameter to allow multiple tensors to be decomposed on the same block.");
  return params;
}

template <bool is_ad>
EigenDecompositionMaterialTempl<is_ad>::EigenDecompositionMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _max_eigen_vector(
        declareGenericProperty<RealVectorValue, is_ad>(_base_name + "max_eigen_vector")),
    _mid_eigen_vector(
        declareGenericProperty<RealVectorValue, is_ad>(_base_name + "mid_eigen_vector")),
    _min_eigen_vector(
        declareGenericProperty<RealVectorValue, is_ad>(_base_name + "min_eigen_vector")),
    _max_eigen_value(declareGenericProperty<Real, is_ad>(_base_name + "max_eigen_value")),
    _mid_eigen_value(declareGenericProperty<Real, is_ad>(_base_name + "mid_eigen_value")),
    _min_eigen_value(declareGenericProperty<Real, is_ad>(_base_name + "min_eigen_value"))
{
  if (LIBMESH_DIM != 3)
    mooseError("EigenDecompositionMaterial is only defined for LIBMESH_DIM=3");
}

template <bool is_ad>
void
EigenDecompositionMaterialTempl<is_ad>::computeQpProperties()
{

  if (!_tensor[_qp].isSymmetric())
    mooseError("EigenDecompositionMaterial will only operate on symmetric rank two tensors.");

  std::vector<GenericReal<is_ad>> eigval(3, 0.0);
  GenericRankTwoTensor<is_ad> eigvec;

  _tensor[_qp].symmetricEigenvaluesEigenvectors(eigval, eigvec);

  _max_eigen_vector[_qp] = eigvec.column(2);
  _mid_eigen_vector[_qp] = eigvec.column(1);
  _min_eigen_vector[_qp] = eigvec.column(0);

  _max_eigen_value[_qp] = eigval[2];
  _mid_eigen_value[_qp] = eigval[1];
  _min_eigen_value[_qp] = eigval[0];
}
