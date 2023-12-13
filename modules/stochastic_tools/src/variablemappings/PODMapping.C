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
                          : nullptr),
    _pod(StochasticTools::POD(_parallel_storage, _extra_slepc_options, _communicator))
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
      _mapping_ready_to_use.emplace(vname, false);
    }
#endif
  }
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
    // Clear storage containers for the basis functions and singular values.
    _left_basis_functions[vname].clear();
    _right_basis_functions[vname].clear();
    _singular_values[vname].clear();
    // Find the number of modes that we would want to compute
    std::size_t num_modes_compute = (std::size_t)_num_modes[var_i];
    // Find the first element that satisfies the threshold
    const Real threshold = (_energy_threshold.empty() ? 0.0 : _energy_threshold[var_i]) +
                           std::numeric_limits<Real>::epsilon();
    // Use POD class to compute for a variable
    _pod.computePOD(vname,
                    _left_basis_functions[vname],
                    _right_basis_functions[vname],
                    _singular_values[vname],
                    num_modes_compute,
                    threshold);
    _mapping_ready_to_use[vname] = true;
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
