//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODMapping.h"
#include <slepcsvd.h>
#include "libmesh/parallel_object.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/dense_vector.h"
#include "MooseTypes.h"
#include <petscdmda.h>

registerMooseObject("StochasticToolsApp", PODMapping);

InputParameters
PODMapping::validParams()
{
  InputParameters params = VariableMappingBase::validParams();
  params.addClassDescription("Class which provides a Proper Orthogonal Decomposition-based mapping "
                             "between full-order and reduced-order spaces.");
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
  return params;
}

PODMapping::PODMapping(const InputParameters & parameters)
  : VariableMappingBase(parameters),
    UserObjectInterface(this),
    _num_modes(isParamValid("num_modes_to_compute")
                   ? getParam<std::vector<dof_id_type>>("num_modes_to_compute")
                   : std::vector<dof_id_type>(_variable_names.size(), 1)),
    _energy_threshold(getParam<std::vector<Real>>("energy_threshold")),
    _left_basis_functions(declareModelData<std::map<VariableName, std::vector<DenseVector<Real>>>>(
        "left_basis_functions")),
    _right_basis_functions(declareModelData<std::map<VariableName, std::vector<DenseVector<Real>>>>(
        "right_basis_functions")),
    _singular_values(
        declareModelData<std::map<VariableName, std::vector<Real>>>("singular_values")),
    _extra_slepc_options(getParam<std::string>("extra_slepc_options")),
    _parallel_storage(isParamValid("solution_storage")
                          ? &getUserObject<ParallelSolutionStorage>("solution_storage")
                          : nullptr)
{
  if (!isParamValid("filename"))
  {
    if (_num_modes.size() != _variable_names.size())
      paramError("num_modes", "The number of modes should be defined for each variable!");

    for (const auto & mode : _num_modes)
      if (!mode)
        paramError("num_modes", "The number of modes should always be a positive integer!");

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
    mooseError("PODMapping is not supported with PETSc version below 3.14!");
#else
    for (const auto & vname : _variable_names)
    {
      _singular_values.emplace(vname, std::vector<Real>());
      _left_basis_functions.emplace(vname, std::vector<DenseVector<Real>>());
      _right_basis_functions.emplace(vname, std::vector<DenseVector<Real>>());
      _svds.try_emplace(vname);
      SVDCreate(_communicator.get(), &_svds[vname]);
      _mapping_ready_to_use.emplace(vname, false);
    }
#endif
  }
}

PODMapping::~PODMapping()
{
#if !PETSC_VERSION_LESS_THAN(3, 14, 0)
  for (const auto & vname : _variable_names)
    SVDDestroy(&_svds[vname]);
#endif
}

dof_id_type
PODMapping::determineNumberOfModes(const VariableName & vname,
                                   const std::vector<Real> & converged_evs)
{
  dof_id_type num_modes = 0;

  auto it = std::find(_variable_names.begin(), _variable_names.end(), vname);
  mooseAssert(it != _variable_names.end(), "Variable " + vname + " is not in PODMapping!");

  unsigned int var_i = std::distance(_variable_names.begin(), it);

  // We either use the number of modes defined by the user or the maximum number of converged
  // modes. We don't want to use modes which are unconverged.
  std::size_t num_requested_modes = std::min((std::size_t)_num_modes[var_i], converged_evs.size());

  // Grab a cumulative sum of singular value squared
  std::vector<Real> ev_sum(converged_evs.begin(), converged_evs.begin() + num_requested_modes);
  std::partial_sum(ev_sum.cbegin(),
                   ev_sum.cend(),
                   ev_sum.begin(),
                   [](Real sum, Real ev) { return sum + ev * ev; });

  // Find the first element that satisfies the threshold
  const Real threshold = (_energy_threshold.empty() ? 0.0 : _energy_threshold[var_i]) +
                         std::numeric_limits<Real>::epsilon();
  for (num_modes = 0; num_modes < ev_sum.size(); ++num_modes)
    if (ev_sum[num_modes] / ev_sum.back() > 1 - threshold)
      break;

  return num_modes + 1;
}

void
PODMapping::buildMapping(const VariableName &
#if !PETSC_VERSION_LESS_THAN(3, 14, 0)
                             vname
#endif
)
{
#if !PETSC_VERSION_LESS_THAN(3, 14, 0)
  if (!_parallel_storage)
    paramError(
        "solution_storage",
        "The parallel storage reporter is not supplied! We cannot build a mapping without data!");

  if (_mapping_ready_to_use.find(vname) != _mapping_ready_to_use.end() &&
      !_mapping_ready_to_use[vname])
  {
    auto it = std::find(_variable_names.begin(), _variable_names.end(), vname);
    mooseAssert(it != _variable_names.end(), "Variable " + vname + " is not in PODMapping!");
    unsigned int var_i = std::distance(_variable_names.begin(), it);

    // Define the petsc matrix which needs and SVD, we will populate it using the snapshots
    Mat mat;

    // We make sure every rank knows how many global and local samples we have and how long the
    // snapshots are. At this point we assume that the snapshots are the same size so we don't
    // need to map them to a reference domain.
    dof_id_type local_rows = 0;
    dof_id_type snapshot_size = 0;
    dof_id_type global_rows = 0;
    if (_parallel_storage->getStorage().size())
    {
      local_rows = _parallel_storage->getStorage(vname).size();
      global_rows = local_rows;
      if (_parallel_storage->getStorage(vname).size())
        snapshot_size = _parallel_storage->getStorage(vname).begin()->second[0].size();
    }

    comm().sum(global_rows);
    comm().max(snapshot_size);

    // The Lanczos method is preferred for symmetric semi positive definite matrices
    // but it only works with sparse matrices at the moment (dense matrix leaks memory).
    // So we create a sparse matrix with the distribution given in ParallelSolutionStorage.
    // TODO: Figure out a way to use a dense matrix without leaking memory, that would
    // avoid the overhead of using a sparse format for a dense matrix
    PetscErrorCode ierr = MatCreateAIJ(_communicator.get(),
                                       local_rows,
                                       snapshot_size,
                                       global_rows,
                                       snapshot_size,
                                       processor_id() == 0 ? snapshot_size : 0,
                                       NULL,
                                       processor_id() == 0 ? 0 : snapshot_size,
                                       NULL,
                                       &mat);
    LIBMESH_CHKERR(ierr);

    // Check where the local rows begin in the matrix, we use these to convert from local to
    // global indices
    dof_id_type local_beg = 0;
    dof_id_type local_end = 0;
    MatGetOwnershipRange(mat, numeric_petsc_cast(&local_beg), numeric_petsc_cast(&local_end));

    unsigned int counter = 0;
    if (local_rows)
      for (const auto & row : _parallel_storage->getStorage(vname))
      {
        std::vector<PetscInt> rows(snapshot_size, (counter++) + local_beg);

        // Fill the column indices with 0,1,...,snapshot_size-1
        std::vector<PetscInt> columns(snapshot_size);
        std::iota(std::begin(columns), std::end(columns), 0);

        // Set the rows in the "sparse" matrix
        MatSetValues(mat,
                     1,
                     rows.data(),
                     snapshot_size,
                     columns.data(),
                     row.second[0].get_values().data(),
                     INSERT_VALUES);
      }

    // Assemble the matrix
    MatAssemblyBegin(mat, MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(mat, MAT_FINAL_ASSEMBLY);

    // Now we set the operators for our SVD objects
    SVDSetOperators(_svds[vname], mat, NULL);

    // We want the Lanczos method, might give the choice to the user
    // at some point
    SVDSetType(_svds[vname], SVDTRLANCZOS);
    ierr = PetscOptionsInsertString(NULL, _extra_slepc_options.c_str());
    LIBMESH_CHKERR(ierr);

    // Set the subspace size for the Lanczos method, we take twice as many
    // basis vectors as the requested number of POD modes. This guarantees in most of the case the
    // convergence of the singular triplets.
    SVDSetFromOptions(_svds[vname]);
    SVDSetDimensions(_svds[vname],
                     _num_modes[var_i],
                     std::min(2 * _num_modes[var_i], global_rows),
                     std::min(2 * _num_modes[var_i], global_rows));

    // Compute the singular value triplets
    ierr = SVDSolve(_svds[vname]);
    LIBMESH_CHKERR(ierr);

    // Check how many singular triplets converged
    PetscInt nconv;
    ierr = SVDGetConverged(_svds[vname], &nconv);
    LIBMESH_CHKERR(ierr);

    // We start extracting the basis functions and the singular values. The left singular
    // vectors are supposed to be all on the root processor
    // PetscReal sigma;
    PetscVector<Real> u(_communicator);
    u.init(snapshot_size);

    PetscVector<Real> v(_communicator);
    v.init(global_rows, local_rows, false, PARALLEL);

    _left_basis_functions[vname].clear();
    _right_basis_functions[vname].clear();
    _singular_values[vname].clear();

    auto & emplaced_singular_values =
        _singular_values.emplace(vname, std::vector<Real>()).first->second;
    emplaced_singular_values.resize(nconv);
    // Fetch the singular value triplet and immediately save the singular value
    for (PetscInt j = 0; j < nconv; ++j)
    {
      ierr = SVDGetSingularTriplet(_svds[vname], j, &emplaced_singular_values[j], NULL, NULL);
      LIBMESH_CHKERR(ierr);
    }
    // Determine how many modes we need
    unsigned int num_requested_modes = determineNumberOfModes(vname, emplaced_singular_values);
    // Only save the basis functions which are needed. We serialize the modes
    // on every processor so all of them have access to every mode.
    _left_basis_functions[vname].resize(num_requested_modes);
    _right_basis_functions[vname].resize(num_requested_modes);
    for (PetscInt j = 0; j < cast_int<PetscInt>(num_requested_modes); ++j)
    {
      SVDGetSingularTriplet(_svds[vname], j, NULL, v.vec(), u.vec());
      u.localize(_left_basis_functions[vname][j].get_values());
      v.localize(_right_basis_functions[vname][j].get_values());
    }

    _mapping_ready_to_use[vname] = true;

    MatDestroy(&mat);
    SVDDestroy(&_svds[vname]);
  }

#endif
}

void
PODMapping::map(const VariableName & vname,
                const unsigned int global_sample_i,
                std::vector<Real> & reduced_order_vector) const
{
  mooseAssert(_parallel_storage, "We need the parallel solution storage for this operation.");
  mooseAssert(_left_basis_functions.find(vname) != _left_basis_functions.end(),
              "The bases for the requested variable are not available!");
  checkIfReadyToUse(vname);

  // This takes the 0th snapshot because we only support steady-state simulations
  // at the moment.
  const auto & snapshot = _parallel_storage->getGlobalSample(global_sample_i, vname)[0];

  const auto & bases = _left_basis_functions[vname];

  reduced_order_vector.clear();
  reduced_order_vector.resize(bases.size());
  for (auto base_i : index_range(bases))
    reduced_order_vector[base_i] = bases[base_i].dot(snapshot);
}

void
PODMapping::map(const VariableName & vname,
                const DenseVector<Real> & full_order_vector,
                std::vector<Real> & reduced_order_vector) const
{
  checkIfReadyToUse(vname);

  mooseAssert(_left_basis_functions.find(vname) != _left_basis_functions.end(),
              "The bases for the requested variable are not available!");

  const auto & bases = _left_basis_functions[vname];

  reduced_order_vector.clear();
  reduced_order_vector.resize(bases.size());
  for (auto base_i : index_range(bases))
    reduced_order_vector[base_i] = bases[base_i].dot(full_order_vector);
}

void
PODMapping::inverse_map(const VariableName & vname,
                        const std::vector<Real> & reduced_order_vector,
                        DenseVector<Real> & full_order_vector) const
{
  mooseAssert(std::find(_variable_names.begin(), _variable_names.end(), vname) !=
                  _variable_names.end(),
              "Variable " + vname + " is not in PODMapping!");

  checkIfReadyToUse(vname);

  if (reduced_order_vector.size() != _left_basis_functions[vname].size())
    mooseError("The number of supplied reduced-order coefficients (",
               reduced_order_vector.size(),
               ") is not the same as the number of basis functions (",
               _left_basis_functions[vname].size(),
               ") for variable ",
               vname,
               "!");
  // This zeros the DenseVector too
  full_order_vector.resize(_left_basis_functions[vname][0].size());

  for (auto base_i : index_range(reduced_order_vector))
    for (unsigned int dof_i = 0; dof_i < _left_basis_functions[vname][base_i].size(); ++dof_i)
      full_order_vector(dof_i) +=
          reduced_order_vector[base_i] * _left_basis_functions[vname][base_i](dof_i);
}

const DenseVector<Real> &
PODMapping::leftBasisFunction(const VariableName & vname, const unsigned int base_i)
{
  mooseAssert(std::find(_variable_names.begin(), _variable_names.end(), vname) !=
                  _variable_names.end(),
              "Variable " + vname + " is not in PODMapping!");

  checkIfReadyToUse(vname);

  mooseAssert(base_i < _left_basis_functions[vname].size(),
              "The POD for " + vname + " only has " +
                  std::to_string(_left_basis_functions[vname].size()) + " left modes!");

  return _left_basis_functions[vname][base_i];
}

const DenseVector<Real> &
PODMapping::rightBasisFunction(const VariableName & vname, const unsigned int base_i)
{
  mooseAssert(std::find(_variable_names.begin(), _variable_names.end(), vname) !=
                  _variable_names.end(),
              "Variable " + vname + " is not in PODMapping!");

  checkIfReadyToUse(vname);

  mooseAssert(base_i < _right_basis_functions[vname].size(),
              "The POD for " + vname + " only has " +
                  std::to_string(_right_basis_functions[vname].size()) + " right modes!");

  return _right_basis_functions[vname][base_i];
}

const std::vector<DenseVector<Real>> &
PODMapping::leftBasis(const VariableName & vname)
{
  checkIfReadyToUse(vname);
  if (_left_basis_functions.find(vname) == _left_basis_functions.end())
    mooseError("We are trying to access container for variable '",
               vname,
               "' but we don't have it in the POD mapping!");
  return _left_basis_functions[vname];
}

const std::vector<DenseVector<Real>> &
PODMapping::rightBasis(const VariableName & vname)
{
  checkIfReadyToUse(vname);
  if (_right_basis_functions.find(vname) == _right_basis_functions.end())
    mooseError("We are trying to access container for variable '",
               vname,
               "' but we don't have it in the POD mapping!");
  return _right_basis_functions[vname];
}

const std::vector<Real> &
PODMapping::singularValues(const VariableName & vname)
{
  checkIfReadyToUse(vname);
  if (_singular_values.find(vname) == _singular_values.end())
    mooseError("We are trying to access container for variable '",
               vname,
               "' but we don't have it in the POD mapping!");
  return _singular_values[vname];
}
