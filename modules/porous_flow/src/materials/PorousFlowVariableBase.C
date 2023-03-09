//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowVariableBase.h"

template <bool is_ad>
InputParameters
PorousFlowVariableBaseTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowMaterial::validParams();
  params.addPrivateParam<std::string>("pf_material_type", "pressure_saturation");
  params.addClassDescription("Base class for thermophysical variable materials. Provides pressure "
                             "and saturation material properties for all phases as required");
  return params;
}

template <bool is_ad>
PorousFlowVariableBaseTempl<is_ad>::PorousFlowVariableBaseTempl(const InputParameters & parameters)
  : DerivativeMaterialInterface<PorousFlowMaterial>(parameters),

    _num_phases(_dictator.numPhases()),
    _num_components(_dictator.numComponents()),
    _num_pf_vars(_dictator.numVariables()),

    _porepressure(
        _nodal_material
            ? declareGenericProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_nodal")
            : declareGenericProperty<std::vector<Real>, is_ad>("PorousFlow_porepressure_qp")),
    _dporepressure_dvar(is_ad             ? nullptr
                        : _nodal_material ? &declareProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_porepressure_nodal_dvar")
                                          : &declareProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_porepressure_qp_dvar")),
    _gradp_qp((_nodal_material || is_ad)
                  ? nullptr
                  : &declareProperty<std::vector<RealGradient>>("PorousFlow_grad_porepressure_qp")),
    _dgradp_qp_dgradv((_nodal_material || is_ad)
                          ? nullptr
                          : &declareProperty<std::vector<std::vector<Real>>>(
                                "dPorousFlow_grad_porepressure_qp_dgradvar")),
    _dgradp_qp_dv((_nodal_material || is_ad)
                      ? nullptr
                      : &declareProperty<std::vector<std::vector<RealGradient>>>(
                            "dPorousFlow_grad_porepressure_qp_dvar")),

    _saturation(
        _nodal_material
            ? declareGenericProperty<std::vector<Real>, is_ad>("PorousFlow_saturation_nodal")
            : declareGenericProperty<std::vector<Real>, is_ad>("PorousFlow_saturation_qp")),
    _dsaturation_dvar(
        is_ad ? nullptr
        : _nodal_material
            ? &declareProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_nodal_dvar")
            : &declareProperty<std::vector<std::vector<Real>>>("dPorousFlow_saturation_qp_dvar")),
    _grads_qp((_nodal_material || is_ad)
                  ? nullptr
                  : &declareProperty<std::vector<RealGradient>>("PorousFlow_grad_saturation_qp")),
    _dgrads_qp_dgradv((_nodal_material || is_ad) ? nullptr
                                                 : &declareProperty<std::vector<std::vector<Real>>>(
                                                       "dPorousFlow_grad_saturation_qp_dgradvar")),
    _dgrads_qp_dv((_nodal_material || is_ad)
                      ? nullptr
                      : &declareProperty<std::vector<std::vector<RealGradient>>>(
                            "dPorousFlow_grad_saturation_qp_dvar"))
{
}

template <bool is_ad>
void
PorousFlowVariableBaseTempl<is_ad>::initQpStatefulProperties()
{
  _porepressure[_qp].resize(_num_phases);
  _saturation[_qp].resize(_num_phases);
  // the porepressure and saturation values get set by derived classes
}

template <bool is_ad>
void
PorousFlowVariableBaseTempl<is_ad>::computeQpProperties()
{
  // do we really need this stuff here?  it seems very inefficient to keep resizing everything!
  _porepressure[_qp].resize(_num_phases);
  _saturation[_qp].resize(_num_phases);

  if (!is_ad)
  {
    (*_dporepressure_dvar)[_qp].resize(_num_phases);
    (*_dsaturation_dvar)[_qp].resize(_num_phases);

    if (!_nodal_material)
    {
      (*_gradp_qp)[_qp].resize(_num_phases);
      (*_dgradp_qp_dgradv)[_qp].resize(_num_phases);
      (*_dgradp_qp_dv)[_qp].resize(_num_phases);

      (*_grads_qp)[_qp].resize(_num_phases);
      (*_dgrads_qp_dgradv)[_qp].resize(_num_phases);
      (*_dgrads_qp_dv)[_qp].resize(_num_phases);
    }

    // Prepare the derivative matrices with zeroes
    for (unsigned phase = 0; phase < _num_phases; ++phase)
    {

      (*_dporepressure_dvar)[_qp][phase].assign(_num_pf_vars, 0.0);
      (*_dsaturation_dvar)[_qp][phase].assign(_num_pf_vars, 0.0);
      if (!_nodal_material)
      {
        (*_dgradp_qp_dgradv)[_qp][phase].assign(_num_pf_vars, 0.0);
        (*_dgradp_qp_dv)[_qp][phase].assign(_num_pf_vars, RealGradient());
        (*_dgrads_qp_dgradv)[_qp][phase].assign(_num_pf_vars, 0.0);
        (*_dgrads_qp_dv)[_qp][phase].assign(_num_pf_vars, RealGradient());
      }
    }
  }
}

template class PorousFlowVariableBaseTempl<false>;
template class PorousFlowVariableBaseTempl<true>;
