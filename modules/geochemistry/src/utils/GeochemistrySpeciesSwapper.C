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

  // In order to invert _swap_matrix, perform the SVD: A = U * D * VT
  try
  {
    _swap_matrix.svd(_swap_sigma, _swap_U, _swap_VT);
  }
  catch (...)
  {
    mooseException("Matrix is not invertible, which signals an invalid basis swap");
  }
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

  // change names
  const std::string basis_name = mgd.basis_species_name[basis_index_to_replace];
  const std::string eqm_name = mgd.eqm_species_name[eqm_index_to_insert];
  mgd.basis_species_name[basis_index_to_replace] = eqm_name;
  mgd.basis_species_index.erase(basis_name);
  mgd.basis_species_index[eqm_name] = basis_index_to_replace;
  mgd.eqm_species_name[eqm_index_to_insert] = basis_name;
  mgd.eqm_species_index.erase(eqm_name);
  mgd.eqm_species_index[basis_name] = eqm_index_to_insert;

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

  // alter equilibrium constants for each temperature point
  const unsigned num_temperatures = mgd.eqm_log10K.n();
  for (unsigned t = 0; t < num_temperatures; ++t)
  {
    const Real log10k_eqm_species = mgd.eqm_log10K(eqm_index_to_insert, t);
    mgd.eqm_log10K(eqm_index_to_insert, t) =
        0.0; // 1 * replace_this <-> 1 * replace_this with log10K = 0
    for (unsigned row = 0; row < num_rows; ++row)
      mgd.eqm_log10K(row, t) -=
          mgd.eqm_stoichiometry(row, basis_index_to_replace) * log10k_eqm_species;
  }

  // swap the "is mineral" information
  const bool basis_was_mineral = mgd.basis_species_mineral[basis_index_to_replace];
  mgd.basis_species_mineral[basis_index_to_replace] = mgd.eqm_species_mineral[eqm_index_to_insert];
  mgd.eqm_species_mineral[eqm_index_to_insert] = basis_was_mineral;

  // swap the "is gas" information
  const bool basis_was_gas = mgd.basis_species_gas[basis_index_to_replace];
  mgd.basis_species_gas[basis_index_to_replace] = mgd.eqm_species_gas[eqm_index_to_insert];
  mgd.eqm_species_gas[eqm_index_to_insert] = basis_was_gas;

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

  // express kinetic stoichiometry in new basis
  mgd.kin_stoichiometry.right_multiply(_inv_swap_matrix);
  const unsigned kin_rows = mgd.kin_stoichiometry.m();
  for (unsigned i = 0; i < kin_rows; ++i)
    for (unsigned j = 0; j < num_cols; ++j)
      if (std::abs(mgd.kin_stoichiometry(i, j)) < _stoi_tol)
        mgd.kin_stoichiometry(i, j) = 0.0;
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
