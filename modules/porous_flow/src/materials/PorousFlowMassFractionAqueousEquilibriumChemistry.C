//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowMassFractionAqueousEquilibriumChemistry.h"

template <>
InputParameters
validParams<PorousFlowMassFractionAqueousEquilibriumChemistry>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addRequiredCoupledVar("primary_concentrations",
                               "List of MOOSE Variables that represent the concentrations "
                               "(m^{3}(chemical)/m^{3}(solution)) of the primary species");
  params.addRequiredParam<unsigned>("num_reactions",
                                    "Number of equations in the system of chemical reactions");
  params.addRequiredParam<std::vector<Real>>("equilibrium_constants",
                                             "Equilibrium constant for each equation "
                                             "(dimensionless) (written in absolute terms, not as a "
                                             "log10)");
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
  params.addRequiredParam<std::vector<Real>>(
      "secondary_activity_coefficients",
      "Activity coefficients for the secondary species (dimensionless) (one for each reaction)");
  params.addClassDescription(
      "This Material forms a std::vector<std::vector ...> of mass-fractions "
      "(total concentrations of primary species (m^{3}(primary species)/m^{3}(solution)) and since "
      "this is for an aqueous system only, mass-fraction equals volume-fraction) corresponding to "
      "an "
      "aqueous equilibrium chemistry system.  The first mass fraction is the "
      "concentration of the first primary species, etc, and the last mass "
      "fraction is the concentration of H2O.");
  return params;
}

PorousFlowMassFractionAqueousEquilibriumChemistry::
    PorousFlowMassFractionAqueousEquilibriumChemistry(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),
    _mass_frac(_nodal_material
                   ? declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")
                   : declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _grad_mass_frac(_nodal_material ? nullptr
                                    : &declareProperty<std::vector<std::vector<RealGradient>>>(
                                          "PorousFlow_grad_mass_frac_qp")),
    _dmass_frac_dvar(_nodal_material ? declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_nodal_dvar")
                                     : declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_qp_dvar")),
    _sec_conc(_nodal_material
                  ? declareProperty<std::vector<Real>>("PorousFlow_secondary_concentration_nodal")
                  : declareProperty<std::vector<Real>>("PorousFlow_secondary_concentration_qp")),
    _dsec_conc_dvar(_nodal_material ? declareProperty<std::vector<std::vector<Real>>>(
                                          "dPorousFlow_secondary_concentration_nodal_dvar")
                                    : declareProperty<std::vector<std::vector<Real>>>(
                                          "dPorousFlow_secondary_concentration_qp_dvar")),

    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                 : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material
            ? getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
            : getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),

    _num_primary(coupledComponents("primary_concentrations")),
    _num_reactions(getParam<unsigned>("num_reactions")),
    _equilibrium_constants(getParam<std::vector<Real>>("equilibrium_constants")),
    _primary_activity_coefficients(getParam<std::vector<Real>>("primary_activity_coefficients")),
    _reactions(getParam<std::vector<Real>>("reactions")),
    _secondary_activity_coefficients(getParam<std::vector<Real>>("secondary_activity_coefficients"))
{
  // only multi-component, single-phase PorousFlow
  if (_num_phases != 1)
    mooseError("PorousFlowMassFractionAqueousEquilibriumChemistry is designed for "
               "single-phase simulations only");
  if (_num_primary != _num_components - 1)
    mooseError("PorousFlowMassFractionAqueousEquilibriumChemistry: The number of "
               "mass_fraction_vars is ",
               _num_components,
               " which must be one greater than the number of primary concentrations (which is ",
               _num_primary,
               ")");

  // correct number of equilibrium constants
  if (_equilibrium_constants.size() != _num_reactions)
    mooseError("PorousFlowMassFractionAqueousEquilibriumChemistry: The number of "
               "equilibrium constants is ",
               _equilibrium_constants.size(),
               " which must be equal to the number of reactions (",
               _num_reactions,
               ")");

  // correct number of activity coefficients
  if (_primary_activity_coefficients.size() != _num_primary)
    mooseError("PorousFlowMassFractionAqueousEquilibriumChemistry: The number of primary activity "
               "coefficients is ",
               _primary_activity_coefficients.size(),
               " which must be equal to the number of primary species (",
               _num_primary,
               ")");

  // correct number of stoichiometry coefficients
  if (_reactions.size() != _num_reactions * _num_primary)
    mooseError("PorousFlowMassFractionAqueousEquilibriumChemistry: The number of stoichiometric "
               "coefficients specified in 'reactions' (",
               _reactions.size(),
               ") must be equal to the number of reactions (",
               _num_reactions,
               ") multiplied by the number of primary species (",
               _num_primary,
               ")");

  // correct number of secondary activity coefficients
  if (_secondary_activity_coefficients.size() != _num_reactions)
    mooseError(
        "PorousFlowMassFractionAqueousEquilibriumChemistry: The number of secondary activity "
        "coefficients is ",
        _secondary_activity_coefficients.size(),
        " which must be equal to the number of secondary species (",
        _num_reactions,
        ")");

  // correct number of reactions
  if (_num_reactions != _dictator.numAqueousEquilibrium())
    mooseError("PorousFlowMassFractionAqueousEquilibriumChemistry: You have specified the number "
               "of reactions to be ",
               _num_reactions,
               " but the Dictator knows that the number of aqueous equilibrium reactions is ",
               _dictator.numAqueousEquilibrium());

  _primary_var_num.resize(_num_primary);
  _primary.resize(_num_primary);
  _grad_primary.resize(_num_primary);
  for (unsigned i = 0; i < _num_primary; ++i)
  {
    _primary_var_num[i] = coupled("primary_concentrations", i);
    _primary[i] = (_nodal_material ? &coupledNodalValue("primary_concentrations", i)
                                   : &coupledValue("primary_concentrations", i));
    _grad_primary[i] = &coupledGradient("primary_concentrations", i);
  }
}

void
PorousFlowMassFractionAqueousEquilibriumChemistry::initQpStatefulProperties()
{
  computeQpProperties();
}

void
PorousFlowMassFractionAqueousEquilibriumChemistry::computeQpProperties()
{
  const unsigned ph = 0; // only single-phase is modelled by this Material

  // size all properties correctly
  _mass_frac[_qp].resize(_num_phases);
  _dmass_frac_dvar[_qp].resize(_num_phases);
  if (!_nodal_material)
    (*_grad_mass_frac)[_qp].resize(_num_phases);
  _mass_frac[_qp][ph].resize(_num_components);
  _dmass_frac_dvar[_qp][ph].resize(_num_components);
  for (unsigned int comp = 0; comp < _num_components; ++comp)
    _dmass_frac_dvar[_qp][ph][comp].assign(_num_var, 0.0);
  if (!_nodal_material)
    (*_grad_mass_frac)[_qp][ph].assign(_num_components, 0.0);
  _sec_conc[_qp].resize(_num_reactions);
  _dsec_conc_dvar[_qp].resize(_num_reactions);
  for (unsigned r = 0; r < _num_reactions; ++r)
    _dsec_conc_dvar[_qp][r].assign(_num_var, 0.0);

  // Compute the secondary concentrations
  if (_t_step == 0 && !_app.isRestarting())
    initQpSecondaryConcentrations();
  else
    computeQpSecondaryConcentrations();

  // compute _mass_frac[_qp][ph]
  _mass_frac[_qp][ph][_num_components - 1] = 1.0; // the final component is H20
  for (unsigned i = 0; i < _num_primary; ++i)
  {
    _mass_frac[_qp][ph][i] = (*_primary[i])[_qp];
    for (unsigned r = 0; r < _num_reactions; ++r)
      _mass_frac[_qp][ph][i] += stoichiometry(r, i) * _sec_conc[_qp][r];

    // remove mass-fraction from the H20 component
    _mass_frac[_qp][ph][_num_components - 1] -= _mass_frac[_qp][ph][i];
  }

  // Compute the derivatives of the secondary concentrations
  std::vector<std::vector<Real>> dsec(_num_reactions);
  std::vector<Real> dsec_dT(_num_reactions);
  for (unsigned r = 0; r < _num_reactions; ++r)
  {
    dQpSecondaryConcentration_dprimary(r, dsec[r]);
    dsec_dT[r] = dQpSecondaryConcentration_dT(r);
  }

  // Compute the derivatives of the mass_frac wrt the primary concentrations
  // This is used in _dmass_frac_dvar as well as _grad_mass_frac
  std::vector<std::vector<Real>> dmf(_num_components);
  for (unsigned i = 0; i < _num_components; ++i)
    dmf[i].assign(_num_primary, 0.0);
  for (unsigned wrt = 0; wrt < _num_primary; ++wrt)
  {
    // run through the mass fractions (except the last one) adding to their derivatives
    // The special case is:
    dmf[wrt][wrt] = 1.0;
    // The secondary-species contributions are:
    for (unsigned i = 0; i < _num_primary; ++i)
      for (unsigned r = 0; r < _num_reactions; ++r)
        dmf[i][wrt] += stoichiometry(r, i) * dsec[r][wrt];

    // compute dmf[_num_components - 1]
    for (unsigned i = 0; i < _num_primary; ++i)
      dmf[_num_components - 1][wrt] -= dmf[i][wrt];
  }

  // Compute the derivatives of the mass_frac wrt the temperature
  // This is used in _dmass_frac_dvar
  std::vector<Real> dmf_dT(_num_components, 0.0);
  for (unsigned i = 0; i < _num_components - 1; ++i)
  {
    for (unsigned r = 0; r < _num_reactions; ++r)
      dmf_dT[i] += stoichiometry(r, i) * dsec_dT[r];
    dmf_dT[_num_components - 1] -= dmf_dT[i];
  }

  // compute _dmass_frac_dvar[_qp][ph] and _dsec_conc_dvar[_qp]
  for (unsigned wrt = 0; wrt < _num_primary; ++wrt)
  {
    // derivative with respect to the "wrt"^th primary species concentration
    if (!_dictator.isPorousFlowVariable(_primary_var_num[wrt]))
      continue;
    const unsigned pf_wrt = _dictator.porousFlowVariableNum(_primary_var_num[wrt]);

    // run through the mass fractions, building the derivative using dmf
    for (unsigned i = 0; i < _num_components; ++i)
      _dmass_frac_dvar[_qp][ph][i][pf_wrt] = dmf[i][wrt];

    // run through the secondary concentrations, using dsec in the appropriate places
    for (unsigned r = 0; r < _num_reactions; ++r)
      _dsec_conc_dvar[_qp][r][pf_wrt] = dsec[r][wrt];
  }

  // use the derivative wrt temperature
  for (unsigned i = 0; i < _num_components; ++i)
    for (unsigned v = 0; v < _num_var; ++v)
      _dmass_frac_dvar[_qp][ph][i][v] += dmf_dT[i] * _dtemperature_dvar[_qp][v];
  for (unsigned r = 0; r < _num_reactions; ++r)
    for (unsigned v = 0; v < _num_var; ++v)
      _dsec_conc_dvar[_qp][r][v] += dsec_dT[r] * _dtemperature_dvar[_qp][v];

  // compute the gradient, if needed
  // NOTE: The derivative d(grad_mass_frac)/d(var) != d(mass_frac)/d(var) * grad_phi
  //       because mass fraction is a nonlinear function of the primary variables
  //       This means that the Jacobian in PorousFlowDispersiveFlux will be wrong
  if (!_nodal_material)
  {
    for (unsigned comp = 0; comp < _num_components - 1; ++comp)
    {
      for (unsigned wrt = 0; wrt < _num_primary; ++wrt)
        (*_grad_mass_frac)[_qp][ph][comp] += dmf[comp][wrt] * (*_grad_primary[wrt])[_qp];
      (*_grad_mass_frac)[_qp][ph][_num_components - 1] -= (*_grad_mass_frac)[_qp][ph][comp];
    }
  }
}

Real
PorousFlowMassFractionAqueousEquilibriumChemistry::stoichiometry(unsigned reaction_num,
                                                                 unsigned primary_num) const
{
  const unsigned index = reaction_num * _num_primary + primary_num;
  return _reactions[index];
}

void
PorousFlowMassFractionAqueousEquilibriumChemistry::findZeroConcentration(
    unsigned & zero_conc_index, unsigned & zero_count) const
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
PorousFlowMassFractionAqueousEquilibriumChemistry::initQpSecondaryConcentrations()
{
  _sec_conc[_qp].assign(_num_reactions, 0.0);
}

void
PorousFlowMassFractionAqueousEquilibriumChemistry::computeQpSecondaryConcentrations()
{
  for (unsigned r = 0; r < _num_reactions; ++r)
  {
    _sec_conc[_qp][r] = 1.0;
    for (unsigned i = 0; i < _num_primary; ++i)
    {
      const Real gamp = _primary_activity_coefficients[i] * (*_primary[i])[_qp];
      if (gamp <= 0.0)
      {
        if (stoichiometry(r, i) < 0.0)
          _sec_conc[_qp][r] = std::numeric_limits<Real>::max();
        else if (stoichiometry(r, i) == 0.0)
          _sec_conc[_qp][r] *= 1.0;
        else
        {
          _sec_conc[_qp][r] = 0.0;
          break;
        }
      }
      else
        _sec_conc[_qp][r] *= std::pow(gamp, stoichiometry(r, i));
    }
    _sec_conc[_qp][r] *= _equilibrium_constants[r] / _secondary_activity_coefficients[r];
  }
}

void
PorousFlowMassFractionAqueousEquilibriumChemistry::dQpSecondaryConcentration_dprimary(
    unsigned reaction_num, std::vector<Real> & dsc) const
{
  dsc.assign(_num_primary, 0.0);

  /*
   * the derivatives are straightforward if all primary > 0.
   *
   * If more than one primary = 0 then I set the derivatives to zero, even though it could be argued
   * that with certain stoichiometric coefficients you might have derivative = 0/0 and it might be
   * appropriate to set this to a non-zero finite value.
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
      dsc[i] = stoichiometry(reaction_num, i) * _sec_conc[_qp][reaction_num] / (*_primary[i])[_qp];
  }
  else
  {
    // count the number of primary <= 0, and record the one that's zero
    if (zero_count == 1 and stoichiometry(reaction_num, zero_conc_index) == 1.0)
    {
      Real conc_without_zero = 1.0;
      for (unsigned i = 0; i < _num_primary; ++i)
      {
        if (i == zero_conc_index)
          conc_without_zero *= _primary_activity_coefficients[i];
        else
          conc_without_zero *= std::pow(_primary_activity_coefficients[i] * (*_primary[i])[_qp],
                                        stoichiometry(reaction_num, i));
      }
      conc_without_zero *=
          _equilibrium_constants[reaction_num] / _secondary_activity_coefficients[reaction_num];
      dsc[zero_conc_index] = conc_without_zero;
    }
    else if (zero_count == 1 && stoichiometry(reaction_num, zero_conc_index) < 1.0)
      dsc[zero_conc_index] = std::numeric_limits<Real>::max();

    // all other cases have dsc = 0
  }
}

Real
PorousFlowMassFractionAqueousEquilibriumChemistry::dQpSecondaryConcentration_dT(
    unsigned /* reaction_num */) const
{
  return 0.0;
}
