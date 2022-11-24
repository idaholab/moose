//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistrySpeciesSwapper.h"

GeochemistrySpeciesSwapper::GeochemistrySpeciesSwapper(unsigned basis_size, Real stoi_tol)
  : _stoi_tol(stoi_tol),
    _swap_matrix(basis_size, basis_size),
    _true_swap_matrix(basis_size, basis_size),
    _inv_swap_matrix(basis_size, basis_size),
    _swap_sigma(basis_size),
    _swap_U(basis_size, basis_size),
    _swap_VT(basis_size, basis_size)
{
}

void
GeochemistrySpeciesSwapper::checkSwap(const ModelGeochemicalDatabase & mgd,
                                      const std::string & replace_this,
                                      const std::string & with_this)
{
  if (replace_this == "H2O")
    mooseError("Cannot remove H2O from the basis");
  if (mgd.basis_species_index.count(replace_this) == 0)
    mooseError(replace_this + " is not in the basis, so cannot be removed from the basis");
  if (mgd.eqm_species_index.count(with_this) == 0)
    mooseError(with_this + " is not an equilibrium species, so cannot be "
                           "removed from the equilibrium species list");

  checkSwap(mgd, mgd.basis_species_index.at(replace_this), mgd.eqm_species_index.at(with_this));
}

void
GeochemistrySpeciesSwapper::checkSwap(const ModelGeochemicalDatabase & mgd,
                                      unsigned basis_index_to_replace,
                                      unsigned eqm_index_to_insert)
{
  const unsigned num_cols = mgd.basis_species_index.size();
  const unsigned num_rows = mgd.eqm_species_index.size();
  if (basis_index_to_replace == 0)
    mooseError("Cannot remove H2O from the basis");
  if (basis_index_to_replace >= num_cols)
    mooseError(basis_index_to_replace, " exceeds the number of basis species in the problem");
  if (eqm_index_to_insert >= num_rows)
    mooseError(eqm_index_to_insert, " exceeds the number of equilibrium species in the problem");
  if (mgd.surface_sorption_related[eqm_index_to_insert])
    mooseError(
        "Equilibrium species ",
        mgd.eqm_species_name[eqm_index_to_insert],
        " is involved in surface sorption so cannot be swapped into the basis.  If this is truly "
        "necessary, code enhancements will need to be made including: recording whether basis "
        "species are involved in surface sorption, including them in the surface-potential "
        "calculations, and carefully swapping surface-potential-modified equilibrium constants");

  constructInverseMatrix(mgd, basis_index_to_replace, eqm_index_to_insert);
}

void
GeochemistrySpeciesSwapper::constructInverseMatrix(const ModelGeochemicalDatabase & mgd,
                                                   unsigned basis_index_to_replace,
                                                   unsigned eqm_index_to_insert)
{
  const unsigned num_cols = mgd.basis_species_index.size();

  if (_swap_matrix.n() != num_cols)
    mooseError("GeochemistrySpeciesSwapper constructed with incorrect "
               "basis_species size");

  // This is a private method, called from checkSwap, so we know that all the inputs have been
  // sanity-checked.  The only way the swap could be invalid is that the swap matrix is not
  // invertible.  This could be due to the solve algorithm attempting to perform an invalid swap, so
  // only mooseExceptions are thrown below, instead of mooseError, to allow the solve algorithm a
  // chance to attempt another swap.

  // construct the swap matrix.  new_basis = swap_matrix * old_basis
  // We shove the equilibrium species into the "slot" of the basis species it is
  // replacing
  _swap_matrix.zero();
  for (unsigned i = 0; i < num_cols; ++i)
    _swap_matrix(i, i) = 1.0;
  for (unsigned i = 0; i < num_cols; ++i)
    _swap_matrix(basis_index_to_replace, i) = mgd.eqm_stoichiometry(eqm_index_to_insert, i);

  // record the swap matrix since the SVD trashes it
  _true_swap_matrix = _swap_matrix;

  // In order to invert _swap_matrix, perform the SVD: A = U * D * VT
  // Although every matrix has a SVD, this process might fail due to numerical errors.  Therefore
  // the following could be wrapped in a try-catch block, but I have never been able to trigger a
  // failure since _swap_matrix is always so well-formed due to all the checks on the database while
  // parsing it
  _swap_matrix.svd(_swap_sigma, _swap_U, _swap_VT);
  const Real l1norm = _swap_sigma.l1_norm();
  for (unsigned i = 0; i < num_cols; ++i)
    if (std::abs(_swap_sigma(i) / l1norm) < _stoi_tol)
      mooseException("Matrix is not invertible, which signals an invalid basis swap");

  // Find the inverse, which is VT^-1 * D^-1 * U^-1 = V * D^-1 * UT
  for (unsigned i = 0; i < num_cols; ++i)
    _swap_U.scale_column(i, 1.0 / _swap_sigma(i)); // (scale the columns of U)^T = D^-1 * UT
  for (unsigned i = 0; i < num_cols; ++i)
    for (unsigned j = 0; j < num_cols; ++j)
      _inv_swap_matrix(i, j) = _swap_VT(j, i);        // _inv_swap_matrix = V
  _inv_swap_matrix.right_multiply_transpose(_swap_U); // V * (scale the columns of U)^T
}

void
GeochemistrySpeciesSwapper::performSwap(ModelGeochemicalDatabase & mgd,
                                        const std::string & replace_this,
                                        const std::string & with_this)
{
  // check the swap is valid (if not then mooseError or mooseException)
  // and if it's valid, create the inverse swap matrix
  checkSwap(mgd, replace_this, with_this);

  // perform the swap inside the MGD datastructure
  alterMGD(mgd, mgd.basis_species_index.at(replace_this), mgd.eqm_species_index.at(with_this));
}

void
GeochemistrySpeciesSwapper::performSwap(ModelGeochemicalDatabase & mgd,
                                        unsigned basis_index_to_replace,
                                        unsigned eqm_index_to_insert)
{
  // check the swap is valid (if not then mooseError or mooseException)
  // and if it's valid, create the inverse swap matrix
  checkSwap(mgd, basis_index_to_replace, eqm_index_to_insert);

  // perform the swap inside the MGD datastructure
  alterMGD(mgd, basis_index_to_replace, eqm_index_to_insert);
}

void
GeochemistrySpeciesSwapper::performSwap(ModelGeochemicalDatabase & mgd,
                                        DenseVector<Real> & bulk_composition,
                                        const std::string & replace_this,
                                        const std::string & with_this)
{
  performSwap(mgd, replace_this, with_this);
  // compute the bulk composition expressed in the new basis
  alterBulkComposition(mgd.basis_species_index.size(), bulk_composition);
}

void
GeochemistrySpeciesSwapper::performSwap(ModelGeochemicalDatabase & mgd,
                                        DenseVector<Real> & bulk_composition,
                                        unsigned basis_index_to_replace,
                                        unsigned eqm_index_to_insert)
{
  performSwap(mgd, basis_index_to_replace, eqm_index_to_insert);
  // compute the bulk composition expressed in the new basis
  alterBulkComposition(mgd.basis_species_index.size(), bulk_composition);
}

void
GeochemistrySpeciesSwapper::alterMGD(ModelGeochemicalDatabase & mgd,
                                     unsigned basis_index_to_replace,
                                     unsigned eqm_index_to_insert)
{
  const unsigned num_cols = mgd.basis_species_index.size();
  const unsigned num_rows = mgd.eqm_species_index.size();
  const unsigned num_temperatures = mgd.eqm_log10K.n();
  const unsigned num_redox = mgd.redox_stoichiometry.m();
  const unsigned kin_rows = mgd.kin_stoichiometry.m();
  const unsigned num_rate = mgd.kin_rate.size();
  const unsigned pro_ind_eqm =
      num_cols + eqm_index_to_insert; // index of the (eqm species that is being swapped into the
                                      // basis) in the kinetic-rate promoting_indices vectors

  // change names
  const std::string basis_name = mgd.basis_species_name[basis_index_to_replace];
  const std::string eqm_name = mgd.eqm_species_name[eqm_index_to_insert];
  mgd.basis_species_name[basis_index_to_replace] = eqm_name;
  mgd.basis_species_index.erase(basis_name);
  mgd.basis_species_index[eqm_name] = basis_index_to_replace;
  mgd.eqm_species_name[eqm_index_to_insert] = basis_name;
  mgd.eqm_species_index.erase(eqm_name);
  mgd.eqm_species_index[basis_name] = eqm_index_to_insert;

  // flag indicating whether the redox_lhs is the equilibrium species that is being put into the
  // basis
  const bool redox_lhs_going_to_basis = (eqm_name == mgd.redox_lhs);
  mooseAssert(
      !redox_lhs_going_to_basis,
      "Should not be able to swap the redox LHS into the basis"); // At present, it is impossible to
                                                                  // swap redox_lhs into the basis,
                                                                  // since redox_lhs cannot be an
                                                                  // equilibrium species, because it
                                                                  // is explicitly excluded in
                                                                  // PertinentGeochemicalSystem when
                                                                  // building the secondary species.
  /*
  In past versions of the code, it was possible to swap the redox LHS into the basis.
  If you want to make it possible again then mgd needs to be modified as follows
  if (redox_lhs_going_to_basis)
  {
    // need to make the redox_lhs equal to the species that is being taken from the basis
    mgd.redox_lhs = basis_name;
    for (unsigned red = 0; red < num_redox; ++red)
    {
      const Real alpha = mgd.eqm_stoichiometry(
          eqm_index_to_insert, basis_index_to_replace); // must be nonzero due to valid swap
      const Real alpha_r = mgd.redox_stoichiometry(red, basis_index_to_replace);
      mooseAssert(alpha != alpha_r,
                  "Cannot swap equilibrium species "
                      << eqm_name << " with basis species " << basis_name
                      << " because a redox reaction would result in 0 <-> 1");
      const Real coef = 1.0 / (alpha - alpha_r);
      for (unsigned i = 0; i < num_cols; ++i)
        mgd.redox_stoichiometry(red, i) = coef * (mgd.redox_stoichiometry(red, i) -
                                                  mgd.eqm_stoichiometry(eqm_index_to_insert, i));
      mgd.redox_stoichiometry(red, basis_index_to_replace) = 0.0;
      for (unsigned t = 0; t < num_temperatures; ++t)
        mgd.redox_log10K(red, t) =
            coef * (mgd.redox_log10K(red, t) - mgd.eqm_log10K(eqm_index_to_insert, t));
    }
  }
  */

  // record that the swap has occurred
  if (mgd.swap_to_original_basis.n() == 0)
    mgd.swap_to_original_basis = _true_swap_matrix;
  else
    mgd.swap_to_original_basis.left_multiply(_true_swap_matrix);
  mgd.have_swapped_out_of_basis.push_back(basis_index_to_replace);
  mgd.have_swapped_into_basis.push_back(eqm_index_to_insert);

  // express stoichiometry in new basis
  for (unsigned i = 0; i < num_cols; ++i)
    mgd.eqm_stoichiometry(eqm_index_to_insert, i) = 0.0;
  mgd.eqm_stoichiometry(eqm_index_to_insert, basis_index_to_replace) =
      1.0; // 1 * replace_this <-> 1 * replace_this
  mgd.eqm_stoichiometry.right_multiply(_inv_swap_matrix);
  for (unsigned i = 0; i < num_rows; ++i)
    for (unsigned j = 0; j < num_cols; ++j)
      if (std::abs(mgd.eqm_stoichiometry(i, j)) < _stoi_tol)
        mgd.eqm_stoichiometry(i, j) = 0.0;

  // if the redox_lhs is not changed by the swap, alter the redox stoichiometry
  if (!redox_lhs_going_to_basis)
    mgd.redox_stoichiometry.right_multiply(_inv_swap_matrix);
  for (unsigned red = 0; red < num_redox; ++red)
    for (unsigned j = 0; j < num_cols; ++j)
      if (std::abs(mgd.redox_stoichiometry(red, j)) < _stoi_tol)
        mgd.redox_stoichiometry(red, j) = 0.0;

  // express kinetic stoichiometry in new basis
  mgd.kin_stoichiometry.right_multiply(_inv_swap_matrix);
  for (unsigned i = 0; i < kin_rows; ++i)
    for (unsigned j = 0; j < num_cols; ++j)
      if (std::abs(mgd.kin_stoichiometry(i, j)) < _stoi_tol)
        mgd.kin_stoichiometry(i, j) = 0.0;

  // alter equilibrium constants for each temperature point
  for (unsigned t = 0; t < num_temperatures; ++t)
  {
    const Real log10k_eqm_species = mgd.eqm_log10K(eqm_index_to_insert, t);
    mgd.eqm_log10K(eqm_index_to_insert, t) =
        0.0; // 1 * replace_this <-> 1 * replace_this with log10K = 0
    for (unsigned row = 0; row < num_rows; ++row)
      mgd.eqm_log10K(row, t) -=
          mgd.eqm_stoichiometry(row, basis_index_to_replace) * log10k_eqm_species;

    // similar for the redox equations
    if (!redox_lhs_going_to_basis)
      for (unsigned red = 0; red < num_redox; ++red)
        mgd.redox_log10K(red, t) -=
            mgd.redox_stoichiometry(red, basis_index_to_replace) * log10k_eqm_species;

    // similar for kinetic
    for (unsigned kin = 0; kin < kin_rows; ++kin)
      mgd.kin_log10K(kin, t) -=
          mgd.kin_stoichiometry(kin, basis_index_to_replace) * log10k_eqm_species;
  }

  // swap the "is mineral" information
  const bool basis_was_mineral = mgd.basis_species_mineral[basis_index_to_replace];
  mgd.basis_species_mineral[basis_index_to_replace] = mgd.eqm_species_mineral[eqm_index_to_insert];
  mgd.eqm_species_mineral[eqm_index_to_insert] = basis_was_mineral;

  // swap the "is gas" information
  const bool basis_was_gas = mgd.basis_species_gas[basis_index_to_replace];
  mgd.basis_species_gas[basis_index_to_replace] = mgd.eqm_species_gas[eqm_index_to_insert];
  mgd.eqm_species_gas[eqm_index_to_insert] = basis_was_gas;

  // swap the "is transported" information
  const bool basis_was_transported = mgd.basis_species_transported[basis_index_to_replace];
  mgd.basis_species_transported[basis_index_to_replace] =
      mgd.eqm_species_transported[eqm_index_to_insert];
  mgd.eqm_species_transported[eqm_index_to_insert] = basis_was_transported;

  // swap the "charge" information
  const Real basis_charge = mgd.basis_species_charge[basis_index_to_replace];
  mgd.basis_species_charge[basis_index_to_replace] = mgd.eqm_species_charge[eqm_index_to_insert];
  mgd.eqm_species_charge[eqm_index_to_insert] = basis_charge;

  // swap the "radius" information
  const Real basis_radius = mgd.basis_species_radius[basis_index_to_replace];
  mgd.basis_species_radius[basis_index_to_replace] = mgd.eqm_species_radius[eqm_index_to_insert];
  mgd.eqm_species_radius[eqm_index_to_insert] = basis_radius;

  // swap the "molecular weight" information
  const Real basis_molecular_weight = mgd.basis_species_molecular_weight[basis_index_to_replace];
  mgd.basis_species_molecular_weight[basis_index_to_replace] =
      mgd.eqm_species_molecular_weight[eqm_index_to_insert];
  mgd.eqm_species_molecular_weight[eqm_index_to_insert] = basis_molecular_weight;

  // swap the "molecular volume" information
  const Real basis_molecular_volume = mgd.basis_species_molecular_volume[basis_index_to_replace];
  mgd.basis_species_molecular_volume[basis_index_to_replace] =
      mgd.eqm_species_molecular_volume[eqm_index_to_insert];
  mgd.eqm_species_molecular_volume[eqm_index_to_insert] = basis_molecular_volume;

  // No need to swap surface_complexation_info or gas_chi because they are not
  // bound to basis or eqm species

  // swap promoting indices in the rates
  for (unsigned r = 0; r < num_rate; ++r)
  {
    const Real promoting_index_of_original_basis =
        mgd.kin_rate[r].promoting_indices[basis_index_to_replace];
    mgd.kin_rate[r].promoting_indices[basis_index_to_replace] =
        mgd.kin_rate[r].promoting_indices[pro_ind_eqm];
    mgd.kin_rate[r].promoting_indices[pro_ind_eqm] = promoting_index_of_original_basis;
    const Real promoting_monod_index_of_original_basis =
        mgd.kin_rate[r].promoting_monod_indices[basis_index_to_replace];
    mgd.kin_rate[r].promoting_monod_indices[basis_index_to_replace] =
        mgd.kin_rate[r].promoting_monod_indices[pro_ind_eqm];
    mgd.kin_rate[r].promoting_monod_indices[pro_ind_eqm] = promoting_monod_index_of_original_basis;
    const Real promoting_half_saturation_of_original_basis =
        mgd.kin_rate[r].promoting_half_saturation[basis_index_to_replace];
    mgd.kin_rate[r].promoting_half_saturation[basis_index_to_replace] =
        mgd.kin_rate[r].promoting_half_saturation[pro_ind_eqm];
    mgd.kin_rate[r].promoting_half_saturation[pro_ind_eqm] =
        promoting_half_saturation_of_original_basis;
  }

  // swap progeny, if needed
  for (unsigned r = 0; r < num_rate; ++r)
  {
    if (mgd.kin_rate[r].progeny_index == pro_ind_eqm)
      mgd.kin_rate[r].progeny_index = basis_index_to_replace;
    else if (mgd.kin_rate[r].progeny_index == basis_index_to_replace)
      mgd.kin_rate[r].progeny_index = pro_ind_eqm;
  }
}

void
GeochemistrySpeciesSwapper::alterBulkComposition(unsigned basis_size,
                                                 DenseVector<Real> & bulk_composition) const
{
  if (bulk_composition.size() != basis_size)
    mooseError("GeochemistrySpeciesSwapper: bulk_composition has size ",
               bulk_composition.size(),
               " which differs from the basis size");
  DenseVector<Real> result;
  _inv_swap_matrix.vector_mult_transpose(result, bulk_composition);
  bulk_composition = result;
}

bool
GeochemistrySpeciesSwapper::findBestEqmSwap(unsigned basis_ind,
                                            const ModelGeochemicalDatabase & mgd,
                                            const std::vector<Real> & eqm_molality,
                                            bool minerals_allowed,
                                            bool gas_allowed,
                                            bool sorption_allowed,
                                            unsigned & best_eqm_species) const
{
  const unsigned num_eqm = mgd.eqm_species_name.size();
  if (eqm_molality.size() != num_eqm)
    mooseError("Size of eqm_molality is ",
               eqm_molality.size(),
               " which is not equal to the number of equilibrium species ",
               num_eqm);
  if (basis_ind >= mgd.basis_species_name.size())
    mooseError("basis index ", basis_ind, " must be less than ", mgd.basis_species_name.size());
  best_eqm_species = 0;
  bool legitimate_swap_found = false;
  Real best_stoi = 0.0;
  for (unsigned j = 0; j < num_eqm; ++j)
  {
    if (mgd.eqm_stoichiometry(j, basis_ind) == 0.0)
      continue;
    if (!minerals_allowed && mgd.eqm_species_mineral[j])
      continue;
    if (!gas_allowed && mgd.eqm_species_gas[j])
      continue;
    if (!sorption_allowed && mgd.surface_sorption_related[j])
      continue;
    const Real stoi = std::abs(mgd.eqm_stoichiometry(j, basis_ind)) * eqm_molality[j];
    if (stoi >= best_stoi)
    {
      best_stoi = stoi;
      best_eqm_species = j;
      legitimate_swap_found = true;
    }
  }
  return legitimate_swap_found;
}
