//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EquilibriumReactionSolverOutput.h"
#include "GeochemistryConstants.h"
#include "GeochemistryFormattedOutput.h"
#include "GeochemistrySortedIndices.h"

registerMooseObject("GeochemistryApp", EquilibriumReactionSolverOutput);

InputParameters
EquilibriumReactionSolverOutput::validParams()
{
  InputParameters params = Output::validParams();
  params.addRequiredParam<UserObjectName>(
      "model_definition",
      "The name of the GeochemicalModelDefinition user object.  Only equilibrium reactions are "
      "solved by EquilibriumReactionSolverOutput, so the model_definition must not contain any "
      "kinetic "
      "species");
  params.addParam<std::vector<std::string>>(
      "swap_out_of_basis",
      "Species that should be removed from the model_definition's basis and be replaced with the "
      "swap_into_basis species.  There must be the same number of species in swap_out_of_basis and "
      "swap_into_basis.  If this list contains more than one species, the swapping is performed "
      "one-by-one, starting with the first pair (swap_out_of_basis[0] and swap_into_basis[0]), "
      "then the next pair, etc");
  params.addParam<std::vector<std::string>>(
      "swap_into_basis",
      "Species that should be removed from the model_definition's "
      "equilibrium species list and added to the basis");
  params.addParam<std::vector<std::string>>(
      "nernst_swap_out_of_basis",
      "Before outputting Nernst potentials (after solving the system) these species are swapped "
      "out of the basis.  Often this is identical to swap_into_basis, so that the Nernst "
      "potentials are defined in terms of the original model definition.  There must be the same "
      "number of species in nernst_swap_out_of_basis and nernst_swap_into_basis.  If this list "
      "contains more than one species, the swapping is performed one-by-one, starting with the "
      "first pair (nernst_swap_out_of_basis[0] and nernst_swap_into_basis[0]) then the next pair, "
      "etc");
  params.addParam<std::vector<std::string>>("nernst_swap_into_basis",
                                            "Before outputting Nernst potentials (after solving "
                                            "the system) these species are swapped into the basis");
  MultiMooseEnum constraint_meaning("moles_bulk_water kg_solvent_water moles_bulk_species "
                                    "free_molality free_moles_mineral_species fugacity activity");
  params.addRequiredParam<MultiMooseEnum>(
      "constraint_meaning",
      constraint_meaning,
      "Meanings of the numerical values given in constrain_value");
  params.addRequiredParam<std::vector<std::string>>(
      "constraint_species",
      "Names of the species that have their values fixed to constraint_value with meaning "
      "constraint_meaning.  All basis species (after swap_into_basis and swap_out_of_basis) must "
      "be provided with exactly one constraint");
  params.addRequiredParam<std::vector<Real>>(
      "constraint_value", "Numerical value of the containts on constraint_species");
  params.addParam<unsigned int>("precision", 4, "Precision for printing values");
  params.addParam<Real>(
      "mol_cutoff",
      1E-40,
      "Information regarding species with molalities less than this amount will not be outputted");
  params.addRangeCheckedParam<Real>(
      "max_ionic_strength", 3.0, "max_ionic_strength >= 0.0", "Maximum value of ionic strength");
  params.addParam<unsigned>("extra_iterations_to_make_consistent",
                            0,
                            "Extra iterations to make the molalities, activities, etc, consistent "
                            "before commencing the solution process's Newton iterations");
  params.addParam<Real>("temperature", 25, "Temperature of the aqueous solution");
  params.addRangeCheckedParam<Real>(
      "stoichiometry_tolerance",
      1E-6,
      "stoichiometry_tolerance >= 0.0",
      "Swapping involves inverting matrices via a singular value decomposition. During this "
      "process: (1) if abs(singular value) < stoi_tol * L1norm(singular values), then the "
      "matrix is deemed singular (so the basis swap is deemed invalid); (2) if abs(any "
      "stoichiometric coefficient) < stoi_tol then it is set to zero.");
  params.addRequiredParam<std::string>(
      "charge_balance_species",
      "Charge balance will be enforced on this basis species.  After swaps have been performed, "
      "this must be in the basis");
  params.addParam<std::vector<std::string>>(
      "prevent_precipitation",
      "Mineral species in this list will be prevented from precipitating, irrespective of their "
      "saturation index, unless they are in the basis");
  params.addParam<Real>("abs_tol",
                        1E-10,
                        "If the residual of the algebraic system (measured in mol) is lower than "
                        "this value, the Newton process is deemed to have converged");
  params.addParam<Real>("rel_tol",
                        1E-200,
                        "If the residual of the algebraic system is lower than this value times "
                        "the initial residual, the Newton process is deemed to have converged");
  params.addRangeCheckedParam<Real>(
      "min_initial_molality",
      1E-20,
      "min_initial_molality > 0.0",
      "Minimum value of the initial-guess molality used in the Newton process");
  params.addParam<unsigned>(
      "max_iter",
      100,
      "Maximum number of Newton iterations allowed to solve one round of the algebraic system");
  params.addParam<bool>("verbose", false, "Print verbose information");
  params.addParam<Real>(
      "max_initial_residual",
      1E3,
      "Attempt to alter the initial-guess molalities so that the initial residual "
      "for the Newton process is less than this number of moles");
  params.addRangeCheckedParam<Real>(
      "swap_threshold",
      0.1,
      "swap_threshold >= 0.0",
      "If the molality of a basis species in the algebraic system falls below swap_threshold * "
      "abs_tol then it is swapped out of the basis.  The dimensions of swap_threshold are "
      "1/kg(solvent water)");
  params.addParam<unsigned>("ramp_max_ionic_strength",
                            20,
                            "The number of iterations over which to progressively increase the "
                            "maximum ionic strength (from zero to max_ionic_strength).  Increasing "
                            "this can help in convergence of the Newton process");
  params.addParam<bool>(
      "ionic_str_using_basis_only",
      false,
      "If set to true, ionic strength and stoichiometric ionic strength will be computed using "
      "only the basis molalities, ignoring molalities of equilibrium species.  Since basis "
      "molality is usually greater than equilibrium molality, and the whole Debye-Huckel concept "
      "of activity coefficients depending on ionic strength is only approximate in practice, "
      "setting this parameter true often results in a reasonable approximation.  It can aid in "
      "convergence since it eliminates problems associated with unphysical huge equilibrium "
      "molalities that can occur during Newton iteration to the solution");
  params.addClassDescription(
      "Solves a equilibrium geochemical reaction system, then outputs results");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_FINAL};
  return params;
}

EquilibriumReactionSolverOutput::EquilibriumReactionSolverOutput(const InputParameters & parameters)
  : Output(parameters),
    UserObjectInterface(this),
    _mgd(getUserObject<GeochemicalModelDefinition>("model_definition").getDatabase()),
    _num_basis(_mgd.basis_species_name.size()),
    _num_eqm(_mgd.eqm_species_name.size()),
    _swapper(_num_basis, getParam<Real>("stoichiometry_tolerance")),
    _initial_max_ionic_str(getParam<Real>("max_ionic_strength") /
                           (1.0 + getParam<unsigned>("ramp_max_ionic_strength"))),

    _is(_initial_max_ionic_str,
        _initial_max_ionic_str,
        getParam<bool>("ionic_str_using_basis_only")),
    _gac(_is),
    _egs(_mgd,
         _gac,
         _is,
         _swapper,
         getParam<std::vector<std::string>>("swap_out_of_basis"),
         getParam<std::vector<std::string>>("swap_into_basis"),
         getParam<std::string>("charge_balance_species"),
         getParam<std::vector<std::string>>("constraint_species"),
         getParam<std::vector<Real>>("constraint_value"),
         getParam<MultiMooseEnum>("constraint_meaning"),
         getParam<Real>("temperature"),
         getParam<unsigned>("extra_iterations_to_make_consistent"),
         getParam<Real>("min_initial_molality")),
    _solver(_mgd,
            _egs,
            _is,
            getParam<Real>("abs_tol"),
            getParam<Real>("rel_tol"),
            getParam<unsigned>("max_iter"),
            getParam<Real>("max_initial_residual"),
            getParam<Real>("swap_threshold") * getParam<Real>("abs_tol"),
            getParam<std::vector<std::string>>("prevent_precipitation"),
            getParam<Real>("max_ionic_strength"),
            getParam<unsigned>("ramp_max_ionic_strength")),
    _original_redox_lhs(_mgd.redox_lhs),
    _precision(getParam<unsigned int>("precision")),
    _stoi_tol(getParam<Real>("stoichiometry_tolerance")),
    _verbose(getParam<bool>("verbose")),
    _mol_cutoff(getParam<Real>("mol_cutoff")),
    _temperature(getParam<Real>("temperature")),
    _nernst_swap_out_of_basis(getParam<std::vector<std::string>>("nernst_swap_out_of_basis")),
    _nernst_swap_into_basis(getParam<std::vector<std::string>>("nernst_swap_into_basis"))
{
  if (_nernst_swap_out_of_basis.size() != _nernst_swap_into_basis.size())
    paramError("nernst_swap_out_of_basis", "must be of same size as nernst_swap_into_basis");
}

void
EquilibriumReactionSolverOutput::output(const ExecFlagType & type)
{
  if (!shouldOutput(type))
    return;

  // solve the geochemical system
  std::stringstream ss;
  unsigned tot_iter;
  Real abs_residual;
  _solver.solveSystem(ss, tot_iter, abs_residual);
  if (_verbose)
    _console << ss.str();

  // retrieve information
  const std::vector<Real> & basis_molality = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
  const std::vector<Real> & basis_activity = _egs.getBasisActivity();
  const std::vector<Real> & basis_act_coef = _egs.getBasisActivityCoefficient();
  const std::vector<Real> & bulk_moles = _egs.getBulkMoles();
  const std::vector<Real> & eqm_molality = _egs.getEquilibriumMolality();
  const std::vector<Real> & eqm_act_coef = _egs.getEquilibriumActivityCoefficient();
  const std::vector<Real> & eqm_SI = _egs.getSaturationIndices();

  _console << std::setprecision(_precision);

  _console << "\nSummary:\n";

  _console << "Total number of iterations required = " << tot_iter << "\n";
  _console << "Error in calculation = " << abs_residual << "mol\n";
  _console << "Charge of solution = " << _egs.getTotalCharge() << "mol";
  _console << " (charge-balance species = "
           << _mgd.basis_species_name[_egs.getChargeBalanceBasisIndex()] << ")\n";

  _console << "Mass of solvent water = " << basis_molality[0] << "kg\n";

  Real mass = basis_molality[0];
  for (unsigned i = 1; i < _num_basis; ++i) // do not loop over water
    mass += bulk_moles[i] * _mgd.basis_species_molecular_weight[i] / 1000.0;
  _console << "Mass of aqueous solution = " << mass << "kg\n";

  // Output the aqueous solution pH, if relevant
  if (_mgd.basis_species_index.count("H+"))
    _console << "pH = " << -std::log10(basis_activity[_mgd.basis_species_index.at("H+")]) << "\n";
  if (_mgd.eqm_species_index.count("H+"))
    _console << "pH = "
             << -std::log10(eqm_molality[_mgd.eqm_species_index.at("H+")] *
                            eqm_act_coef[_mgd.eqm_species_index.at("H+")])
             << "\n";

  // Output the aqueous solution pe, if relevant
  Real pe = 0.0;
  const bool pe_defined =
      (_mgd.basis_species_index.count("e-") == 1) || (_mgd.eqm_species_index.count("e-") == 1);
  if (_mgd.basis_species_index.count("e-"))
    pe = -std::log10(basis_activity[_mgd.basis_species_index.at("e-")]);
  if (_mgd.eqm_species_index.count("e-"))
    pe = -std::log10(eqm_molality[_mgd.eqm_species_index.at("e-")] *
                     eqm_act_coef[_mgd.eqm_species_index.at("e-")]);
  if (pe_defined)
    _console << "pe = " << pe << "\n";

  // Output ionic strengths
  _console << "Ionic strength = " << _egs.getIonicStrength() << "mol/kg(solvent water)\n";
  _console << "Stoichiometric ionic strength = " << _egs.getStoichiometricIonicStrength()
           << "mol/kg(solvent water)\n";

  // Output activity of water
  _console << "Activity of water = " << basis_activity[0] << "\n";

  // Output the basis species information, sorted by molality
  std::vector<unsigned> basis_order =
      GeochemistrySortedIndices::sortedIndices(basis_molality, false);
  _console << "\nBasis Species:\n";
  for (const auto & i : basis_order)
    if (i == 0 || _mgd.basis_species_gas[i])
      continue;
    else
    {
      _console << _mgd.basis_species_name[i] << ";  bulk_moles = " << bulk_moles[i]
               << "mol;  bulk_conc = "
               << bulk_moles[i] * _mgd.basis_species_molecular_weight[i] * 1000.0 / mass
               << "mg/kg(solution);  molality = " << basis_molality[i]
               << "mol/kg(solvent water);  free_conc = "
               << basis_molality[i] * _mgd.basis_species_molecular_weight[i] * 1000.0
               << "mg/kg(solvent water)";
      if (_mgd.basis_species_mineral[i])
        _console << "\n";
      else
        _console << ";  act_coeff = " << basis_act_coef[i]
                 << ";  log10(a) = " << std::log10(basis_activity[i]) << "\n";
    }
  for (unsigned i = 0; i < _num_basis; ++i)
    if (_mgd.basis_species_gas[i])
      _console << _mgd.basis_species_name[i] << ";  fugacity = " << basis_activity[i] << "\n";

  // Output the equilibrium species info, sorted by molality
  std::vector<unsigned> eqm_order = GeochemistrySortedIndices::sortedIndices(eqm_molality, false);
  _console << "\nEquilibrium Species:\n";
  for (const auto & i : eqm_order)
    if (eqm_molality[i] <= _mol_cutoff)
      break;
    else if (_mgd.eqm_species_gas[i])
      continue;
    else
      _console << _mgd.eqm_species_name[i] << ";  molality = " << eqm_molality[i]
               << "mol/kg(solvent water);  free_conc = "
               << eqm_molality[i] * _mgd.eqm_species_molecular_weight[i] * 1000.0
               << "mg/kg(solvent water);  act_coeff = " << eqm_act_coef[i]
               << ";  log10(a) = " << std::log10(eqm_molality[i] * eqm_act_coef[i]) << ";  "
               << _mgd.eqm_species_name[i] << " = "
               << GeochemistryFormattedOutput::reaction(
                      _mgd.eqm_stoichiometry, i, _mgd.basis_species_name, _stoi_tol, _precision)
               << ";  log10K = " << _egs.getLog10K(i) << "\n";
  for (unsigned i = 0; i < _num_eqm; ++i)
    if (_mgd.eqm_species_gas[i])
      _console << _mgd.eqm_species_name[i]
               << ";  act_coeff = " << _egs.getEquilibriumActivityCoefficient(i) << ";  "
               << _mgd.eqm_species_name[i] << " = "
               << GeochemistryFormattedOutput::reaction(
                      _mgd.eqm_stoichiometry, i, _mgd.basis_species_name, _stoi_tol, _precision)
               << ";  log10K = " << _egs.getLog10K(i) << "\n";

  // Output the mineral info, sorted by saturation indices
  std::vector<unsigned> mineral_order = GeochemistrySortedIndices::sortedIndices(eqm_SI, false);
  _console << "\nMinerals:\n";
  for (const auto & i : mineral_order)
    if (_mgd.eqm_species_mineral[i])
      _console << _mgd.eqm_species_name[i] << " = "
               << GeochemistryFormattedOutput::reaction(
                      _mgd.eqm_stoichiometry, i, _mgd.basis_species_name, _stoi_tol, _precision)
               << ";  log10K = " << _egs.getLog10K(i) << ";  SI = " << eqm_SI[i] << "\n";

  // Output the Nernst potentials, if relevant
  const Real prefactor = -GeochemistryConstants::LOGTEN * GeochemistryConstants::GAS_CONSTANT *
                         (_temperature + GeochemistryConstants::CELSIUS_TO_KELVIN) /
                         GeochemistryConstants::FARADAY;
  _console << "\nNernst potentials:\n";
  if (pe_defined)
    _console << "e- = 0.5*H20 - 0.25*O2(aq) - 1*H+;  Eh = " << -prefactor * pe << "V\n";
  performNernstSwaps();
  if (_mgd.redox_lhs == _original_redox_lhs)
    for (unsigned red = 0; red < _mgd.redox_stoichiometry.m(); ++red)
      _console << _mgd.redox_lhs << " = "
               << GeochemistryFormattedOutput::reaction(
                      _mgd.redox_stoichiometry, red, _mgd.basis_species_name, _stoi_tol, _precision)
               << ";  Eh = "
               << prefactor * (_egs.log10RedoxActivityProduct(red) - _egs.getRedoxLog10K(red))
               << "V\n";

  const unsigned num_pot = _egs.getNumSurfacePotentials();
  if (num_pot > 0)
  {
    _console << "\nSorbing surfaces:\n";
    const std::vector<Real> area = _egs.getSorbingSurfaceArea();
    for (unsigned sp = 0; sp < num_pot; ++sp)
      _console << _mgd.surface_sorption_name[sp] << "; area = " << area[sp]
               << "m^2; specific charge = " << _egs.getSurfaceCharge(sp)
               << "C/m^2; surface potential = " << _egs.getSurfacePotential(sp) << "V\n";
  }
}

void
EquilibriumReactionSolverOutput::performNernstSwaps()
{
  // attempt the swaps specified by the user.  It is not an error if these are impossible
  for (unsigned sw = 0; sw < _nernst_swap_out_of_basis.size(); ++sw)
    if (_mgd.basis_species_index.count(_nernst_swap_out_of_basis[sw]) == 1 &&
        _mgd.eqm_species_index.count(_nernst_swap_into_basis[sw]) == 1)
      try
      {
        _egs.performSwap(_mgd.basis_species_index.at(_nernst_swap_out_of_basis[sw]),
                         _mgd.eqm_species_index.at(_nernst_swap_into_basis[sw]));
      }
      catch (const MooseException & e)
      {
        // it is not an error to be unable to make the swap, so just continue
        mooseWarning("Swapping ",
                     _nernst_swap_out_of_basis[sw],
                     " and ",
                     _nernst_swap_into_basis[sw],
                     ": ",
                     e.what());
      }
  if (_mgd.redox_lhs != _original_redox_lhs &&
      _mgd.basis_species_index.count(_original_redox_lhs) == 1 &&
      _mgd.eqm_species_index.count(_mgd.redox_lhs) == 1)
    try
    {
      // at some stage the original redox left-hand side must have been put into the basis, so let's
      // try to take it out and replace with _mgd.redox_lhs
      _egs.performSwap(_mgd.basis_species_index.at(_original_redox_lhs),
                       _mgd.eqm_species_index.at(_mgd.redox_lhs));
    }
    catch (const MooseException & e)
    {
      // it is not an error to be unable to make the swap, so just continue
    }
}
