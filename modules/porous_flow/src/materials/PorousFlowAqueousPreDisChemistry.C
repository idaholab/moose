//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowAqueousPreDisChemistry.h"

registerMooseObject("PorousFlowApp", PorousFlowAqueousPreDisChemistry);

InputParameters
PorousFlowAqueousPreDisChemistry::validParams()
{
  InputParameters params = PorousFlowMaterialVectorBase::validParams();
  params.addRequiredCoupledVar(
      "primary_concentrations",
      "List of MOOSE Variables that represent the concentrations of the primary species");
  params.addRequiredParam<unsigned>("num_reactions",
                                    "Number of equations in the system of chemical reactions");
  params.addParam<bool>("equilibrium_constants_as_log10",
                        false,
                        "If true, the equilibrium constants are written in their log10 form, eg, "
                        "-2.  If false, the equilibrium constants are written in absolute terms, "
                        "eg, 0.01");
  params.addRequiredCoupledVar("equilibrium_constants",
                               "Equilibrium constant for each equation (dimensionless).  If these "
                               "are temperature dependent AuxVariables, the Jacobian will not be "
                               "exact");
  params.addRequiredParam<std::vector<Real>>(
      "primary_activity_coefficients",
      "Activity coefficients for the primary species (dimensionless) (one for each)");
  params.addRequiredParam<std::vector<Real>>(
      "reactions",
      "A matrix defining the aqueous reactions.  The matrix is entered as a long vector: the first "
      "row is "
      "entered first, followed by the second row, etc.  There should be num_reactions rows.  All "
      "primary species should appear only on the LHS of each reaction (and there should be just "
      "one secondary species on the RHS, by definition) so they may have negative coefficients.  "
      "Each row should have number of primary_concentrations entries, which are the stoichiometric "
      "coefficients.  The first coefficient must always correspond to the first primary species, "
      "etc");
  params.addRequiredParam<std::vector<Real>>("specific_reactive_surface_area",
                                             "Specific reactive surface area in m^2/(L solution).");
  params.addRequiredParam<std::vector<Real>>(
      "kinetic_rate_constant",
      "Kinetic rate constant in mol/(m^2 s), at the reference temperature (one for each reaction)");
  params.addRequiredParam<std::vector<Real>>("molar_volume",
                                             "Volume occupied by one mole of the secondary species "
                                             "(L(solution)/mol) (one for each reaction)");
  params.addRequiredParam<std::vector<Real>>("activation_energy",
                                             "Activation energy, J/mol (one for each reaction)");
  params.addParam<Real>("gas_constant", 8.31434, "Gas constant, in J/(mol K)");
  params.addParam<Real>("reference_temperature", 298.15, "Reference temperature, K");
  params.addParam<std::vector<Real>>("theta_exponent",
                                     "Theta exponent.  Defaults to 1.  (one for each reaction)");
  params.addParam<std::vector<Real>>("eta_exponent",
                                     "Eta exponent.  Defaults to 1.  (one for each reaction)");
  params.addPrivateParam<std::string>("pf_material_type", "chemistry");
  params.addClassDescription("This Material forms a std::vector of mineralisation reaction rates "
                             "(L(precipitate)/L(solution)/s) appropriate to the aqueous "
                             "precipitation-dissolution system provided.  Note: the "
                             "PorousFlowTemperature must be measured in Kelvin.");
  return params;
}

PorousFlowAqueousPreDisChemistry::PorousFlowAqueousPreDisChemistry(
    const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _porosity_old(_nodal_material ? getMaterialPropertyOld<Real>("PorousFlow_porosity_nodal")
                                  : getMaterialPropertyOld<Real>("PorousFlow_porosity_qp")),
    _aq_ph(_dictator.aqueousPhaseNumber()),
    _saturation(_nodal_material
                    ? getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_nodal")
                    : getMaterialProperty<std::vector<Real>>("PorousFlow_saturation_qp")),

    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                 : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material
            ? getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
            : getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),

    _num_primary(coupledComponents("primary_concentrations")),
    _num_reactions(getParam<unsigned>("num_reactions")),
    _equilibrium_constants_as_log10(getParam<bool>("equilibrium_constants_as_log10")),
    _num_equilibrium_constants(coupledComponents("equilibrium_constants")),
    _equilibrium_constants(_num_equilibrium_constants),
    _primary_activity_coefficients(getParam<std::vector<Real>>("primary_activity_coefficients")),
    _reactions(getParam<std::vector<Real>>("reactions")),

    _sec_conc_old(
        _nodal_material
            ? getMaterialPropertyOld<std::vector<Real>>("PorousFlow_mineral_concentration_nodal")
            : getMaterialPropertyOld<std::vector<Real>>("PorousFlow_mineral_concentration_qp")),

    _mineral_sat(_num_reactions),
    _bounded_rate(_num_reactions),
    _reaction_rate(
        _nodal_material
            ? declareProperty<std::vector<Real>>("PorousFlow_mineral_reaction_rate_nodal")
            : declareProperty<std::vector<Real>>("PorousFlow_mineral_reaction_rate_qp")),
    _dreaction_rate_dvar(_nodal_material ? declareProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_mineral_reaction_rate_nodal_dvar")
                                         : declareProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_mineral_reaction_rate_qp_dvar")),

    _r_area(getParam<std::vector<Real>>("specific_reactive_surface_area")),
    _molar_volume(getParam<std::vector<Real>>("molar_volume")),
    _ref_kconst(getParam<std::vector<Real>>("kinetic_rate_constant")),
    _e_act(getParam<std::vector<Real>>("activation_energy")),
    _gas_const(getParam<Real>("gas_constant")),
    _one_over_ref_temp(1.0 / getParam<Real>("reference_temperature")),
    _theta_exponent(isParamValid("theta_exponent") ? getParam<std::vector<Real>>("theta_exponent")
                                                   : std::vector<Real>(_num_reactions, 1.0)),
    _eta_exponent(isParamValid("eta_exponent") ? getParam<std::vector<Real>>("eta_exponent")
                                               : std::vector<Real>(_num_reactions, 1.0))
{
  if (_dictator.numPhases() < 1)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of fluid phases must not be zero");

  if (_num_primary != _num_components - 1)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of mass_fraction_vars is ",
               _num_components,
               " which must be one greater than the number of primary concentrations (which is ",
               _num_primary,
               ")");

  // correct number of equilibrium constants
  if (_num_equilibrium_constants != _num_reactions)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of equilibrium constants is ",
               _num_equilibrium_constants,
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  // correct number of activity coefficients
  if (_primary_activity_coefficients.size() != _num_primary)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of primary activity "
               "coefficients is ",
               _primary_activity_coefficients.size(),
               " which must be equal to the number of primary species (",
               _num_primary,
               ")");

  // correct number of stoichiometry coefficients
  if (_reactions.size() != _num_reactions * _num_primary)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of stoichiometric "
               "coefficients specified in 'reactions' (",
               _reactions.size(),
               ") must be equal to the number of reactions (",
               _num_reactions,
               ") multiplied by the number of primary species (",
               _num_primary,
               ")");

  if (_r_area.size() != _num_reactions)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of specific reactive "
               "surface areas provided is ",
               _r_area.size(),
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  if (_ref_kconst.size() != _num_reactions)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of kinetic rate constants is ",
               _ref_kconst.size(),
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  if (_e_act.size() != _num_reactions)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of activation energies is ",
               _e_act.size(),
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  if (_molar_volume.size() != _num_reactions)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of molar volumes is ",
               _molar_volume.size(),
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  if (_theta_exponent.size() != _num_reactions)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of theta exponents is ",
               _theta_exponent.size(),
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  if (_eta_exponent.size() != _num_reactions)
    mooseError("PorousFlowAqueousPreDisChemistry: The number of eta exponents is ",
               _eta_exponent.size(),
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  if (_num_reactions != _dictator.numAqueousKinetic())
    mooseError("PorousFlowAqueousPreDisChemistry: You have specified the number of "
               "reactions to be ",
               _num_reactions,
               " but the Dictator knows that the number of aqueous kinetic "
               "(precipitation-dissolution) reactions is ",
               _dictator.numAqueousKinetic());

  _primary_var_num.resize(_num_primary);
  _primary.resize(_num_primary);
  for (unsigned i = 0; i < _num_primary; ++i)
  {
    _primary_var_num[i] = coupled("primary_concentrations", i);
    _primary[i] = (_nodal_material ? &coupledDofValues("primary_concentrations", i)
                                   : &coupledValue("primary_concentrations", i));
  }

  for (unsigned i = 0; i < _num_equilibrium_constants; ++i)
  {
    // If equilibrium_constants are elemental AuxVariables (or constants), we want to use
    // coupledGenericValue() rather than coupledGenericDofValue()
    const bool is_nodal =
        isCoupled("equilibrium_constants") ? getVar("equilibrium_constants", i)->isNodal() : false;

    _equilibrium_constants[i] =
        (_nodal_material && is_nodal ? &coupledDofValues("equilibrium_constants", i)
                                     : &coupledValue("equilibrium_constants", i));
  }
}

void
PorousFlowAqueousPreDisChemistry::initQpStatefulProperties()
{
  _reaction_rate[_qp].resize(_num_reactions);
  _dreaction_rate_dvar[_qp].resize(_num_reactions);
  for (unsigned r = 0; r < _num_reactions; ++r)
    _dreaction_rate_dvar[_qp][r].assign(_num_var, 0.0);
}

void
PorousFlowAqueousPreDisChemistry::computeQpProperties()
{
  _reaction_rate[_qp].resize(_num_reactions);
  _dreaction_rate_dvar[_qp].resize(_num_reactions);
  for (unsigned r = 0; r < _num_reactions; ++r)
    _dreaction_rate_dvar[_qp][r].assign(_num_var, 0.0);

  // Compute the reaction rates
  computeQpReactionRates();

  // Compute the derivatives of the reaction rates
  std::vector<std::vector<Real>> drr(_num_reactions);
  std::vector<Real> drr_dT(_num_reactions);
  for (unsigned r = 0; r < _num_reactions; ++r)
  {
    dQpReactionRate_dprimary(r, drr[r]);
    drr_dT[r] = dQpReactionRate_dT(r);
  }

  // compute _dreaction_rate_dvar[_qp]
  for (unsigned wrt = 0; wrt < _num_primary; ++wrt)
  {
    // derivative with respect to the "wrt"^th primary species concentration
    if (!_dictator.isPorousFlowVariable(_primary_var_num[wrt]))
      continue;
    const unsigned pf_wrt = _dictator.porousFlowVariableNum(_primary_var_num[wrt]);

    // run through the reactions, using drr in the appropriate places
    for (unsigned r = 0; r < _num_reactions; ++r)
      _dreaction_rate_dvar[_qp][r][pf_wrt] = drr[r][wrt];
  }

  // use the derivative wrt temperature
  for (unsigned r = 0; r < _num_reactions; ++r)
    for (unsigned v = 0; v < _num_var; ++v)
      _dreaction_rate_dvar[_qp][r][v] += drr_dT[r] * _dtemperature_dvar[_qp][v];
}

Real
PorousFlowAqueousPreDisChemistry::stoichiometry(unsigned reaction_num, unsigned primary_num) const
{
  const unsigned index = reaction_num * _num_primary + primary_num;
  return _reactions[index];
}

void
PorousFlowAqueousPreDisChemistry::findZeroConcentration(unsigned & zero_conc_index,
                                                        unsigned & zero_count) const
{
  zero_count = 0;
  for (unsigned i = 0; i < _num_primary; ++i)
  {
    if (_primary_activity_coefficients[i] * (*_primary[i])[_qp] <= 0.0)
    {
      zero_count += 1;
      zero_conc_index = i;
      if (zero_count > 1)
        return;
    }
  }
  return;
}

void
PorousFlowAqueousPreDisChemistry::computeQpReactionRates()
{
  for (unsigned r = 0; r < _num_reactions; ++r)
  {
    _mineral_sat[r] =
        (_equilibrium_constants_as_log10 ? std::pow(10.0, -(*_equilibrium_constants[r])[_qp])
                                         : 1.0 / (*_equilibrium_constants[r])[_qp]);
    for (unsigned j = 0; j < _num_primary; ++j)
    {
      const Real gamp = _primary_activity_coefficients[j] * (*_primary[j])[_qp];
      if (gamp <= 0.0)
      {
        if (stoichiometry(r, j) < 0.0)
          _mineral_sat[r] = std::numeric_limits<Real>::max();
        else if (stoichiometry(r, j) == 0.0)
          _mineral_sat[r] *= 1.0;
        else
        {
          _mineral_sat[r] = 0.0;
          break;
        }
      }
      else
        _mineral_sat[r] *= std::pow(gamp, stoichiometry(r, j));
    }
    const Real fac = 1.0 - std::pow(_mineral_sat[r], _theta_exponent[r]);
    // if fac > 0 then dissolution occurs; if fac < 0 then precipitation occurs.
    const Real sgn = (fac < 0 ? -1.0 : 1.0);
    const Real unbounded_rr = -sgn * rateConstantQp(r) * _r_area[r] * _molar_volume[r] *
                              std::pow(std::abs(fac), _eta_exponent[r]);

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

    // bound the reaction so _sec_conc lies between zero and unity
    const Real por_times_rr_dt = _porosity_old[_qp] * _saturation[_qp][_aq_ph] * unbounded_rr * _dt;
    if (_sec_conc_old[_qp][r] + por_times_rr_dt > 1.0)
    {
      _bounded_rate[r] = true;
      _reaction_rate[_qp][r] =
          (1.0 - _sec_conc_old[_qp][r]) / _porosity_old[_qp] / _saturation[_qp][_aq_ph] / _dt;
    }
    else if (_sec_conc_old[_qp][r] + por_times_rr_dt < 0.0)
    {
      _bounded_rate[r] = true;
      _reaction_rate[_qp][r] =
          -_sec_conc_old[_qp][r] / _porosity_old[_qp] / _saturation[_qp][_aq_ph] / _dt;
    }
    else
    {
      _bounded_rate[r] = false;
      _reaction_rate[_qp][r] = unbounded_rr;
    }
  }
}

void
PorousFlowAqueousPreDisChemistry::dQpReactionRate_dprimary(unsigned reaction_num,
                                                           std::vector<Real> & drr) const
{
  drr.assign(_num_primary, 0.0);

  // handle corner case
  if (_bounded_rate[reaction_num])
    return;

  /*
   * Form the derivative of _mineral_sat, and store it in drr for now.
   * The derivatives are straightforward if all primary > 0.
   *
   * If more than one primary = 0 then I set the derivatives to zero, even though it could be
   * argued that with certain stoichiometric coefficients you might have derivative = 0/0 and it
   * might be appropriate to set this to a non-zero finite value.
   *
   * If exactly one primary = 0 and its stoichiometry = 1 then the derivative wrt this one is
   * nonzero.
   * If exactly one primary = 0 and its stoichiometry > 1 then all derivatives are zero.
   * If exactly one primary = 0 and its stoichiometry < 1 then the derivative wrt this one is
   * infinity
   */

  unsigned zero_count = 0;
  unsigned zero_conc_index = 0;
  findZeroConcentration(zero_conc_index, zero_count);
  if (zero_count == 0)
  {
    for (unsigned i = 0; i < _num_primary; ++i)
      drr[i] = stoichiometry(reaction_num, i) * _mineral_sat[reaction_num] /
               std::max((*_primary[i])[_qp], 0.0);
  }
  else
  {
    if (_theta_exponent[reaction_num] < 1.0)
      // dfac = infinity (see below) so the derivative may be inf, inf * 0, or inf/inf.  I simply
      // return with drr = 0
      return;

    // count the number of primary <= 0, and record the one that's zero
    if (zero_count == 1 && stoichiometry(reaction_num, zero_conc_index) == 1.0)
    {
      Real conc_without_zero = (_equilibrium_constants_as_log10
                                    ? std::pow(10.0, -(*_equilibrium_constants[reaction_num])[_qp])
                                    : 1.0 / (*_equilibrium_constants[reaction_num])[_qp]);
      for (unsigned i = 0; i < _num_primary; ++i)
      {
        if (i == zero_conc_index)
          conc_without_zero *= _primary_activity_coefficients[i];
        else
          conc_without_zero *=
              std::pow(_primary_activity_coefficients[i] * std::max((*_primary[i])[_qp], 0.0),
                       stoichiometry(reaction_num, i));
      }
      drr[zero_conc_index] = conc_without_zero;
    }
    else if (zero_count == 0 and stoichiometry(reaction_num, zero_conc_index) < 1.0)
      drr[zero_conc_index] = std::numeric_limits<Real>::max();
    else
      // all other cases have drr = 0, so return without performing any other calculations
      return;
  }

  // At the moment _drr = d(mineral_sat)/d(primary)
  // Multiply by appropriate term to give _drr = d(reaction_rate)/d(primary)
  const Real fac = 1.0 - std::pow(_mineral_sat[reaction_num], _theta_exponent[reaction_num]);
  const Real dfac = -_theta_exponent[reaction_num] *
                    std::pow(_mineral_sat[reaction_num], _theta_exponent[reaction_num] - 1.0);
  const Real multiplier = -rateConstantQp(reaction_num) * _r_area[reaction_num] *
                          _molar_volume[reaction_num] *
                          std::pow(std::abs(fac), _eta_exponent[reaction_num] - 1.0) *
                          _eta_exponent[reaction_num] * dfac;
  for (unsigned i = 0; i < _num_primary; ++i)
    drr[i] *= multiplier;
}

Real
PorousFlowAqueousPreDisChemistry::rateConstantQp(unsigned reaction_num) const
{
  return _ref_kconst[reaction_num] * std::exp(_e_act[reaction_num] / _gas_const *
                                              (_one_over_ref_temp - 1.0 / _temperature[_qp]));
}

Real
PorousFlowAqueousPreDisChemistry::dQpReactionRate_dT(unsigned reaction_num) const
{
  // handle corner case
  if (_bounded_rate[reaction_num])
    return 0.0;

  const Real drate_const = rateConstantQp(reaction_num) * _e_act[reaction_num] / _gas_const *
                           std::pow(_temperature[_qp], -2.0);
  const Real fac = 1.0 - std::pow(_mineral_sat[reaction_num], _theta_exponent[reaction_num]);
  const Real sgn = (fac < 0 ? -1.0 : 1.0);
  const Real dkinetic_rate = -sgn * drate_const * _r_area[reaction_num] *
                             _molar_volume[reaction_num] *
                             std::pow(std::abs(fac), _eta_exponent[reaction_num]);

  return dkinetic_rate;
}
