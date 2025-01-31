//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Density.h"

registerMooseObject("MiscApp", Density);
registerMooseObject("MiscApp", ADDensity);

template <bool is_ad>
InputParameters
DensityTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");

  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple material systems on the same block, "
                               "e.g. for multiple phases");
  params.addParam<Real>("density", "Density");
  params.deprecateParam("density", "strain_free_density", "02/01/2026");
  params.addParam<MaterialPropertyName>("strain_free_density",
                                        "Material property for the strain free density.");

  params.addClassDescription("Creates density material property");

  return params;
}

template <bool is_ad>
DensityTempl<is_ad>::DensityTempl(const InputParameters & parameters)
  : Material(parameters),
    _is_coupled(isCoupled("displacements")),
    _coord_system(getBlockCoordSystem()),
    _disp_r(_is_coupled ? this->template coupledGenericValue<is_ad>("displacements", 0)
                        : genericZeroValue<is_ad>()),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _initial_density(isParamValid("density") ? &getParam<Real>("density") : nullptr),
    _strain_free_density(
        isParamValid("strain_free_density")
            ? &this->template getGenericMaterialProperty<Real, is_ad>("strain_free_density")
            : nullptr),
    _grad_disp(_is_coupled ? this->template coupledGenericGradients<is_ad>("displacements")
                           : std::vector<const GenericVariableGradient<is_ad> *>()),
    _density(declareGenericProperty<Real, is_ad>(_base_name + "density"))
{
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "Density needs to act on an undisplaced mesh. Use of a displaced mesh leads to "
               "incorrect gradient values");

  // get coupled gradients
  const unsigned int ndisp = coupledComponents("displacements");

  if (ndisp == 0 && _fe_problem.getDisplacedProblem())
    paramError(
        "displacements",
        "The system uses a displaced problem but 'displacements' are not provided in Density.");

  // fill remaining components with zero
  _grad_disp.resize(3, &genericZeroGradient<is_ad>());

  if ((isParamValid("density") && isParamValid("strain_free_density")) ||
      (!isParamValid("density") && !isParamValid("strain_free_density")))
    paramError("strain_free_density",
               "Either density or strain_free_density must be supplied. Since density is "
               "deprecated, please provide a strain_free_density");
}

template <bool is_ad>
void
DensityTempl<is_ad>::initQpStatefulProperties()
{
  _density[_qp] = _initial_density ? *_initial_density : (*_strain_free_density)[_qp];
}

template <bool is_ad>
void
DensityTempl<is_ad>::computeQpProperties()
{
  _density[_qp] = _initial_density ? *_initial_density : (*_strain_free_density)[_qp];

  // TODO: We should deprecate the !_is_coupled case and have the
  // user use a GenericConstantMaterial
  if (_is_coupled)
  {
    // rho * V = rho0 * V0
    // rho = rho0 * V0 / V
    // rho = rho0 / det(F)
    // rho = rho0 / det(grad(u) + 1)

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
}

template class DensityTempl<false>;
template class DensityTempl<true>;
