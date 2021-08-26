//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryConsoleOutput.h"
#include "GeochemistryConstants.h"
#include "GeochemistryFormattedOutput.h"
#include "GeochemistrySortedIndices.h"

registerMooseObject("GeochemistryApp", GeochemistryConsoleOutput);

InputParameters
GeochemistryConsoleOutput::sharedParams()
{
  InputParameters params = emptyInputParameters();
  params.addParam<unsigned int>("precision", 4, "Precision for printing values");
  params.addParam<Real>(
      "mol_cutoff",
      1E-40,
      "Information regarding species with molalities less than this amount will not be outputted");
  params.addParam<bool>(
      "solver_info",
      false,
      "Print information (to the console) from the solver including residuals, swaps, etc");
  return params;
}

InputParameters
GeochemistryConsoleOutput::validParams()
{
  InputParameters params = Output::validParams();
  params += GeochemistryConsoleOutput::sharedParams();
  params.addRequiredParam<UserObjectName>("geochemistry_reactor",
                                          "The name of the GeochemistryReactor UserObject");
  params.addRangeCheckedParam<Real>("stoichiometry_tolerance",
                                    1E-6,
                                    "stoichiometry_tolerance >= 0.0",
                                    "if abs(any stoichiometric coefficient) < stoi_tol then it is "
                                    "set to zero, and so will not appear in the output");
  params.addRequiredParam<UserObjectName>(
      "nearest_node_number_UO",
      "The NearestNodeNumber UserObject that defines the physical point at which to query the "
      "GeochemistryReactor");
  params.addClassDescription("Outputs results from a GeochemistryReactor at a particular point");
  return params;
}

GeochemistryConsoleOutput::GeochemistryConsoleOutput(const InputParameters & parameters)
  : Output(parameters),
    UserObjectInterface(this),
    _reactor(getUserObject<GeochemistryReactorBase>("geochemistry_reactor")),
    _nnn(getUserObject<NearestNodeNumberUO>("nearest_node_number_UO")),
    _precision(getParam<unsigned int>("precision")),
    _stoi_tol(getParam<Real>("stoichiometry_tolerance")),
    _solver_info(getParam<bool>("solver_info")),
    _mol_cutoff(getParam<Real>("mol_cutoff"))
{
}

void
GeochemistryConsoleOutput::output(const ExecFlagType & type)
{
  if (!shouldOutput(type))
    return;
  const Node * closest_node = _nnn.getClosestNode();
  if (!closest_node)
    return;
  const dof_id_type closest_id = closest_node->id();

  if (_solver_info)
    _console << _reactor.getSolverOutput(closest_id).str();

  // retrieve information
  const GeochemicalSystem & egs = _reactor.getGeochemicalSystem(closest_id);
  const unsigned num_basis = egs.getNumInBasis();
  const unsigned num_eqm = egs.getNumInEquilibrium();
  const unsigned num_kin = egs.getNumKinetic();
  const std::vector<Real> & basis_molality = egs.getSolventMassAndFreeMolalityAndMineralMoles();
  const std::vector<Real> & basis_activity = egs.getBasisActivity();
  const std::vector<Real> & basis_act_coef = egs.getBasisActivityCoefficient();
  const std::vector<Real> & bulk_moles = egs.getBulkMolesOld();
  const std::vector<Real> & eqm_molality = egs.getEquilibriumMolality();
  const std::vector<Real> & eqm_act_coef = egs.getEquilibriumActivityCoefficient();
  const std::vector<Real> & eqm_SI = egs.getSaturationIndices();
  const std::vector<Real> & kin_moles = egs.getKineticMoles();
  const ModelGeochemicalDatabase & mgd = egs.getModelGeochemicalDatabase();

  _console << std::setprecision(_precision);

  _console << "\nSummary:\n";

  _console << "Total number of iterations required = " << _reactor.getSolverIterations(closest_id)
           << "\n";
  _console << "Error in calculation = " << _reactor.getSolverResidual(closest_id) << "mol\n";
  _console << "Charge of solution = " << egs.getTotalChargeOld() << "mol";
  _console << " (charge-balance species = "
           << mgd.basis_species_name[egs.getChargeBalanceBasisIndex()] << ")\n";

  _console << "Mass of solvent water = " << basis_molality[0] << "kg\n";

  Real mass = bulk_moles[0] / GeochemistryConstants::MOLES_PER_KG_WATER;
  for (unsigned i = 1; i < num_basis; ++i) // do not loop over water
    mass += bulk_moles[i] * mgd.basis_species_molecular_weight[i] / 1000.0;
  _console << "Total mass = " << mass << "kg";
  if (num_kin == 0)
    _console << "\n";
  else
  {
    _console << " (including kinetic species and free minerals)\n";
    for (unsigned k = 0; k < num_kin; ++k)
      mass -= kin_moles[k] * mgd.kin_species_molecular_weight[k] / 1000.0;
    _console << "Mass without kinetic species but including free minerals = " << mass << "kg\n";
  }
  // remove the free minerals
  for (unsigned i = 1; i < num_basis; ++i) // do not loop over water
    if (mgd.basis_species_mineral[i])
      mass -= basis_molality[i] * mgd.basis_species_molecular_weight[i] / 1000.0;
  // remove surface complexes
  for (const auto & name_info :
       mgd.surface_complexation_info) // all minerals involved in surface complexation
    for (const auto & name_frac :
         name_info.second.sorption_sites) // all sorption sites on the given mineral
    {
      const unsigned i =
          mgd.basis_species_index.at(name_frac.first); // i = basis_index_of_sorption_site
      mass -= basis_molality[i] * mgd.basis_species_molecular_weight[i] / 1000.0;
    }
  _console << "Mass of aqueous solution = " << mass << "kg";
  if (num_kin == 0)
    _console << " (without free minerals)\n";
  else
    _console << " (without kinetic species and without free minerals)\n";

  // Output the aqueous solution pH, if relevant
  if (mgd.basis_species_index.count("H+"))
    _console << "pH = " << -std::log10(basis_activity[mgd.basis_species_index.at("H+")]) << "\n";
  if (mgd.eqm_species_index.count("H+"))
    _console << "pH = "
             << -std::log10(eqm_molality[mgd.eqm_species_index.at("H+")] *
                            eqm_act_coef[mgd.eqm_species_index.at("H+")])
             << "\n";

  // Output the aqueous solution pe, if relevant
  if (mgd.redox_stoichiometry.m() > 0)
    _console << "pe = " << egs.getRedoxLog10K(0) - egs.log10RedoxActivityProduct(0) << "\n";

  // Output ionic strengths
  _console << "Ionic strength = " << egs.getIonicStrength() << "mol/kg(solvent water)\n";
  _console << "Stoichiometric ionic strength = " << egs.getStoichiometricIonicStrength()
           << "mol/kg(solvent water)\n";

  // Output activity of water
  _console << "Activity of water = " << basis_activity[0] << "\n";

  // Output temperature
  _console << "Temperature = " << egs.getTemperature() << "\n";

  // Output the basis species information, sorted by molality
  std::vector<unsigned> basis_order =
      GeochemistrySortedIndices::sortedIndices(basis_molality, false);
  _console << "\nBasis Species:\n";
  for (const auto & i : basis_order)
    if (i == 0 || mgd.basis_species_gas[i])
      continue;
    else
    {
      _console << mgd.basis_species_name[i] << ";  bulk_moles = " << bulk_moles[i]
               << "mol;  bulk_conc = "
               << bulk_moles[i] * mgd.basis_species_molecular_weight[i] * 1000.0 / mass
               << "mg/kg(soln);";
      if (!mgd.basis_species_mineral[i])
        _console << "  molality = " << basis_molality[i] << "mol/kg(solvent water);  free_conc = "
                 << basis_molality[i] * basis_molality[0] / mass *
                        mgd.basis_species_molecular_weight[i] * 1000.0
                 << "mg/kg(soln);  act_coeff = " << basis_act_coef[i]
                 << ";  log10(a) = " << std::log10(basis_activity[i]) << "\n";
      else if (mgd.basis_species_mineral[i])
        _console << "  free_moles = " << basis_molality[i] << "mol;  free_mass = "
                 << basis_molality[i] * mgd.basis_species_molecular_weight[i] * 1000.0 << "mg\n";
    }
  for (unsigned i = 0; i < num_basis; ++i)
    if (mgd.basis_species_gas[i])
      _console << mgd.basis_species_name[i] << ";  fugacity = " << basis_activity[i] << "\n";

  // Output the equilibrium species info, sorted by molality
  std::vector<unsigned> eqm_order = GeochemistrySortedIndices::sortedIndices(eqm_molality, false);
  _console << "\nEquilibrium Species:\n";
  for (const auto & i : eqm_order)
    if (eqm_molality[i] <= _mol_cutoff)
      break;
    else if (mgd.eqm_species_gas[i])
      continue;
    else
      _console << mgd.eqm_species_name[i] << ";  molality = " << eqm_molality[i]
               << "mol/kg(solvent water);  free_conc = "
               << eqm_molality[i] * basis_molality[0] / mass * mgd.eqm_species_molecular_weight[i] *
                      1000.0
               << "mg/kg(soln);  act_coeff = " << eqm_act_coef[i]
               << ";  log10(a) = " << std::log10(eqm_molality[i] * eqm_act_coef[i]) << ";  "
               << mgd.eqm_species_name[i] << " = "
               << GeochemistryFormattedOutput::reaction(
                      mgd.eqm_stoichiometry, i, mgd.basis_species_name, _stoi_tol, _precision)
               << ";  log10K = " << egs.getLog10K(i) << "\n";
  for (unsigned i = 0; i < num_eqm; ++i)
    if (mgd.eqm_species_gas[i])
    {
      Real log10f = 0;
      for (unsigned basis_i = 0; basis_i < num_basis; ++basis_i)
        log10f += mgd.eqm_stoichiometry(i, basis_i) * std::log10(basis_activity[basis_i]);
      log10f -= egs.getLog10K(i);
      _console << mgd.eqm_species_name[i]
               << ";  act_coeff = " << egs.getEquilibriumActivityCoefficient(i)
               << ";  fugacity = " << std::pow(10.0, log10f) << ";  " << mgd.eqm_species_name[i]
               << " = "
               << GeochemistryFormattedOutput::reaction(
                      mgd.eqm_stoichiometry, i, mgd.basis_species_name, _stoi_tol, _precision)
               << ";  log10K = " << egs.getLog10K(i) << "\n";
    }

  // Output the kinetic species information, sorted by mole number
  std::vector<unsigned> kin_order = GeochemistrySortedIndices::sortedIndices(kin_moles, false);
  _console << "\nKinetic Species:\n";
  for (const auto & k : kin_order)
  {
    _console << mgd.kin_species_name[k] << ";  moles = " << kin_moles[k]
             << ";  mass = " << kin_moles[k] * mgd.kin_species_molecular_weight[k] * 1000.0
             << "mg;  ";
    if (mgd.kin_species_mineral[k])
      _console << "volume = " << kin_moles[k] * mgd.kin_species_molecular_volume[k] << "cm^3;  ";
    _console << mgd.kin_species_name[k] << " = "
             << GeochemistryFormattedOutput::reaction(
                    mgd.kin_stoichiometry, k, mgd.basis_species_name, _stoi_tol, _precision)
             << ";  log10(Q) = " << egs.log10KineticActivityProduct(k)
             << ";  log10K = " << egs.getKineticLog10K(k)
             << ";  dissolution_rate*dt = " << -_reactor.getMoleAdditions(closest_id)(num_basis + k)
             << "\n";
  }

  // Output the mineral info, sorted by saturation indices
  std::vector<unsigned> mineral_order = GeochemistrySortedIndices::sortedIndices(eqm_SI, false);
  _console << "\nMinerals:\n";
  for (const auto & i : mineral_order)
    if (mgd.eqm_species_mineral[i])
      _console << mgd.eqm_species_name[i] << " = "
               << GeochemistryFormattedOutput::reaction(
                      mgd.eqm_stoichiometry, i, mgd.basis_species_name, _stoi_tol, _precision)
               << ";  log10K = " << egs.getLog10K(i) << ";  SI = " << eqm_SI[i] << "\n";

  // Output the Nernst potentials, if relevant
  _console << "\nNernst potentials:\n";
  if (mgd.redox_stoichiometry.m() > 0)
    outputNernstInfo(egs);

  const unsigned num_pot = egs.getNumSurfacePotentials();
  if (num_pot > 0)
  {
    _console << "\nSorbing surfaces:\n";
    const std::vector<Real> area = egs.getSorbingSurfaceArea();
    for (unsigned sp = 0; sp < num_pot; ++sp)
      _console << mgd.surface_sorption_name[sp] << "; area = " << area[sp]
               << "m^2; specific charge = " << egs.getSurfaceCharge(sp)
               << "C/m^2; surface potential = " << egs.getSurfacePotential(sp) << "V\n";
  }

  DenseVector<Real> bulk_in_original_basis = egs.getBulkOldInOriginalBasis();
  DenseVector<Real> transported_bulk_in_original_basis = egs.getTransportedBulkInOriginalBasis();
  std::vector<std::string> original_basis_names =
      _reactor.getPertinentGeochemicalSystem().originalBasisNames();
  _console << "\nIn original basis:\n";
  for (unsigned i = 0; i < num_basis; ++i)
    _console << original_basis_names[i] << ";  total_bulk_moles = " << bulk_in_original_basis(i)
             << ";  transported_bulk_moles = " << transported_bulk_in_original_basis(i) << "\n";

  _console << std::flush;
}

void
GeochemistryConsoleOutput::outputNernstInfo(const GeochemicalSystem & egs_ref) const
{
  // Copy egs_ref so we can call non-const methods (viz, swaps).  This does not copy the data in
  // egs.getModelGeochemicalDatabase(), only the reference (both references refer to the same
  // block of memory) and unfortunately the swaps below manipulate that memory
  GeochemicalSystem egs = egs_ref;
  // Since we only want to do the swaps to print out some Nernst info, but don't want to impact
  // the rest of the simulation, copy the mgd, so that we can copy it back into the aforementioned
  // block of memory at the end of this method
  ModelGeochemicalDatabase mgd_without_nernst_swaps = egs.getModelGeochemicalDatabase();

  const ModelGeochemicalDatabase & mgd_ref = egs.getModelGeochemicalDatabase();
  // attempt to undo the swaps that have been done to mgd_ref.
  // NOTE FOR FUTURE: In the current code (2020, June 1) swapping the redox stuff in mgd seems
  // redundant.  While the current code does these swaps, here we just undo all the swaps again to
  // write the redox stuff in the original basis.  Since this is the only place that the redox
  // stuff is used, doing any swaps on the redox stuff is currently just a waste of time! take a
  // copy of the following because they get modified during the swaps
  const std::vector<unsigned> have_swapped_out_of_basis = mgd_ref.have_swapped_out_of_basis;
  const std::vector<unsigned> have_swapped_into_basis = mgd_ref.have_swapped_into_basis;
  for (int sw = have_swapped_out_of_basis.size() - 1; sw >= 0; --sw)
  {
    // Don't check for gases being swapped in or out of the basis (which usually can't happen in
    // the middle of a Newton process) because we're going to trash all the swaps at the end of
    // this method, and we don't have to worry about bulk moles, etc: we just want the
    // stoichiometries and the activities
    try
    {
      egs.performSwapNoCheck(have_swapped_out_of_basis[sw], have_swapped_into_basis[sw]);
    }
    catch (const MooseException & e)
    {
      const std::string to_swap_in = mgd_ref.eqm_species_name[have_swapped_into_basis[sw]];
      const std::string to_swap_out = mgd_ref.basis_species_name[have_swapped_out_of_basis[sw]];
      // Don't crash the entire simulation, just because Nernst-related swapping does not work
      mooseWarning("Swapping ", to_swap_out, " and ", to_swap_in, ": ", e.what());
    }
  }

  const Real prefactor = -GeochemistryConstants::LOGTEN * GeochemistryConstants::GAS_CONSTANT *
                         (egs.getTemperature() + GeochemistryConstants::CELSIUS_TO_KELVIN) /
                         GeochemistryConstants::FARADAY;

  if (mgd_ref.redox_lhs == egs_ref.getOriginalRedoxLHS())
  {
    std::vector<Real> eh(mgd_ref.redox_stoichiometry.m());
    for (unsigned red = 0; red < mgd_ref.redox_stoichiometry.m(); ++red)
      eh[red] = prefactor * (egs.log10RedoxActivityProduct(red) - egs.getRedoxLog10K(red));
    const std::vector<unsigned> eh_order = GeochemistrySortedIndices::sortedIndices(eh, false);
    for (const auto & red : eh_order)
      _console << mgd_ref.redox_lhs << " = "
               << GeochemistryFormattedOutput::reaction(mgd_ref.redox_stoichiometry,
                                                        red,
                                                        mgd_ref.basis_species_name,
                                                        _stoi_tol,
                                                        _precision)
               << ";  Eh = " << eh[red] << "V\n";
  }

  _console << std::flush;

  // restore the original mgd by copying the data in copy_of_mgd into the memory referenced by
  // egs.getModelGeochemicalDatabase()
  egs.setModelGeochemicalDatabase(mgd_without_nernst_swaps);
}
