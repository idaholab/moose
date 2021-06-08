//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPorosity.h"

registerMooseObject("PorousFlowApp", PorousFlowPorosity);

InputParameters
PorousFlowPorosity::validParams()
{
  InputParameters params = PorousFlowPorosityExponentialBase::validParams();
  params.addParam<bool>(
      "mechanical", false, "If true, porosity will be a function of total volumetric strain");
  params.addParam<bool>(
      "fluid", false, "If true, porosity will be a function of effective porepressure");
  params.addParam<bool>("thermal", false, "If true, porosity will be a function of temperature");
  params.addParam<bool>("chemical", false, "If true, porosity will be a function of precipitate");
  params.addRequiredCoupledVar("porosity_zero",
                               "The porosity at zero volumetric strain and "
                               "reference temperature and reference effective "
                               "porepressure and reference chemistry.  This must be a real number "
                               "or a constant monomial variable (not a linear lagrange or other "
                               "type of variable)");
  params.addParam<Real>("thermal_expansion_coeff",
                        "Volumetric thermal expansion coefficient of the drained porous solid "
                        "skeleton (only used if thermal=true)");
  params.addRangeCheckedParam<Real>(
      "biot_coefficient", 1, "biot_coefficient>=0 & biot_coefficient<=1", "Biot coefficient");
  params.addParam<Real>("biot_coefficient_prime",
                        "Biot coefficient that appears in the term (biot_coefficient_prime - 1) * "
                        "(P - reference_porepressure) / solid_bulk.  If not provided, this "
                        "defaults to the standard biot_coefficient");
  params.addRangeCheckedParam<Real>(
      "solid_bulk",
      "solid_bulk>0",
      "Bulk modulus of the drained porous solid skeleton (only used if fluid=true)");
  params.addCoupledVar(
      "reference_temperature", 0.0, "Reference temperature (only used if thermal=true)");
  params.addCoupledVar(
      "reference_porepressure", 0.0, "Reference porepressure (only used if fluid=true)");
  params.addCoupledVar("reference_chemistry",
                       "Reference values of the solid mineral concentrations "
                       "(m^3(precipitate)/m^3(porous material)), entered as "
                       "a vector (one value per mineral).  (Only used if chemical=true)");
  params.addCoupledVar(
      "initial_mineral_concentrations",
      "Initial mineral concentrations (m^3(precipitate)/m^3(porous material)), entered as "
      "a vector (one value per mineral).  (Only used if chemical=true)");
  params.addParam<std::vector<Real>>("chemical_weights",
                                     "When chemical=true, porosity is a linear combination of the "
                                     "solid mineral concentrations multiplied by these weights.  "
                                     "Default=1 for all minerals.");
  params.addClassDescription("This Material calculates the porosity PorousFlow simulations");
  return params;
}

PorousFlowPorosity::PorousFlowPorosity(const InputParameters & parameters)
  : PorousFlowPorosityExponentialBase(parameters),

    _mechanical(getParam<bool>("mechanical")),
    _fluid(getParam<bool>("fluid")),
    _thermal(getParam<bool>("thermal")),
    _chemical(getParam<bool>("chemical")),
    _phi0(coupledValue("porosity_zero")),
    _biot(getParam<Real>("biot_coefficient")),
    _exp_coeff(isParamValid("thermal_expansion_coeff") ? getParam<Real>("thermal_expansion_coeff")
                                                       : 0.0),
    _solid_bulk(isParamValid("solid_bulk") ? getParam<Real>("solid_bulk")
                                           : std::numeric_limits<Real>::max()),
    _coeff(isParamValid("biot_coefficient_prime")
               ? (getParam<Real>("biot_coefficient_prime") - 1.0) / _solid_bulk
               : (_biot - 1.0) / _solid_bulk),

    _t_reference(_nodal_material ? coupledDofValues("reference_temperature")
                                 : coupledValue("reference_temperature")),
    _p_reference(_nodal_material ? coupledDofValues("reference_porepressure")
                                 : coupledValue("reference_porepressure")),
    _num_c_ref(coupledComponents("reference_chemistry")),
    _c_reference(_num_c_ref),
    _num_initial_c(coupledComponents("initial_mineral_concentrations")),
    _initial_c(_num_initial_c),
    _c_weights(isParamValid("chemical_weights") ? getParam<std::vector<Real>>("chemical_weights")
                                                : std::vector<Real>(_num_c_ref, 1.0)),

    _porosity_old(_chemical ? (_nodal_material
                                   ? &getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")
                                   : &getMaterialPropertyOld<Real>("PorousFlow_porosity_qp"))
                            : nullptr),
    _vol_strain_qp(_mechanical ? &getMaterialProperty<Real>("PorousFlow_total_volumetric_strain_qp")
                               : nullptr),
    _dvol_strain_qp_dvar(_mechanical ? &getMaterialProperty<std::vector<RealGradient>>(
                                           "dPorousFlow_total_volumetric_strain_qp_dvar")
                                     : nullptr),

    _pf(_fluid ? (_nodal_material
                      ? &getMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_nodal")
                      : &getMaterialProperty<Real>("PorousFlow_effective_fluid_pressure_qp"))
               : nullptr),
    _dpf_dvar(_fluid ? (_nodal_material ? &getMaterialProperty<std::vector<Real>>(
                                              "dPorousFlow_effective_fluid_pressure_nodal_dvar")
                                        : &getMaterialProperty<std::vector<Real>>(
                                              "dPorousFlow_effective_fluid_pressure_qp_dvar"))
                     : nullptr),

    _temperature(_thermal
                     ? (_nodal_material ? &getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                        : &getMaterialProperty<Real>("PorousFlow_temperature_qp"))
                     : nullptr),
    _dtemperature_dvar(
        _thermal
            ? (_nodal_material
                   ? &getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
                   : &getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar"))
            : nullptr),

    _mineral_conc_old(_chemical ? (_nodal_material ? &getMaterialPropertyOld<std::vector<Real>>(
                                                         "PorousFlow_mineral_concentration_nodal")
                                                   : &getMaterialPropertyOld<std::vector<Real>>(
                                                         "PorousFlow_mineral_concentration_qp"))
                                : nullptr),
    _reaction_rate(_chemical ? (_nodal_material ? &getMaterialProperty<std::vector<Real>>(
                                                      "PorousFlow_mineral_reaction_rate_nodal")
                                                : &getMaterialProperty<std::vector<Real>>(
                                                      "PorousFlow_mineral_reaction_rate_qp"))
                             : nullptr),
    _dreaction_rate_dvar(_chemical ? (_nodal_material
                                          ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_mineral_reaction_rate_nodal_dvar")
                                          : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                "dPorousFlow_mineral_reaction_rate_qp_dvar"))
                                   : nullptr),
    _aq_ph(_dictator.aqueousPhaseNumber()),
    _saturation(_chemical
                    ? (_nodal_material
                           ? &getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                           : &getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp"))
                    : nullptr),
    _dsaturation_dvar(_chemical
                          ? (_nodal_material ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                   "dPorousFlow_saturation_nodal_dvar")
                                             : &getMaterialProperty<std::vector<std::vector<Real>>>(
                                                   "dPorousFlow_saturation_qp_dvar"))
                          : nullptr)
{
  if (_thermal && !isParamValid("thermal_expansion_coeff"))
    mooseError("PorousFlowPorosity: When thermal=true you must provide a thermal_expansion_coeff");
  if (_fluid && !isParamValid("solid_bulk"))
    mooseError("PorousFlowPorosity: When fluid=true you must provide a solid_bulk");
  if (_chemical && _num_c_ref != _dictator.numAqueousKinetic())
    mooseError("PorousFlowPorosity: When chemical=true you must provide the reference_chemistry "
               "values.  The Dictator proclaims there should be ",
               _dictator.numAqueousKinetic(),
               " of these");
  if (_chemical && _num_initial_c != _dictator.numAqueousKinetic())
    mooseError("PorousFlowPorosity: When chemical=true you must provide the "
               "initial_mineral_concentrations.  "
               "The Dictator proclaims there should be ",
               _dictator.numAqueousKinetic(),
               " of these");
  if (_chemical && _c_weights.size() != _dictator.numAqueousKinetic())
    mooseError(
        "PorousFlowPorosity: When chemical=true you must provde the correct number of "
        "chemical_weights (which the Dictator knows is ",
        _dictator.numAqueousKinetic(),
        ") or do not provide any chemical_weights and use the default value of 1 for each mineral");

  for (unsigned i = 0; i < _num_c_ref; ++i)
  {
    _c_reference[i] = (_nodal_material ? &coupledDofValues("reference_chemistry", i)
                                       : &coupledValue("reference_chemistry", i));
    _initial_c[i] = (_nodal_material ? &coupledDofValues("initial_mineral_concentrations", i)
                                     : &coupledValue("initial_mineral_concentrations", i));
  }
}

Real
PorousFlowPorosity::atNegInfinityQp() const
{
  /*
   *
   * Note the use of the OLD value of porosity here.
   * This strategy, which breaks the cyclic dependency between porosity
   * and mineral concentration, is used in
   * Kernel: PorousFlowPreDis
   * Material: PorousFlowPorosity
   * Material: PorousFlowAqueousPreDisChemistry
   * Material: PorousFlowAqueousPreDisMineral
   *
   */
  Real result = _biot;
  if (_chemical)
  {
    if (_t_step == 0 && !_app.isRestarting())
      for (unsigned i = 0; i < _num_c_ref; ++i)
        result -= _c_weights[i] * (*_initial_c[i])[_qp];
    else
      for (unsigned i = 0; i < _num_c_ref; ++i)
        result -= _c_weights[i] * ((*_mineral_conc_old)[_qp][i] + _dt * (*_porosity_old)[_qp] *
                                                                      (*_saturation)[_qp][_aq_ph] *
                                                                      (*_reaction_rate)[_qp][i]);
  }
  return result;
}

Real
PorousFlowPorosity::datNegInfinityQp(unsigned pvar) const
{
  Real result = 0.0;
  if (_chemical && (_t_step >= 1 || _app.isRestarting()))
    for (unsigned i = 0; i < _num_c_ref; ++i)
      result -= _c_weights[i] * _dt * (*_porosity_old)[_qp] *
                ((*_saturation)[_qp][_aq_ph] * (*_dreaction_rate_dvar)[_qp][i][pvar] +
                 (*_dsaturation_dvar)[_qp][_aq_ph][pvar] * (*_reaction_rate)[_qp][i]);
  return result;
}

Real
PorousFlowPorosity::atZeroQp() const
{
  // note the [0] below: _phi0 is a constant monomial and we use [0] regardless of _nodal_material
  Real result = _phi0[0];
  if (_chemical)
  {
    if (_t_step == 0 && !_app.isRestarting())
      for (unsigned i = 0; i < _num_c_ref; ++i)
        result -= _c_weights[i] * ((*_initial_c[i])[_qp] - (*_c_reference[i])[_qp]);
    else
      for (unsigned i = 0; i < _num_c_ref; ++i)
        result -= _c_weights[i] * ((*_mineral_conc_old)[_qp][i] +
                                   _dt * (*_porosity_old)[_qp] * (*_saturation)[_qp][_aq_ph] *
                                       (*_reaction_rate)[_qp][i] -
                                   (*_c_reference[i])[_qp]);
  }
  return result;
}

Real
PorousFlowPorosity::datZeroQp(unsigned pvar) const
{
  Real result = 0.0;
  if (_chemical && (_t_step >= 1 || _app.isRestarting()))
    for (unsigned i = 0; i < _num_c_ref; ++i)
      result -= _c_weights[i] * _dt * (*_porosity_old)[_qp] *
                ((*_saturation)[_qp][_aq_ph] * (*_dreaction_rate_dvar)[_qp][i][pvar] +
                 (*_dsaturation_dvar)[_qp][_aq_ph][pvar] * (*_reaction_rate)[_qp][i]);
  return result;
}

Real
PorousFlowPorosity::decayQp() const
{
  Real result = 0.0;

  if (_thermal)
    result += _exp_coeff * ((*_temperature)[_qp] - _t_reference[_qp]);

  if (_fluid)
    result += _coeff * ((*_pf)[_qp] - _p_reference[_qp]);

  if (_mechanical)
  {
    // Note that in the following _strain[_qp] is evaluated at q quadpoint
    // So _porosity_nodal[_qp], which should be the nodal value of porosity
    // actually uses the strain at a quadpoint.  This
    // is OK for LINEAR elements, as strain is constant over the element anyway.
    const unsigned qp_to_use =
        (_nodal_material && (_bnd || _strain_at_nearest_qp) ? nearestQP(_qp) : _qp);
    result += -(*_vol_strain_qp)[qp_to_use];
  }

  return result;
}

Real
PorousFlowPorosity::ddecayQp_dvar(unsigned pvar) const
{
  Real result = 0.0;

  if (_thermal)
    result += _exp_coeff * (*_dtemperature_dvar)[_qp][pvar];

  if (_fluid)
    result += _coeff * (*_dpf_dvar)[_qp][pvar];

  return result;
}

RealGradient
PorousFlowPorosity::ddecayQp_dgradvar(unsigned pvar) const
{
  RealGradient result(0.0, 0.0, 0.0);
  if (_mechanical)
  {
    const unsigned qp_to_use =
        (_nodal_material && (_bnd || _strain_at_nearest_qp) ? nearestQP(_qp) : _qp);
    result += -(*_dvol_strain_qp_dvar)[qp_to_use][pvar];
  }
  return result;
}
