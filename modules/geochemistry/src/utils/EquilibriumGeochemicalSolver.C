//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EquilibriumGeochemicalSolver.h"
#include "GeochemistrySortedIndices.h"

EquilibriumGeochemicalSolver::EquilibriumGeochemicalSolver(
    const ModelGeochemicalDatabase & mgd,
    EquilibriumGeochemicalSystem & egs,
    GeochemistryIonicStrength & is,
    Real abs_tol,
    Real rel_tol,
    unsigned max_iter,
    Real max_initial_residual,
    Real swap_threshold,
    const std::vector<std::string> & prevent_precipitation,
    Real max_ionic_strength,
    unsigned ramp_max_ionic_strength)
  : _mgd(mgd),
    _egs(egs),
    _is(is),
    _num_basis(_egs.getNumInBasis()),
    _num_eqm(_egs.getNumInEquilibrium()),
    _num_basis_in_algebraic_system(_egs.getNumBasisInAlgebraicSystem()),
    _num_in_algebraic_system(_egs.getNumInAlgebraicSystem()),
    _residual(_num_in_algebraic_system),
    _abs_residual(0.0),
    _jacobian(_num_in_algebraic_system, _num_in_algebraic_system),
    _new_mol(_num_in_algebraic_system),
    _abs_tol(abs_tol),
    _rel_tol(rel_tol),
    _res0_times_rel(0.0),
    _max_iter(max_iter),
    _max_initial_residual(max_initial_residual),
    _swap_threshold(swap_threshold),
    _prevent_precipitation(prevent_precipitation),
    _max_ionic_strength(max_ionic_strength),
    _ramp_max_ionic_strength(ramp_max_ionic_strength)
{
  if (_ramp_max_ionic_strength > _max_iter)
    mooseError("EquilibriumGeochemicalSolver: ramp_max_ionic_strength must be less than max_iter");
  if (max_initial_residual <= 0.0)
    mooseError("EquilibriumGeochemicalSolver: max_initial_residual must be positive");
  if (max_ionic_strength < 0.0)
    mooseError("EquilibriumGeochemicalSolver: max_ionic_strength must not be negative");
  if (abs_tol < 0.0)
    mooseError("EquilibriumGeochemicalSolver: abs_tol must not be negative");
  if (rel_tol < 0.0)
    mooseError("EquilibriumGeochemicalSolver: rel_tol must not be negative");
  if (rel_tol == 0.0 && abs_tol == 0.0)
    mooseError("EquilibriumGeochemicalSolver: either rel_tol or abs_tol must be positive");
}

Real
EquilibriumGeochemicalSolver::computeResidual(DenseVector<Real> & residual) const
{
  for (unsigned a_ind = 0; a_ind < _num_in_algebraic_system; ++a_ind)
    residual(a_ind) = _egs.getResidualComponent(a_ind);
  return residual.l1_norm();
}

void
EquilibriumGeochemicalSolver::solveAndUnderrelax(DenseMatrix<Real> & jacobian,
                                                 DenseVector<Real> & new_mol) const
{
  jacobian.lu_solve(_residual, new_mol);

  // at this point we want to do molality = molality - new_mol, but
  // Bethke recommends underrelaxation - probably want to do PETSc variational bounds in the
  // future
  Real one_over_delta = 1.0;
  const std::vector<Real> current_molality_and_pot = _egs.getAlgebraicVariableValues();
  for (unsigned a_ind = 0; a_ind < _num_in_algebraic_system; ++a_ind)
    one_over_delta =
        std::max(one_over_delta, new_mol(a_ind) * 2.0 / current_molality_and_pot[a_ind]);
  for (unsigned a_ind = 0; a_ind < _num_in_algebraic_system; ++a_ind)
    new_mol(a_ind) = current_molality_and_pot[a_ind] - new_mol(a_ind) / one_over_delta;
}

bool
EquilibriumGeochemicalSolver::swapNeeded(unsigned & swap_out_of_basis,
                                         unsigned & swap_into_basis,
                                         std::stringstream & ss) const
{
  bool swap_needed = false;

  // check if any basis minerals have negative free number of moles
  const std::vector<Real> & basis_molality = _egs.getSolventMassAndFreeMolalityAndMineralMoles();
  const std::vector<unsigned> molality_order =
      GeochemistrySortedIndices::sortedIndices(basis_molality, true);

  // if the Newton did not converge then check for small molalities in the non-minerals
  if (_abs_residual > _res0_times_rel && _abs_residual > _abs_tol)
  {
    for (const auto & i : molality_order)
    {
      if (basis_molality[i] >= _swap_threshold)
      {
        // since we're going through basis_molality in ascending order, as soon as we get >=
        // _swap_threshold, we're safe
        break;
      }
      else if (!_mgd.basis_species_mineral[i] && _egs.getInAlgebraicSystem()[i] &&
               _egs.getChargeBalanceBasisIndex() != i)
      {
        // a non-mineral in the algebraic system has super low molality: try to find a legitimate
        // swap
        swap_out_of_basis = i;
        swap_into_basis = 0;

        bool legitimate_swap_found = false;
        Real best_stoi = 0.0;
        // the first non-gas is the best possible so far
        for (unsigned j = 0; j < _num_eqm; ++j)
        {
          if (_mgd.eqm_species_gas[j] || _mgd.eqm_stoichiometry(j, i) == 0.0 ||
              _mgd.surface_sorption_related[j])
            continue;
          best_stoi = std::abs(_mgd.eqm_stoichiometry(j, i)) * _egs.getEquilibriumMolality(j);
          swap_into_basis = j;
          legitimate_swap_found = true;
        }
        // now go through the remainder, trying to find a better swap
        for (unsigned j = swap_into_basis; j < _num_eqm; ++j)
        {
          if (_mgd.eqm_species_gas[j] || _mgd.eqm_stoichiometry(j, i) == 0.0 ||
              _mgd.surface_sorption_related[j])
            continue;
          const Real stoi = std::abs(_mgd.eqm_stoichiometry(j, i)) * _egs.getEquilibriumMolality(j);
          if (stoi > best_stoi)
          {
            best_stoi = stoi;
            swap_into_basis = j;
          }
        }
        if (legitimate_swap_found)
        {
          ss << "Basis species " << _mgd.basis_species_name[swap_out_of_basis]
             << " has very low molality of " << basis_molality[i] << " compared to "
             << _swap_threshold << ".  Swapping it with equilibrium species "
             << _mgd.eqm_species_name[swap_into_basis] << std::endl;
          swap_needed = true;
          break;
        }
        // if no legitimate swap is found, then loop around to the next basis species
      }
    }
  }
  if (swap_needed)
    return swap_needed;

  // now look through the molalities for minerals that are consumed
  for (const auto & i : molality_order)
  {
    if (basis_molality[i] >= 0.0)
    {
      // since we're going through basis_molality in ascending order, as soon as we get >=0, we're
      // safe
      break;
    }
    else if (_mgd.basis_species_mineral[i])
    {
      swap_needed = true;
      swap_out_of_basis = i;
      swap_into_basis = 0;

      bool legitimate_swap_found = false;
      Real best_stoi = 0.0;
      // the first non-mineral and non-gas is the best possible so far
      for (unsigned j = 0; j < _num_eqm; ++j)
      {
        if (_mgd.eqm_species_mineral[j] || _mgd.eqm_species_gas[j] ||
            _mgd.surface_sorption_related[j] || _mgd.eqm_stoichiometry(j, i) == 0.0)
          continue;
        best_stoi = std::abs(_mgd.eqm_stoichiometry(j, i)) * _egs.getEquilibriumMolality(j);
        swap_into_basis = j;
        legitimate_swap_found = true;
      }
      // now go through the remainder, trying to find a better swap
      for (unsigned j = swap_into_basis; j < _num_eqm; ++j)
      {
        if (_mgd.eqm_species_mineral[j] || _mgd.eqm_species_gas[j] ||
            _mgd.surface_sorption_related[j] || _mgd.eqm_stoichiometry(j, i) == 0.0)
          continue;
        const Real stoi = std::abs(_mgd.eqm_stoichiometry(j, i)) * _egs.getEquilibriumMolality(j);
        if (stoi > best_stoi)
        {
          best_stoi = stoi;
          swap_into_basis = j;
        }
      }
      if (!legitimate_swap_found)
        mooseException("Cannot find a legitimate swap for mineral ",
                       _mgd.basis_species_name[swap_out_of_basis]);
      ss << "Mineral " << _mgd.basis_species_name[swap_out_of_basis]
         << " consumed.  Swapping it with equilibrium species "
         << _mgd.eqm_species_name[swap_into_basis] << std::endl;
      break;
    }
  }
  if (swap_needed)
    return swap_needed;

  // check maximum saturation index is not positive
  const std::vector<Real> & eqm_SI = _egs.getSaturationIndices();
  const std::vector<unsigned> mineral_SI_order =
      GeochemistrySortedIndices::sortedIndices(eqm_SI, false);

  for (const auto & j : mineral_SI_order)
    if (eqm_SI[j] > 0.0)
      if (_mgd.eqm_species_mineral[j])
        if (std::find(_prevent_precipitation.begin(),
                      _prevent_precipitation.end(),
                      _mgd.eqm_species_name[j]) == _prevent_precipitation.end())
        {
          // mineral has positive saturation index and user is not preventing its precipitation.
          // determine the basis species to swap out
          swap_needed = true;
          swap_into_basis = j;
          bool legitimate_swap_found = false;
          swap_out_of_basis = 0;
          Real best_stoi = 0.0;
          for (unsigned i = 1; i < _num_basis; ++i) // never swap water (i=0)
          {
            if (basis_molality[i] > 0.0 && i != _egs.getChargeBalanceBasisIndex() &&
                !_mgd.basis_species_gas[i] && !_egs.getBasisActivityKnown()[i])
            {
              // don't want to swap out the charge-balance species or any gases of fixed fugacity
              // or any species with fixed activity
              const Real stoi = std::abs(_mgd.eqm_stoichiometry(j, i)) / basis_molality[i];
              if (stoi > best_stoi)
              {
                legitimate_swap_found = true;
                best_stoi = stoi;
                swap_out_of_basis = i;
              }
            }
          }
          if (!legitimate_swap_found)
            mooseException(
                "Cannot find a legitimate swap for the supersaturated equilibrium species ",
                _mgd.eqm_species_name[j]);
          ss << "Mineral " << _mgd.eqm_species_name[j]
             << " supersaturated.  Swapping it with basis species "
             << _mgd.basis_species_name[swap_out_of_basis] << std::endl;
          break;
        }
  return swap_needed;
}

bool
EquilibriumGeochemicalSolver::reduceInitialResidual()
{
  const Real initial_r = _abs_residual;

  // to get an indication of whether we should increase or decrease molalities in the algorithm
  // below, find the median of the original molalities
  const std::vector<Real> & original_molality = _egs.getAlgebraicBasisValues();
  const std::vector<unsigned> mol_order =
      GeochemistrySortedIndices::sortedIndices(original_molality, false);
  const Real median_molality = original_molality[mol_order[_num_basis_in_algebraic_system / 2]];

  // get the index order of the residual vector (largest first): ignore residuals for surface
  // potentials (we're only using _num_basis_in_algebraic_system, not _num_in_algebraic_system)
  unsigned ind = 0;
  std::vector<unsigned> res_order(_num_basis_in_algebraic_system);
  std::iota(res_order.begin(), res_order.end(), ind++);
  std::sort(res_order.begin(), res_order.end(), [&](int i, int j) {
    return std::abs(_residual(i)) > std::abs(_residual(j));
  });

  const std::vector<Real> & original_molality_and_pot = _egs.getAlgebraicVariableValues();
  DenseVector<Real> new_molality_and_pot(original_molality_and_pot);
  for (const auto & a : res_order)
  {
    if (std::abs(_residual(a)) < _max_initial_residual)
      return false; // haven't managed to find a suitable new molality, and all remaining residual
                    // components are less than the cutoff, so cannot appreciably reduce from now
                    // on

    const Real multiplier = (original_molality_and_pot[a] > median_molality) ? 0.5 : 2.0;
    // try using the multiplier
    new_molality_and_pot(a) = multiplier * original_molality_and_pot[a];
    _egs.setAlgebraicVariables(new_molality_and_pot);
    _abs_residual = computeResidual(_residual);
    if (_abs_residual < initial_r)
      return true; // success: found a new molality that decreases the initial |R|

    // the above approach did not decrease |R|, so try using 1/multiplier
    new_molality_and_pot(a) = original_molality_and_pot[a] / multiplier;
    _egs.setAlgebraicVariables(new_molality_and_pot);
    _abs_residual = computeResidual(_residual);
    if (_abs_residual < initial_r)
      return true; // success: found a new molality that decreases the initial |R|

    // the new molalities did not decrease |R|, so revert to the original molality, and move to
    // next-biggest residual component
    new_molality_and_pot(a) = original_molality_and_pot[a];
    _egs.setAlgebraicVariables(new_molality_and_pot);
    _abs_residual = computeResidual(_residual);
  }
  return false;
}

void
EquilibriumGeochemicalSolver::solveSystem(std::stringstream & ss,
                                          unsigned & tot_iter,
                                          Real & abs_residual)
{
  ss.str("");
  tot_iter = 0;

  bool still_swapping = true;
  while (still_swapping)
  {
    _num_basis_in_algebraic_system = _egs.getNumBasisInAlgebraicSystem();
    _num_in_algebraic_system = _egs.getNumInAlgebraicSystem();
    _residual = DenseVector<Real>(_num_in_algebraic_system);
    _jacobian = DenseMatrix<Real>(_num_in_algebraic_system, _num_in_algebraic_system);
    _new_mol = DenseVector<Real>(_num_in_algebraic_system);

    unsigned iter = 0;
    const Real max_is0 =
        std::min(1.0, (iter + 1.0) / (_ramp_max_ionic_strength + 1.0)) * _max_ionic_strength;
    _is.setMaxIonicStrength(max_is0);
    _is.setMaxStoichiometricIonicStrength(max_is0);
    _abs_residual = computeResidual(_residual);
    bool reducing_initial_molalities = (_abs_residual > _max_initial_residual);
    const unsigned max_tries =
        _num_basis *
        unsigned(
            std::log(_abs_residual / _max_initial_residual) /
            std::log(1.1)); // assume each successful call to reduceInitialResidual reduces residual
                            // by about 1.1, but that the correct basis species has to be chosen
    unsigned tries = 0;
    while (reducing_initial_molalities && ++tries <= max_tries)
      reducing_initial_molalities = reduceInitialResidual();

    ss << "iter = " << iter << " |R| = " << _abs_residual << std::endl;
    _res0_times_rel = _abs_residual * _rel_tol;
    while ((_abs_residual >= _res0_times_rel && _abs_residual >= _abs_tol && iter < _max_iter) ||
           iter < _ramp_max_ionic_strength)
    {
      iter += 1;
      tot_iter += 1;
      _egs.computeJacobian(_residual, _jacobian);
      solveAndUnderrelax(_jacobian, _new_mol);
      const Real max_is =
          std::min(1.0, (iter + 1.0) / (_ramp_max_ionic_strength + 1.0)) * _max_ionic_strength;
      _is.setMaxIonicStrength(max_is);
      _is.setMaxStoichiometricIonicStrength(max_is);
      _egs.setAlgebraicVariables(_new_mol);
      if (_egs.alterChargeBalanceSpecies(_swap_threshold))
        ss << "Changed change balance species to "
           << _mgd.basis_species_name[_egs.getChargeBalanceBasisIndex()] << std::endl;
      _abs_residual = computeResidual(_residual);
      ss << "iter = " << iter << " |R| = " << _abs_residual << std::endl;
    }

    abs_residual = _abs_residual; // record the final residual

    _egs.enforceChargeBalance(); // just to get _bulk_moles of the charge-balance species correct.
                                 // This is not used within EquilibriumGeochemicalSystem, but may be
                                 // used in the output, so we should ensure it is set correctly here
    if (iter >= _max_iter)
      ss << std::endl << "Warning: Number of iterations exceeds " << _max_iter << std::endl;

    unsigned swap_out_of_basis = 0;
    unsigned swap_into_basis = 0;
    try
    {
      still_swapping = swapNeeded(swap_out_of_basis, swap_into_basis, ss);
    }
    catch (const MooseException & e)
    {
      mooseError(e.what());
    }
    if (still_swapping)
    {
      // need to do a swap and re-solve
      try
      {
        _egs.performSwap(swap_out_of_basis, swap_into_basis);
      }
      catch (const MooseException & e)
      {
        mooseError(e.what());
      }
    }
  }
}
