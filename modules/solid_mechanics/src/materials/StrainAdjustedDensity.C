//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StrainAdjustedDensity.h"

registerMooseObject("SolidMechanicsApp", StrainAdjustedDensity);
registerMooseObject("SolidMechanicsApp", ADStrainAdjustedDensity);

template <bool is_ad>
InputParameters
StrainAdjustedDensityTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system. If "
      "displacements are not coupled, a different material class such as GenericMaterialProperty "
      "or ParsedMaterial should be used.");

  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple material systems on the same block, "
                               "e.g. for multiple phases");
  params.addRequiredParam<MaterialPropertyName>("strain_free_density",
                                                "Material property for strain-free density");
  params.addClassDescription("Creates density material property");

  return params;
}

template <bool is_ad>
StrainAdjustedDensityTempl<is_ad>::StrainAdjustedDensityTempl(const InputParameters & parameters)
  : Material(parameters),
    _coord_system(getBlockCoordSystem()),
    _disp_r(this->template coupledGenericValue<is_ad>("displacements", 0)),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _strain_free_density(
        this->template getGenericMaterialProperty<Real, is_ad>("strain_free_density")),
    _grad_disp(this->template coupledGenericGradients<is_ad>("displacements")),
    _density(declareGenericProperty<Real, is_ad>(_base_name + "density"))
{
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "StrainAdjustedDensity needs to act on an undisplaced mesh. Use of a displaced mesh "
               "leads to "
               "incorrect gradient values");

  // fill remaining components with zero
  _grad_disp.resize(3, &genericZeroGradient<is_ad>());
}

template <bool is_ad>
void
StrainAdjustedDensityTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
StrainAdjustedDensityTempl<is_ad>::computeQpProperties()
{
  auto A = GenericRankTwoTensor<is_ad>::initializeFromRows(
      (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);
  A.addIa(1.0);

  switch (_coord_system)
  {
    case Moose::COORD_XYZ:
      break;

    case Moose::COORD_RZ:
      if (_q_point[_qp](0) != 0.0)
        A(2, 2) = _disp_r[_qp] / _q_point[_qp](0) + 1.0;
      break;

    case Moose::COORD_RSPHERICAL:
      if (_q_point[_qp](0) != 0.0)
        A(1, 1) = A(2, 2) = _disp_r[_qp] / _q_point[_qp](0) + 1.0;
      break;
  }

  _density[_qp] = _strain_free_density[_qp] / A.det();
}

template class StrainAdjustedDensityTempl<false>;
template class StrainAdjustedDensityTempl<true>;
