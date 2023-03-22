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
#include "MooseTypes.h"
#include <petscdmda.h>

registerMooseObject("StochasticToolsApp", PODMapping);

InputParameters
PODMapping::validParams()
{
  InputParameters params = MappingBase::validParams();
  params.addParam<UserObjectName>(
      "solution_storage", "The name of the storage reporter where the snapshots are located.");
  params.addRequiredParam<std::vector<VariableName>>(
      "variables", "The names of the variables which need a reduced basis.");
  params.addParam<std::vector<unsigned int>>(
      "num_modes",
      "The number of modes requested for each variable. Modes with 0 eigenvalues are filtered out, "
      "so the real number of modes might be lower than this.");
  params.addParam<std::vector<Real>>(
      "energy_threshold",
      "The energy threshold for the automatic selection (truncation) of the number of modes.");
  params.addParam<std::string>("extra_slepc_options",
                               "",
                               "Additional options for the singular/eigenvalue solvers in SLEPc.");
  return params;
}

PODMapping::PODMapping(const InputParameters & parameters)
  : MappingBase(parameters),
    _variable_names(getParam<std::vector<VariableName>>("variables")),
    _num_modes(isParamValid("num_modes") ? getParam<std::vector<unsigned int>>("num_modes")
                                         : std::vector<unsigned int>()),
    _energy_threshold(isParamValid("energy_threshold")
                          ? getParam<std::vector<Real>>("energy_threshold")
                          : std::vector<Real>()),
    _basis_functions(declareRestartableData<
                     std::map<VariableName, std::vector<std::unique_ptr<DenseVector<Real>>>>>(
        "basis_functions")),
    _eigen_values(
        declareRestartableData<std::map<VariableName, std::vector<Real>>>("eigen_values")),
    _extra_slepc_options(getParam<std::string>("extra_slepc_options"))

{
  if (isParamValid("num_modes"))
  {
    if (_num_modes.size() != _variable_names.size())
      paramError("num_modes", "The number of modes should be defined for each variable!");

    for (const auto & mode : _num_modes)
      if (!mode)
        paramError("num_modes", "The number of modes should always be a positive!");
  }

  if (isParamValid("energy_threshold"))
  {
    if (_energy_threshold.size() != _variable_names.size())
      paramError("energy_threshold", "The energy thresholds should be defined for each variable!");

    for (const auto & threshold : _energy_threshold)
      if (threshold < 0 || threshold >= 1)
        paramError("energy_threshold",
                   "The energy thresholds should always be in the [0,1) range!");
  }

  for (const auto & vname : _variable_names)
  {
    _eigen_values.emplace(vname, std::vector<Real>());
    _basis_functions.emplace(vname, std::vector<std::unique_ptr<DenseVector<Real>>>());
    _svds.try_emplace(vname);
    SVDCreate(_communicator.get(), &_svds[vname]);
    _computed_svd.emplace(vname, false);
  }
}

PODMapping::~PODMapping()
{
  for (const auto & vname : _variable_names)
  {
    SVDDestroy(&_svds[vname]);
  }
}

unsigned int
PODMapping::determineNumberOfModes(const VariableName & vname,
                                   const std::vector<Real> & converged_evs)
{
  unsigned int num_modes = 0;

  auto it = std::find(_variable_names.begin(), _variable_names.end(), vname);
  mooseAssert(it != _variable_names.end(), "Variable " + vname + " is not in PODMapping!");

  unsigned int var_i = std::distance(_variable_names.begin(), it);

  if (_num_modes.size())
  {
    unsigned int num_requested_modes =
        std::min((unsigned long)_num_modes[var_i], converged_evs.size());
    for (auto mode_i : make_range(num_requested_modes))
    {
      if (std::pow(converged_evs[mode_i], 2) > std::numeric_limits<Real>::epsilon())
      {
        num_modes = mode_i + 1;
      }
    }
  }

  if (_energy_threshold.size())
  {
    Real cumulative_evs = 0;
    Real sum_evs = 0;
    Real threshold = _energy_threshold[var_i] + std::numeric_limits<Real>::epsilon();
    for (auto mode_i : index_range(converged_evs))
    {
      Real ev = std::pow(converged_evs[mode_i], 2);
      sum_evs += (ev > std::numeric_limits<Real>::epsilon()) ? ev : 0.0;
    }
    for (auto mode_i : index_range(converged_evs))
    {
      Real ev = std::pow(converged_evs[mode_i], 2);
      cumulative_evs += (ev > std::numeric_limits<Real>::epsilon()) ? ev : 0.0;
      if (cumulative_evs / sum_evs > 1 - threshold)
      {
        num_modes = mode_i + 1;
        break;
      }
    }
  }

  return num_modes;
}

void
PODMapping::buildMapping(const VariableName & vname)
{
  auto it = std::find(_variable_names.begin(), _variable_names.end(), vname);
  mooseAssert(it != _variable_names.end(), "Variable " + vname + " is not in PODMapping!");
  unsigned int var_i = std::distance(_variable_names.begin(), it);

  UserObjectName parallel_storage_name = getParam<UserObjectName>("solution_storage");
  /// Reference to FEProblemBase instance
  FEProblemBase & feproblem = *_pars.get<FEProblemBase *>("_fe_problem_base");

  std::vector<UserObject *> reporters;
  feproblem.theWarehouse()
      .query()
      .condition<AttribSystem>("UserObject")
      .condition<AttribName>(parallel_storage_name)
      .queryInto(reporters);

  if (reporters.empty())
    paramError(
        "solution_storage", "Unable to find reporter with name '", parallel_storage_name, "'");
  else if (reporters.size() > 1)
    paramError("solution_storage",
               "We found more than one reporter with the name '",
               parallel_storage_name,
               "'");

  _parallel_storage = dynamic_cast<ParallelSolutionStorage *>(reporters[0]);

  if (!_parallel_storage)
    paramError("solution_storage",
               "The parallel storage reporter is not of type '",
               parallel_storage_name,
               "'");

  Mat mat;

  unsigned int local_rows = 0;
  unsigned int snapshot_size = 0;
  unsigned int global_rows = 0;
  if (_parallel_storage->getStorage().size())
  {
    local_rows = _parallel_storage->getStorage(vname).size();
    global_rows = local_rows;
    snapshot_size = _parallel_storage->getStorage(vname).begin()->second[0]->size();
  }

  comm().sum(global_rows);
  comm().max(snapshot_size);

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

  dof_id_type local_beg;
  dof_id_type local_end;

  MatGetOwnershipRange(mat, numeric_petsc_cast(&local_beg), numeric_petsc_cast(&local_end));

  unsigned int counter = 0;
  if (local_rows)
    for (const auto & row : _parallel_storage->getStorage(vname))
    {
      std::vector<PetscInt> rows(snapshot_size, (counter++) + local_beg);
      std::vector<PetscInt> columns = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
      MatSetValues(mat,
                   1,
                   rows.data(),
                   snapshot_size,
                   columns.data(),
                   row.second[0]->get_values().data(),
                   INSERT_VALUES);
    }

  MatAssemblyBegin(mat, MAT_FINAL_ASSEMBLY);
  MatAssemblyEnd(mat, MAT_FINAL_ASSEMBLY);

  ierr = MatView(mat, PETSC_VIEWER_STDOUT_SELF);
  LIBMESH_CHKERR(ierr);

  SVDCreate(_communicator.get(), &_svds[vname]);
  SVDSetOperators(_svds[vname], mat, NULL);
  // SVDSetProblemType(svd, SVD_STANDARD);
  SVDSetType(_svds[vname], SVDTRLANCZOS);
  ierr = PetscOptionsInsertString(NULL, _extra_slepc_options.c_str());
  LIBMESH_CHKERR(ierr);
  SVDSetFromOptions(_svds[vname]);
  SVDSetDimensions(_svds[vname],
                   _num_modes[var_i],
                   std::min(2 * _num_modes[var_i], global_rows),
                   std::min(2 * _num_modes[var_i], global_rows));

  ierr = SVDSolve(_svds[vname]);
  LIBMESH_CHKERR(ierr);

  PetscInt nconv;
  ierr = SVDGetConverged(_svds[vname], &nconv);
  LIBMESH_CHKERR(ierr);

  PetscVector<Real> u(_communicator);
  // PetscVector<Real> v(_communicator);
  u.init(snapshot_size);

  const numeric_index_type converted_asd = global_rows;
  const numeric_index_type converted_local = local_rows;

  // // std::cerr << converted_asd << std::endl;
  // v.init(converted_asd, converted_local, false, PARALLEL);
  PetscReal sigma;
  // PetscReal error;

  _basis_functions[vname].clear();
  _eigen_values[vname].clear();

  const auto emplaced_eigenvalues = _eigen_values.emplace(vname, std::vector<Real>());

  if (emplaced_eigenvalues.second)
    emplaced_eigenvalues.first->second.clear();

  for (PetscInt j = 0; j < nconv; j++)
  {
    ierr = SVDGetSingularTriplet(_svds[vname], j, &sigma, NULL, NULL);
    LIBMESH_CHKERR(ierr);
    emplaced_eigenvalues.first->second.emplace_back(sigma);
  }

  std::ostringstream mystream;
  mystream << "Proc " << processor_id() << " EVs "
           << Moose::stringify(emplaced_eigenvalues.first->second) << std::endl;

  unsigned int num_requested_modes =
      determineNumberOfModes(vname, emplaced_eigenvalues.first->second);

  mystream << " Need this many modes " << num_requested_modes << std::endl;

  for (PetscInt j = 0; j < num_requested_modes; j++)
  {
    SVDGetSingularTriplet(_svds[vname], j, &sigma, NULL, u.vec());
    std::unique_ptr<DenseVector<Real>> serialized_mode = std::make_unique<DenseVector<Real>>();
    u.localize(serialized_mode->get_values());
    _basis_functions[vname].push_back(std::move(serialized_mode));

    mystream << " Mode " << j << " " << Moose::stringify(_basis_functions[vname][j]->get_values())
             << std::endl;
  }
  _computed_svd[vname] = true;

  std::cerr << mystream.str() << std::endl;

  MatDestroy(&mat);
}

void
PODMapping::map(const VariableName & vname,
                const unsigned int global_sample_i,
                std::vector<Real> & reduced_order_vector) const
{
  mooseAssert(_parallel_storage, "We need the parallel solution storage for this operation.");
  mooseAssert(_basis_functions.find(vname) != _basis_functions.end(),
              "The bases for the requested variable are not available!");
  const auto & snapshot = *_parallel_storage->getGlobalSample(global_sample_i, vname)[0];

  const auto & bases = _basis_functions[vname];

  reduced_order_vector.clear();
  reduced_order_vector.resize(bases.size());
  for (auto base_i : index_range(bases))
    reduced_order_vector[base_i] = bases[base_i]->dot(snapshot);
}

void
PODMapping::map(const DenseVector<Real> & full_order_vector,
                std::vector<Real> & reduced_order_vector) const
{
  std::cerr << "Something smart" << std::endl;
}

void
PODMapping::map(const NumericVector<Number> & full_order_vector,
                std::vector<Real> & reduced_order_vector) const
{
  std::cerr << "Something smart" << std::endl;
}

void
PODMapping::inverse_map(const std::vector<Real> & reduced_order_vector,
                        std::vector<Real> & full_order_vector) const
{
  std::cerr << "Something smart" << std::endl;
}
