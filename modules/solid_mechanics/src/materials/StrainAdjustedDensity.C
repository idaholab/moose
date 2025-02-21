//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  _density[_qp] = _strain_free_density[_qp];
}

template <bool is_ad>
void
StrainAdjustedDensityTempl<is_ad>::computeQpProperties()
{
  _density[_qp] = _strain_free_density[_qp];

  const auto Axx = (*_grad_disp[0])[_qp](0) + 1.0;
  const auto & Axy = (*_grad_disp[0])[_qp](1);
  const auto & Axz = (*_grad_disp[0])[_qp](2);
  const auto & Ayx = (*_grad_disp[1])[_qp](0);
  auto Ayy = (*_grad_disp[1])[_qp](1) + 1.0;
  const auto & Ayz = (*_grad_disp[1])[_qp](2);
  const auto & Azx = (*_grad_disp[2])[_qp](0);
  const auto & Azy = (*_grad_disp[2])[_qp](1);
  auto Azz = (*_grad_disp[2])[_qp](2) + 1.0;

  switch (_coord_system)
  {
    case Moose::COORD_XYZ:
      Azz = (*_grad_disp[2])[_qp](2) + 1.0;
      break;

    case Moose::COORD_RZ:
      if (_q_point[_qp](0) != 0.0)
        Azz = _disp_r[_qp] / _q_point[_qp](0) + 1.0;
      break;

    case Moose::COORD_RSPHERICAL:
      if (_q_point[_qp](0) != 0.0)
        Ayy = Azz = _disp_r[_qp] / _q_point[_qp](0) + 1.0;
      break;
  }

  const auto detF = Axx * Ayy * Azz + Axy * Ayz * Azx + Axz * Ayx * Azy - Azx * Ayy * Axz -
                    Azy * Ayz * Axx - Azz * Ayx * Axy;
  _density[_qp] /= detF;
}

template class StrainAdjustedDensityTempl<false>;
template class StrainAdjustedDensityTempl<true>;
