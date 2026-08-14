//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowVolumetricStrain.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowVolumetricStrain);
registerMooseObject("PorousFlowApp", ADPorousFlowVolumetricStrain);

template <bool is_ad>
InputParameters
PorousFlowVolumetricStrainTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addParam<std::string>("base_name",
                               "This should be the same base_name as given to the TensorMechanics "
                               "object that computes strain");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addClassDescription(
      "Compute volumetric strain and the volumetric_strain rate, for use in PorousFlow.");
  params.set<std::string>("pf_material_type") = "volumetric_strain";
  params.set<bool>("stateful_displacements") = true;
  params.set<bool>("at_nodes") = false;
  return params;
}

template <bool is_ad>
PorousFlowVolumetricStrainTempl<is_ad>::PorousFlowVolumetricStrainTempl(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _total_strain(getGenericMaterialProperty<RankTwoTensor, is_ad>(_base_name + "total_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _ndisp(coupledComponents("displacements")),
    _disp_var_num(coupledIndices("displacements")),

    _vol_strain_rate_qp(
        declareGenericProperty<Real, is_ad>("PorousFlow_volumetric_strain_rate_qp")),
    _dvol_strain_rate_qp_dvar(is_ad ? nullptr
                                    : &declareProperty<std::vector<RealGradient>>(
                                          "dPorousFlow_volumetric_strain_rate_qp_dvar")),
    _vol_total_strain_qp(
        declareGenericProperty<Real, is_ad>("PorousFlow_total_volumetric_strain_qp")),
    _dvol_total_strain_qp_dvar(is_ad ? nullptr
                                     : &declareProperty<std::vector<RealGradient>>(
                                           "dPorousFlow_total_volumetric_strain_qp_dvar"))
{
  if (_ndisp != _mesh.dimension())
    paramError("displacements", "The number of variables supplied must match the mesh dimension.");

  if (_nodal_material)
    mooseError("PorousFlowVolumetricStrain classes are only defined for at_nodes = false");
}

template <bool is_ad>
void
PorousFlowVolumetricStrainTempl<is_ad>::initQpStatefulProperties()
{
  _vol_total_strain_qp[_qp] = 0.0;
}

template <bool is_ad>
void
PorousFlowVolumetricStrainTempl<is_ad>::computeQpProperties()
{
  _vol_total_strain_qp[_qp] = _total_strain[_qp].trace();
  _vol_strain_rate_qp[_qp] = (_vol_total_strain_qp[_qp] - _total_strain_old[_qp].trace()) / _dt;

  if constexpr (!is_ad)
  {
    // The AD path carries the derivatives with respect to the displacement variables inside the
    // AD strain, so only the non-AD path needs to fill in these derivative material properties.
    (*_dvol_strain_rate_qp_dvar)[_qp].resize(_num_var, RealGradient());
    (*_dvol_total_strain_qp_dvar)[_qp].resize(_num_var, RealGradient());
    for (unsigned i = 0; i < _ndisp; ++i)
      if (_dictator.isPorousFlowVariable(_disp_var_num[i]))
      {
        // the i_th displacement is a PorousFlow variable
        const unsigned int pvar = _dictator.porousFlowVariableNum(_disp_var_num[i]);
        (*_dvol_strain_rate_qp_dvar)[_qp][pvar](i) = 1.0 / _dt;
        (*_dvol_total_strain_qp_dvar)[_qp][pvar](i) = 1.0;
      }
  }
}

template class PorousFlowVolumetricStrainTempl<false>;
template class PorousFlowVolumetricStrainTempl<true>;
