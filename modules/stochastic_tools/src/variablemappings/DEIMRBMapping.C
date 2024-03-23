//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DEIMRBMapping.h"
#include <cstddef>
#include <slepcsvd.h>
#include "InputParameters.h"

#include "MooseError.h"
#include "MooseTypes.h"
#include "QDEIM.h"
#include "ReporterInterface.h"
#include "VariableMappingBase.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include "libmesh/int_range.h"
#include <petscdmda.h>
#include <vector>

registerMooseObject("StochasticToolsApp", DEIMRBMapping);

InputParameters
DEIMRBMapping::validParams()
{
  InputParameters params = VariableMappingBase::validParams();
  params.addClassDescription("Class for integrating Discrete Empirical Interpolation Method (DEIM) "
                             "with Proper Orthogonal Decomposition (POD)-based Reduced Basis (RB) "
                             "mapping. This class specializes in mapping between full-order model "
                             "spaces and reduced-order model spaces.");
  params.addParam<UserObjectName>(
      "solution_storage", "The name of the storage reporter where the snapshots are located.");
  params.addParam<std::vector<dof_id_type>>(
      "num_modes_to_compute",
      "The number of modes that this object should compute. "
      "Modes with 0 eigenvalues are filtered out, so the real number of modes "
      "might be lower than this. This is also used for setting the "
      "subspace sizes for distributed singular value solves. By default, the subspace used for the "
      "SVD is twice as big as the number of requested vectors. For more information see the SLEPc "
      "manual. If not specified, only one mode is computed per variable.");
  params.addParam<std::vector<Real>>(
      "energy_threshold",
      std::vector<Real>(),
      "The energy threshold for the automatic truncation of the number of modes. In general, the "
      "lower this number is the more information is retained about the system by keeping more POD "
      "modes.");
  params.addParam<std::string>("extra_slepc_options",
                               "",
                               "Additional options for the singular/eigenvalue solvers in SLEPc.");
  params.addParam<ReporterName>("jac_index_name",
                                "Name of the reporter that holds the jacobian indicies.");
  return params;
}

DEIMRBMapping::DEIMRBMapping(const InputParameters & parameters)
  : VariableMappingBase(parameters),
    UserObjectInterface(this),
    ReporterInterface(this),
    _num_modes(isParamValid("num_modes_to_compute")
                   ? getParam<std::vector<dof_id_type>>("num_modes_to_compute")
                   : std::vector<dof_id_type>(_variable_names.size(), 1)),
    _energy_threshold(getParam<std::vector<Real>>("energy_threshold")),
    _sol_basis(declareModelData<std::vector<DenseVector<Real>>>("solution_basis")),
    _residual_selection_inds(
        declareModelData<std::vector<dof_id_type>>("residual_selection_indices")),
    _jacobian_selection_inds(
        declareModelData<std::vector<dof_id_type>>("jacobian_selection_indices")),
    _jac_matrix_selection_inds(declareModelData<std::vector<std::pair<dof_id_type, dof_id_type>>>(
        "jacobian_matrix_selection_indices")),
    _reduced_residual(declareModelData<std::vector<DenseVector<Real>>>("reduced_residual_basis")),
    _reduced_jacobian(declareModelData<std::vector<DenseMatrix<Real>>>("reduced_jacobian_basis")),
    _residual_selection_matrix(declareModelData<DenseMatrix<Real>>("residual_selection_matrix")),
    _jacobian_selection_matrix(declareModelData<DenseMatrix<Real>>(("jacobian_selection_matrix"))),
    _parallel_storage(isParamValid("solution_storage")
                          ? &getUserObject<ParallelSolutionStorage>("solution_storage")
                          : nullptr),
    _pod(StochasticTools::POD(
        _parallel_storage, getParam<std::string>("extra_slepc_options"), _communicator)),
    _jacobian_indices(
        isParamValid("jac_index_name")
            ? &getReporterValue<std::vector<std::pair<dof_id_type, dof_id_type>>>("jac_index_name")
            : nullptr)
{
  if (!isParamValid("filename"))
  {
    if (_num_modes.size() != _variable_names.size())
      paramError("num_modes_to_compute",
                 "The number of modes should be defined for each variable!");

    for (const auto & mode : _num_modes)
      if (!mode)
        paramError("num_modes_to_compute",
                   "The number of modes should always be a positive integer!");

    if (_energy_threshold.size())
    {
      if (_energy_threshold.size() != _variable_names.size())
        paramError("energy_threshold",
                   "The energy thresholds should be defined for each variable!");

      for (const auto & threshold : _energy_threshold)
        if (threshold < 0 || threshold >= 1)
          paramError("energy_threshold",
                     "The energy thresholds should always be in the [0,1) range!");
    }

#if PETSC_VERSION_LESS_THAN(3, 14, 0)
    mooseError("DEIMRBMapping is not supported with PETSc version below 3.14!");
#else
    // This mapping will only map these three "variables"
    std::vector<VariableName> rom_components({"solution", "residual", "jacobian"});
    // Make a copy of _variable_names to sort. Don't want to change the original vector.
    std::vector<VariableName> variable_names_sorted = _variable_names;

    // Sort both vectors
    std::sort(variable_names_sorted.begin(), variable_names_sorted.end());
    std::sort(rom_components.begin(), rom_components.end());

    if (rom_components != variable_names_sorted)
      paramError("variables",
                 "The variable name should only be in the set {solution, residual, jacobian}.");

    for (const auto & vname : _variable_names)
    {
      _mapping_ready_to_use.emplace(vname, false);
    }
#endif
  }
}
void
DEIMRBMapping::buildAllMappings()
{

  if (!_jacobian_indices)
    mooseError("Jacobian indices reporter was not provided.");

  // Build the basis for each component
  buildComponentBases();

  // The rest of these are currently only done in serial. No need for all ranks to
  // do the work.
  if (_communicator.rank() == 0)
  {
    // Construct jacobian matrix basis from vector basis
    constructReducedBasis();
    // Compute DEIM on the on residual and jacobian to get the selection
    // indicies
    computeDEIMSelectionIndices();
    //   store the selection matrices
    storeSelectionMatrices();

    // Now we need to fix the selection indicies. They are currently the selection
    // indices for the dense nonzero data. Need to convert back to sparse.
    computeSparseMatrixConversion();

    // All mappings are now ready to use
    for (auto & comp : _mapping_ready_to_use)
      comp.second = true;
  }
}

void
DEIMRBMapping::map(const VariableName & /*vname*/,
                   const DenseVector<Real> & /*full_order_vector*/,
                   std::vector<Real> & /*reduced_order_vector*/) const
{
  mooseError("DEIMRBMapping does not provide this functionalty.");
}

void
DEIMRBMapping::map(const VariableName & /*vname*/,
                   const unsigned int /*global_sample_i*/,
                   std::vector<Real> & /*reduced_order_vector*/) const
{
  mooseError("DEIMRBMapping does not provide this functionalty.");
}

void
DEIMRBMapping::inverse_map(const VariableName & vname,
                           const std::vector<Real> & reduced_order_vector,
                           DenseVector<Real> & full_order_vector) const
{
  if (vname != "solution")
    mooseError("We are only inverse mapping the solution currently");

  if (reduced_order_vector.size() != _sol_basis.size())
    mooseError("The number of supplied reduced-order coefficients (",
               reduced_order_vector.size(),
               ") is not the same as the number of basis functions (",
               _sol_basis.size(),
               ") for the solution!");

  // This zeros the DenseVector too
  full_order_vector.resize(_sol_basis[0].size());

  for (auto base_i : index_range(reduced_order_vector))
    for (unsigned int dof_i = 0; dof_i < _sol_basis[base_i].size(); ++dof_i)
      full_order_vector(dof_i) += reduced_order_vector[base_i] * _sol_basis[base_i](dof_i);
}

void
DEIMRBMapping::buildComponentBases()
{
  _sol_basis = computeBasis("solution");

  _res_basis = computeBasis("residual");

  _jac_basis = computeBasis("jacobian");

  for (const auto & basis : {_sol_basis, _res_basis, _jac_basis})
    if (basis.size() == 0)
      mooseError("Basis is empty. This will not work for DEIMRBMapping.");
}

void
DEIMRBMapping::constructReducedBasis()
{
  // Need to generate a basis matrix for multiplication
  // _sol_basis is a vector of columns of the matrix

  RealEigenMatrix sol_basis(_sol_basis[0].size(), _sol_basis.size());
  for (MooseIndex(_sol_basis) col = 0; col < _sol_basis.size(); ++col)
  {
    const auto & column_vector = _sol_basis[col];
    for (MooseIndex(column_vector) row = 0; row < column_vector.size(); ++row)
    {
      sol_basis(row, col) = column_vector(row);
    }
  }

  RealEigenMatrix sol_basis_transpose = sol_basis.transpose();

  // Calculate the dimensions of the square matrix
  const auto matrix_size = _sol_basis[0].size();

  for (const auto & basis : _jac_basis)
  {
    // Create a new matrix for each basis
    Eigen::SparseMatrix<Real> matrix(matrix_size, matrix_size);

    // Prepare to fill the matrix using triplets
    std::vector<Eigen::Triplet<Real>> triplets;
    triplets.reserve(_jacobian_indices->size()); // Reserve space to avoid reallocations

    for (const auto i : make_range(_jacobian_indices->size()))
    {
      auto index_pair = _jacobian_indices->at(i);
      Real value = basis(i); // Corresponding value for this entry

      auto row = index_pair.first;  //  division to get the row
      auto col = index_pair.second; // Modulus to get the column

      // Add a triplet for the current non-zero entry
      triplets.emplace_back(row, col, value);
    }

    // Bulk insert the triplets into the matrix
    matrix.setFromTriplets(triplets.begin(), triplets.end());

    // Optionally, make the matrix compressed to optimize operations
    // matrix.makeCompressed();

    // Compute reduced matrix: V^T * J_i * V
    RealEigenMatrix reduced_matrix = sol_basis_transpose * matrix * sol_basis;

    // Fill the real dense matrix with the values from the reduced matrix
    RealDenseMatrix real_dense_mat(reduced_matrix.rows(), reduced_matrix.cols());
    for (const auto i : make_range(reduced_matrix.rows()))
    {
      for (const auto j : make_range(reduced_matrix.cols()))
      {
        real_dense_mat(i, j) = reduced_matrix(i, j);
      }
    }

    // Add the matrix to the vector of matrices
    _reduced_jacobian.push_back(real_dense_mat);
  }

  RealDenseMatrix real_dense_solution_basis(sol_basis.rows(), sol_basis.cols());
  for (const auto i : make_range(sol_basis.rows()))
  {
    for (const auto j : make_range(sol_basis.cols()))
    {
      real_dense_solution_basis(i, j) = sol_basis(i, j);
    }
  }

  // * Residual Basis;
  for (const auto & basis : _res_basis)
  {
    DenseVector<Real> red_resid;
    real_dense_solution_basis.vector_mult_transpose(red_resid, basis);

    _reduced_residual.push_back(red_resid);
  }
}

void
DEIMRBMapping::computeDEIMSelectionIndices()
{

  // QDEIM for Jacobian
  QDEIM jac_qd(_jac_basis);
  jac_qd.computeSelection(_jacobian_selection_inds);

  // QDEIM for residual
  QDEIM res_qd(_res_basis);
  res_qd.computeSelection(_residual_selection_inds);
}

void
DEIMRBMapping::storeSelectionMatrices()
{

  // Save jacobian selection matrix
  const auto num_jac_sel = _jacobian_selection_inds.size();
  _jacobian_selection_matrix.resize(num_jac_sel, num_jac_sel);
  for (MooseIndex(num_jac_sel) i = 0; i < num_jac_sel; ++i)
  {
    dof_id_type selected_row = _jacobian_selection_inds[i]; // Get the index of the selected row
    for (MooseIndex(_jac_basis) col = 0; col < _jac_basis.size(); ++col)
      // Assign the value from the selected row of each column in _jac_basis
      _jacobian_selection_matrix(i, col) = _jac_basis[col](selected_row);
  }

  // save residual selection matrix
  const auto num_res_sel = _residual_selection_inds.size();
  _residual_selection_matrix.resize(num_res_sel,
                                    _res_basis.size()); // Resize to the correct dimensions

  for (MooseIndex(num_res_sel) selection = 0; selection < num_res_sel; ++selection)
  {
    dof_id_type selected_row =
        _residual_selection_inds[selection]; // Get the index of the selected row
    for (MooseIndex(_res_basis) col = 0; col < _res_basis.size(); ++col)
    {
      // Assign the value from the selected row of each column in _res_basis
      _residual_selection_matrix(selection, col) = _res_basis[col](selected_row);
    }
  }
}

std::vector<DenseVector<Real>>
DEIMRBMapping::computeBasis(const VariableName & vname)
{

  std::vector<DenseVector<Real>> basis;
  // TODO: These are throwaway vectors that are not needed. Change POD class so
  // it doesn't need it
  std::vector<DenseVector<Real>> right_basis;
  std::vector<Real> singular_values;

  // First find and create the basis for the solution
  auto it = std::find(_variable_names.begin(), _variable_names.end(), vname);
  unsigned int index = std::distance(_variable_names.begin(), it);
  // Find the number of modes that we would want to compute
  std::size_t num_modes_compute = (std::size_t)_num_modes[index];
  const Real threshold = (_energy_threshold.empty() ? 0.0 : _energy_threshold[index]) +
                         std::numeric_limits<Real>::epsilon();

  _pod.computePOD(vname, basis, right_basis, singular_values, num_modes_compute, threshold);
  //   Again we don't need these so clearing them right away

  return basis;
}

std::vector<std::pair<dof_id_type, dof_id_type>>
DEIMRBMapping::convertMatrixSelectionInds(dof_id_type num_dofs,
                                          const std::vector<dof_id_type> & indices)
{
  std::vector<std::pair<dof_id_type, dof_id_type>> dof_pairs;

  for (const auto & index : indices)
  {
    dof_id_type row = index / num_dofs; // Determine the row in the Jacobian matrix
    dof_id_type col = index % num_dofs; // Determine the column in the Jacobian matrix

    // Add the row and column as a pair
    dof_pairs.emplace_back(row, col);
  }

  return dof_pairs;
}

RealDenseMatrix
DEIMRBMapping::compute_reduced_jac(const std::vector<Real> & red_jac_values)
{
  // We assume the reduced values come in the same order as
  // _jacobian_selection_inds
  DenseVector<Real> jac_theta;

  _jacobian_selection_matrix.lu_solve(red_jac_values, jac_theta);

  // Now we can construct the reduced jacobian
  RealDenseMatrix reduce_jac(_reduced_jacobian[0].m(), _reduced_jacobian[0].n());

  for (MooseIndex(jac_theta) i = 0; i < jac_theta.size(); i++)
    reduce_jac.add(jac_theta(i), _reduced_jacobian[i]);

  return reduce_jac;
}

DenseVector<Real>
DEIMRBMapping::compute_reduced_res(const std::vector<Real> & red_res_values)
{
  // We assume the reduced values come in the same order as
  // _residual_selection_inds
  DenseVector<Real> res_theta;

  _residual_selection_matrix.lu_solve(red_res_values, res_theta);

  // Now we can construct the reduced residual
  DenseVector<Real> reduce_res(_reduced_residual[0].size());

  for (MooseIndex(res_theta) i = 0; i < res_theta.size(); i++)
    reduce_res.add(res_theta(i), _reduced_residual[i]);

  return reduce_res;
}

void
DEIMRBMapping::computeSparseMatrixConversion()
{

  _jac_matrix_selection_inds.clear();

  // Convert from the selection indices
  for (auto & sel_index : _jacobian_selection_inds)
    _jac_matrix_selection_inds.push_back(_jacobian_indices->at(sel_index));
}
