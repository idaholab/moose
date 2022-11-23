//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalSolver.h"
#include "GeochemistrySortedIndices.h"

GeochemicalSolver::GeochemicalSolver(unsigned num_basis,
                                     unsigned num_kin,
                                     GeochemistryIonicStrength & is,
                                     Real abs_tol,
                                     Real rel_tol,
                                     unsigned max_iter,
                                     Real max_initial_residual,
                                     Real swap_threshold,
                                     unsigned max_swaps_allowed,
                                     const std::vector<std::string> & prevent_precipitation,
                                     Real max_ionic_strength,
                                     unsigned ramp_max_ionic_strength,
                                     bool evaluate_kin_always)
  : _is(is),
    _num_basis(num_basis),
    _num_kin(num_kin),
    _num_basis_in_algebraic_system(_num_basis),
    _num_in_algebraic_system(_num_basis + _num_kin),
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
    _max_swaps_allowed(max_swaps_allowed),
    _prevent_precipitation(prevent_precipitation),
    _max_ionic_strength(max_ionic_strength),
    _ramp_max_ionic_strength(ramp_max_ionic_strength),
    _evaluate_kin_always(evaluate_kin_always),
    _input_mole_additions(_num_basis + _num_kin),
    _input_dmole_additions(_num_basis + _num_kin, _num_basis + _num_kin)
{
  if (_ramp_max_ionic_strength > _max_iter)
    mooseError("GeochemicalSolver: ramp_max_ionic_strength must be less than max_iter");
  if (max_initial_residual <= 0.0)
    mooseError("GeochemicalSolver: max_initial_residual must be positive");
  if (max_ionic_strength < 0.0)
    mooseError("GeochemicalSolver: max_ionic_strength must not be negative");
  if (abs_tol < 0.0)
    mooseError("GeochemicalSolver: abs_tol must not be negative");
  if (rel_tol < 0.0)
    mooseError("GeochemicalSolver: rel_tol must not be negative");
  if (rel_tol == 0.0 && abs_tol == 0.0)
    mooseError("GeochemicalSolver: either rel_tol or abs_tol must be positive");
}

void
GeochemicalSolver::setMaxInitialResidual(Real max_initial_residual)
{
  if (max_initial_residual <= 0.0)
    mooseError("GeochemicalSolver: max_initial_residual must be positive");
  _max_initial_residual = max_initial_residual;
}

Real
GeochemicalSolver::getMaxInitialResidual() const
{
  return _max_initial_residual;
}

Real
GeochemicalSolver::computeResidual(const GeochemicalSystem & egs,
                                   DenseVector<Real> & residual,
                                   const DenseVector<Real> & mole_additions) const
{
  for (unsigned a_ind = 0; a_ind < _num_in_algebraic_system; ++a_ind)
    residual(a_ind) = egs.getResidualComponent(a_ind, mole_additions);
  return residual.l1_norm();
}

void
GeochemicalSolver::solveAndUnderrelax(const GeochemicalSystem & egs,
                                      DenseMatrix<Real> & jacobian,
                                      DenseVector<Real> & new_mol) const
{
  jacobian.lu_solve(_residual, new_mol);

  // at this point we want to do molality = molality - new_mol, but
  // Bethke recommends underrelaxation - probably want to do PETSc variational bounds in the
  // future
  Real one_over_delta = 1.0;
  const std::vector<Real> current_molality_and_pot = egs.getAlgebraicVariableValues();
  for (unsigned a_ind = 0; a_ind < _num_in_algebraic_system; ++a_ind)
    one_over_delta =
        std::max(one_over_delta, new_mol(a_ind) * 2.0 / current_molality_and_pot[a_ind]);
  for (unsigned a_ind = 0; a_ind < _num_in_algebraic_system; ++a_ind)
    new_mol(a_ind) = current_molality_and_pot[a_ind] - new_mol(a_ind) / one_over_delta;
}

bool
GeochemicalSolver::swapNeeded(const GeochemicalSystem & egs,
                              unsigned & swap_out_of_basis,
                              unsigned & swap_into_basis,
                              std::stringstream & ss) const
{
  bool swap_needed = false;

  const ModelGeochemicalDatabase & mgd = egs.getModelGeochemicalDatabase();

  // check if any basis minerals have negative free number of moles
  const std::vector<Real> & basis_molality = egs.getSolventMassAndFreeMolalityAndMineralMoles();
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
      else if (!mgd.basis_species_mineral[i] && egs.getInAlgebraicSystem()[i] &&
               egs.getChargeBalanceBasisIndex() != i)
      {
        // a non-mineral in the algebraic system has super low molality: try to find a legitimate
        // swap
        swap_out_of_basis = i;
        bool legitimate_swap_found = egs.getSwapper().findBestEqmSwap(swap_out_of_basis,
                                                                      mgd,
                                                                      egs.getEquilibriumMolality(),
                                                                      true,
                                                                      false,
                                                                      false,
                                                                      swap_into_basis);
        if (legitimate_swap_found)
        {
          ss << "Basis species " << mgd.basis_species_name[swap_out_of_basis]
             << " has very low molality of " << basis_molality[i] << " compared to "
             << _swap_threshold << ".  Swapping it with equilibrium species "
             << mgd.eqm_species_name[swap_into_basis] << std::endl;
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
    if (basis_molality[i] > 0.0)
    {
      // since we're going through basis_molality in ascending order, as soon as we get >0, we're
      // safe
      break;
    }
    else if (mgd.basis_species_mineral[i])
    {
      swap_needed = true;
      swap_out_of_basis = i;
      bool legitimate_swap_found = egs.getSwapper().findBestEqmSwap(swap_out_of_basis,
                                                                    mgd,
                                                                    egs.getEquilibriumMolality(),
                                                                    false,
                                                                    false,
                                                                    false,
                                                                    swap_into_basis);
      if (!legitimate_swap_found)
        mooseException("Cannot find a legitimate swap for mineral ",
                       mgd.basis_species_name[swap_out_of_basis]);
      ss << "Mineral " << mgd.basis_species_name[swap_out_of_basis]
         << " consumed.  Swapping it with equilibrium species "
         << mgd.eqm_species_name[swap_into_basis] << std::endl;
      break;
    }
  }
  if (swap_needed)
    return swap_needed;

  // check maximum saturation index is not positive
  const std::vector<Real> & eqm_SI = egs.getSaturationIndices();
  const std::vector<unsigned> mineral_SI_order =
      GeochemistrySortedIndices::sortedIndices(eqm_SI, false);

  for (const auto & j : mineral_SI_order)
    if (eqm_SI[j] > 0.0)
      if (mgd.eqm_species_mineral[j])
        if (std::find(_prevent_precipitation.begin(),
                      _prevent_precipitation.end(),
                      mgd.eqm_species_name[j]) == _prevent_precipitation.end())
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
            if (basis_molality[i] > 0.0 && i != egs.getChargeBalanceBasisIndex() &&
                !mgd.basis_species_gas[i] && !egs.getBasisActivityKnown()[i])
            {
              // don't want to swap out the charge-balance species or any gases of fixed fugacity
              // or any species with fixed activity
              const Real stoi = std::abs(mgd.eqm_stoichiometry(j, i)) / basis_molality[i];
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
                mgd.eqm_species_name[j]);
          ss << "Mineral " << mgd.eqm_species_name[j]
             << " supersaturated.  Swapping it with basis species "
             << mgd.basis_species_name[swap_out_of_basis] << std::endl;
          break;
        }
  return swap_needed;
}

bool
GeochemicalSolver::reduceInitialResidual(GeochemicalSystem & egs,
                                         Real dt,
                                         DenseVector<Real> & mole_additions,
                                         DenseMatrix<Real> & dmole_additions)
{
  const Real initial_r = _abs_residual;

  // to get an indication of whether we should increase or decrease molalities in the algorithm
  // below, find the median of the original molalities
  const std::vector<Real> & original_molality = egs.getAlgebraicBasisValues();
  const std::vector<unsigned> mol_order =
      GeochemistrySortedIndices::sortedIndices(original_molality, false);
  const Real median_molality = original_molality[mol_order[_num_basis_in_algebraic_system / 2]];

  // get the index order of the residual vector (largest first): ignore residuals for surface
  // potentials (we're only using _num_basis_in_algebraic_system, not _num_in_algebraic_system)
  unsigned ind = 0;
  std::vector<unsigned> res_order(_num_basis_in_algebraic_system);
  std::iota(res_order.begin(), res_order.end(), ind++);
  std::sort(res_order.begin(),
            res_order.end(),
            [&](int i, int j) { return std::abs(_residual(i)) > std::abs(_residual(j)); });

  const std::vector<Real> & original_molality_and_pot = egs.getAlgebraicVariableValues();
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
    egs.setAlgebraicVariables(new_molality_and_pot);
    if (_evaluate_kin_always)
    {
      mole_additions = _input_mole_additions;
      dmole_additions = _input_dmole_additions;
      egs.addKineticRates(dt, mole_additions, dmole_additions);
    }
    _abs_residual = computeResidual(egs, _residual, mole_additions);
    if (_abs_residual < initial_r)
      return true; // success: found a new molality that decreases the initial |R|

    // the above approach did not decrease |R|, so try using 1/multiplier
    new_molality_and_pot(a) = original_molality_and_pot[a] / multiplier;
    egs.setAlgebraicVariables(new_molality_and_pot);
    if (_evaluate_kin_always)
    {
      mole_additions = _input_mole_additions;
      dmole_additions = _input_dmole_additions;
      egs.addKineticRates(dt, mole_additions, dmole_additions);
    }
    _abs_residual = computeResidual(egs, _residual, mole_additions);
    if (_abs_residual < initial_r)
      return true; // success: found a new molality that decreases the initial |R|

    // the new molalities did not decrease |R|, so revert to the original molality, and move to
    // next-biggest residual component
    new_molality_and_pot(a) = original_molality_and_pot[a];
    egs.setAlgebraicVariables(new_molality_and_pot);
    if (_evaluate_kin_always)
    {
      mole_additions = _input_mole_additions;
      dmole_additions = _input_dmole_additions;
      egs.addKineticRates(dt, mole_additions, dmole_additions);
    }
    _abs_residual = computeResidual(egs, _residual, mole_additions);
  }
  return false;
}

void
GeochemicalSolver::solveSystem(GeochemicalSystem & egs,
                               std::stringstream & ss,
                               unsigned & tot_iter,
                               Real & abs_residual,
                               Real dt,
                               DenseVector<Real> & mole_additions,
                               DenseMatrix<Real> & dmole_additions)
{
  ss.str("");
  tot_iter = 0;
  unsigned num_swaps = 0;

  // capture the inputs expressed in the current basis
  _input_mole_additions = mole_additions;
  _input_dmole_additions = dmole_additions;

  const ModelGeochemicalDatabase & mgd = egs.getModelGeochemicalDatabase();

  bool still_swapping = true;
  while (still_swapping && num_swaps <= _max_swaps_allowed)
  {
    _num_basis_in_algebraic_system = egs.getNumBasisInAlgebraicSystem();
    _num_in_algebraic_system = egs.getNumInAlgebraicSystem();
    _residual = DenseVector<Real>(_num_in_algebraic_system);
    _jacobian = DenseMatrix<Real>(_num_in_algebraic_system, _num_in_algebraic_system);
    _new_mol = DenseVector<Real>(_num_in_algebraic_system);

    unsigned iter = 0;
    const Real max_is0 =
        std::min(1.0, (iter + 1.0) / (_ramp_max_ionic_strength + 1.0)) * _max_ionic_strength;
    _is.setMaxIonicStrength(max_is0);
    _is.setMaxStoichiometricIonicStrength(max_is0);

    mole_additions = _input_mole_additions;
    dmole_additions = _input_dmole_additions;
    egs.addKineticRates(dt, mole_additions, dmole_additions);

    _abs_residual = computeResidual(egs, _residual, mole_additions);
    bool reducing_initial_molalities = (_abs_residual > _max_initial_residual);
    const unsigned max_tries =
        _num_basis *
        unsigned(
            std::log(_abs_residual / _max_initial_residual) /
            std::log(1.1)); // assume each successful call to reduceInitialResidual reduces residual
                            // by about 1.1, but that the correct basis species has to be chosen
    unsigned tries = 0;
    while (reducing_initial_molalities && ++tries <= max_tries)
      reducing_initial_molalities = reduceInitialResidual(egs, dt, mole_additions, dmole_additions);

    ss << "iter = " << iter << " |R| = " << _abs_residual << std::endl;
    _res0_times_rel = _abs_residual * _rel_tol;
    while ((_abs_residual >= _res0_times_rel && _abs_residual >= _abs_tol && iter < _max_iter) ||
           iter < _ramp_max_ionic_strength)
    {
      iter += 1;
      tot_iter += 1;
      egs.computeJacobian(_residual, _jacobian, mole_additions, dmole_additions);
      solveAndUnderrelax(egs, _jacobian, _new_mol);
      const Real max_is =
          std::min(1.0, (iter + 1.0) / (_ramp_max_ionic_strength + 1.0)) * _max_ionic_strength;
      _is.setMaxIonicStrength(max_is);
      _is.setMaxStoichiometricIonicStrength(max_is);
      egs.setAlgebraicVariables(_new_mol);
      if (egs.alterChargeBalanceSpecies(_swap_threshold))
        ss << "Changed change balance species to "
           << mgd.basis_species_name.at(egs.getChargeBalanceBasisIndex()) << std::endl;
      if (_evaluate_kin_always)
      {
        mole_additions = _input_mole_additions;
        dmole_additions = _input_dmole_additions;
        egs.addKineticRates(dt, mole_additions, dmole_additions);
      }
      _abs_residual = computeResidual(egs, _residual, mole_additions);
      ss << "iter = " << iter << " |R| = " << _abs_residual << std::endl;
    }

    abs_residual = _abs_residual; // record the final residual

    if (iter >= _max_iter)
      ss << std::endl << "Warning: Number of iterations exceeds " << _max_iter << std::endl;

    unsigned swap_out_of_basis = 0;
    unsigned swap_into_basis = 0;
    try
    {
      // to ensure basis molality is correct:
      for (unsigned i = 0; i < _num_basis; ++i)
        egs.addToBulkMoles(i, mole_additions(i));
      // now check for small basis molalities, minerals consumed and minerals precipitated
      still_swapping = swapNeeded(egs, swap_out_of_basis, swap_into_basis, ss);
    }
    catch (const MooseException & e)
    {
      mooseException(e.what());
    }
    if (still_swapping)
    {
      // need to do a swap and re-solve
      try
      {
        // before swapping, remove any basis mole_additions that came from kinetics.  The
        // following loop over i, combined with the above egs.addToBulkMoles(mole_additions), where
        // mole_additions = _input_mole_additions + kinetic_additions, means that the bulk
        // moles in egs will have only been incremented by _input_mole_additions.  Hence,
        // _input_mole_additions can be set to zero in preparation for the next solve in the new
        // basis.
        // NOTE: if _input_mole_additions depend on molalities, this approach introduces error,
        // because the following call to addToBulkMoles and subsequent _input_mole_additions = 0
        // means the mole additions are FIXED
        for (unsigned i = 0; i < _num_basis; ++i)
        {
          egs.addToBulkMoles(i, _input_mole_additions(i) - mole_additions(i));
          _input_mole_additions(i) = 0.0;
          for (unsigned j = 0; j < _num_basis + _num_kin; ++j)
            _input_dmole_additions(i, j) = 0.0;
        }
        egs.performSwap(swap_out_of_basis, swap_into_basis);
        num_swaps += 1;
      }
      catch (const MooseException & e)
      {
        mooseException(e.what());
      }
    }
  }

  if (num_swaps > _max_swaps_allowed)
  {
    mooseException("Maximum number of swaps performed during solve");
  }
  else if (_abs_residual >= _res0_times_rel && _abs_residual >= _abs_tol)
  {
    mooseException("Failed to converge");
  }
  else
  {
    // the basis species additions have been added above with addtoBulkMoles: now add the kinetic
    // additions:
    for (unsigned i = 0; i < _num_basis; ++i)
      mole_additions(i) = 0.0;
    egs.updateOldWithCurrent(mole_additions);
    egs.enforceChargeBalance();
  }
}

void
GeochemicalSolver::setRampMaxIonicStrength(unsigned ramp_max_ionic_strength)
{
  if (ramp_max_ionic_strength > _max_iter)
    mooseError("GeochemicalSolver: ramp_max_ionic_strength must be less than max_iter");
  _ramp_max_ionic_strength = ramp_max_ionic_strength;
}

unsigned
GeochemicalSolver::getRampMaxIonicStrength() const
{
  return _ramp_max_ionic_strength;
}
