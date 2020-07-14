//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PODReducedBasisTrainer.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_generation.h"
#include "SerializerGuard.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel_sync.h"
#include "VectorPacker.h"

registerMooseObject("StochasticToolsApp", PODReducedBasisTrainer);

InputParameters
PODReducedBasisTrainer::validParams()
{
  InputParameters params = SurrogateTrainer::validParams();

  params.addClassDescription("Computes the reduced subspace plus the reduced operators for "
                             "POD-RB surrogate.");
  params.addRequiredParam<std::vector<std::string>>("var_names",
                                                    "Names of variables we want to"
                                                    "extract from solution vectors.");
  params.addRequiredParam<std::vector<Real>>("en_limits",
                                             "List of energy retention limits for each variable.");
  params.addRequiredParam<std::vector<std::string>>("tag_names",
                                                    "Names of tags for the reduced operators.");
  params.addParam<std::vector<std::string>>(
      "dir_tag_names",
      std::vector<std::string>(0),
      "Names of tags for reduced operators corresponding to dirichlet BCs.");
  params.addRequiredParam<std::vector<unsigned int>>(
      "independent",
      "List of bools describing if the tags"
      " correspond to and independent operator or not.");
  return params;
}

PODReducedBasisTrainer::PODReducedBasisTrainer(const InputParameters & parameters)
  : SurrogateTrainer(parameters),
    _var_names(declareModelData<std::vector<std::string>>(
        "_var_names", getParam<std::vector<std::string>>("var_names"))),
    _en_limits(getParam<std::vector<Real>>("en_limits")),
    _tag_names(declareModelData<std::vector<std::string>>(
        "_tag_names", getParam<std::vector<std::string>>("tag_names"))),
    _dir_tag_names(declareModelData<std::vector<std::string>>(
        "_dir_tag_names", getParam<std::vector<std::string>>("dir_tag_names"))),
    _independent(declareModelData<std::vector<unsigned int>>(
        "_independent", getParam<std::vector<unsigned int>>("independent"))),
    _base(declareModelData<std::vector<std::vector<DenseVector<Real>>>>("_base")),
    _red_operators(declareModelData<std::vector<DenseMatrix<Real>>>("_red_operators")),
    _base_completed(false),
    _empty_operators(true)
{
  if (_en_limits.size() != _var_names.size())
    paramError("en_limits",
               "The number of elements is not equal to the number"
               " of elements in 'var_names'!");

  if (_tag_names.size() != _independent.size())
    paramError("independent",
               "The number of elements is not equal to the number"
               " of elements in 'tag_names'!");

  for (auto dir_tag : _dir_tag_names)
  {
    auto it = std::find(_tag_names.begin(), _tag_names.end(), dir_tag);
    if (it == _tag_names.end())
      paramError("dir_tag_names", "Dirichlet BC tag '", dir_tag, "' is not present in tag_names!");
  }
}

void
PODReducedBasisTrainer::initialSetup()
{
  // Initializing the containers for the essential data to construct the
  // reduced operators.

  _snapshots.clear();
  _snapshots.resize(_var_names.size(), DistributedSnapshots(_communicator));

  _base.clear();
  _base.resize(_var_names.size());

  _corr_mx.clear();
  _corr_mx.resize(_var_names.size());

  _eigenvalues.clear();
  _eigenvalues.resize(_var_names.size());

  _eigenvectors.clear();
  _eigenvectors.resize(_var_names.size());
}

void
PODReducedBasisTrainer::initialize()
{
}

void
PODReducedBasisTrainer::execute()
{
  // If the base is not ready yet, create it by performing the POD on the
  // snapshot matrices.
  if (!_base_completed)
  {
    computeCorrelationMatrix();
    computeEigenDecomposition();
    computeBasisVectors();
    initReducedOperators();
    _base_completed = true;
    _empty_operators = true;
  }
}

void
PODReducedBasisTrainer::finalize()
{
  if (!_empty_operators)
    for (unsigned int tag_i = 0; tag_i < _red_operators.size(); ++tag_i)
    {
      gatherSum(_red_operators[tag_i].get_values());
    }
}

void
PODReducedBasisTrainer::addSnapshot(dof_id_type v_ind,
                                    dof_id_type g_ind,
                                    const std::shared_ptr<DenseVector<Real>> & snapshot)
{
  _snapshots[v_ind].addNewSample(g_ind, snapshot);
}

void
PODReducedBasisTrainer::computeCorrelationMatrix()
{
  // Getting the number of snapshots. It is assumed that every variable has the
  // same number of snapshots.
  const auto no_snaps = getSnapsSize(0);

  ReplicatedMesh mesh(_communicator, 2);
  MeshTools::Generation::build_square(
      mesh, no_snaps, no_snaps, -0.5, no_snaps - 0.5, -0.5, no_snaps - 0.5);
  mesh.add_elem_integer("filled");

  for (Elem * elem : mesh.active_element_ptr_range())
  {
    const auto centroid = elem->centroid();
    if (centroid(0) > centroid(1))
      mesh.delete_elem(elem);
  }

  mesh.prepare_for_use();

  std::unordered_map<dof_id_type, std::vector<std::shared_ptr<DenseVector<Real>>>> local_vectors;
  for (dof_id_type loc_vec_i = 0; loc_vec_i < _snapshots[0].getNumberOfLocalSamples(); ++loc_vec_i)
  {
    const dof_id_type glob_vec_i = _snapshots[0].getGlobalIndex(loc_vec_i);
    auto & entry = local_vectors[glob_vec_i];

    for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
      entry.push_back(_snapshots[v_ind].getLocalSample(loc_vec_i));
  }

  std::unordered_map<processor_id_type, std::set<dof_id_type>> send_vectors;
  std::unordered_map<
      processor_id_type,
      std::vector<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>>
      send_map;

  // Fill the send map of processors we need to send data to and the corresponding data
  for (Elem * elem : mesh.active_element_ptr_range())
    if (elem->processor_id() != processor_id())
    {
      const auto centroid = elem->centroid();
      const dof_id_type i = centroid(0);
      const dof_id_type j = centroid(1);

      if (_snapshots[0].hasGlobalSample(i))
      {
        if (send_vectors[elem->processor_id()].count(i))
          continue;

        send_vectors[elem->processor_id()].insert(i);
        for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
          send_map[elem->processor_id()].emplace_back(i, v_ind, _snapshots[v_ind].getSample(i));
      }
      else if (_snapshots[0].hasGlobalSample(j))
      {
        if (send_vectors[elem->processor_id()].count(j))
          continue;

        send_vectors[elem->processor_id()].insert(j);
        for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
          send_map[elem->processor_id()].emplace_back(j, v_ind, _snapshots[v_ind].getSample(j));
      }
    }
    else
      elem->set_extra_integer(0, 0);

  for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
    _corr_mx[v_ind] = DenseMatrix<Real>(no_snaps, no_snaps);

  // Send and receive the data
  std::unordered_map<dof_id_type, std::vector<std::shared_ptr<DenseVector<Real>>>> received_vectors;
  auto functor =
      [this, &mesh, &received_vectors, &local_vectors](
          processor_id_type /*pid*/,
          const std::vector<
              std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>> & vectors) {
        for (auto & tuple : vectors)
        {
          const auto glob_vec_i = std::get<0>(tuple);
          const auto var_i = std::get<1>(tuple);
          const auto & vector = std::get<2>(tuple);

          // The entry that will be filled with all of the variable solutions for a vector
          std::vector<std::shared_ptr<DenseVector<Real>>> & entry = received_vectors[glob_vec_i];

          // Size it to the number of variables in case we haven't already
          entry.resize(_snapshots.size(), nullptr);

          // Add this varaible's contribution - this is shared_ptr so we are claiming partial
          // ownership of this vector and do not have to do a copy
          entry[var_i] = vector;
        }

        for (Elem * elem : mesh.active_local_element_ptr_range())
        {
          if (elem->get_extra_integer(0))
            continue;

          const dof_id_type i = elem->centroid()(0);
          const dof_id_type j = elem->centroid()(1);
          std::vector<std::shared_ptr<DenseVector<Real>>> * i_vec = nullptr;
          std::vector<std::shared_ptr<DenseVector<Real>>> * j_vec = nullptr;

          const auto find_i_local = local_vectors.find(i);
          if (find_i_local != local_vectors.end())
            i_vec = &find_i_local->second;
          else
          {
            const auto find_i_received = received_vectors.find(i);
            if (find_i_received != received_vectors.end())
            {
              i_vec = &find_i_received->second;
            }
            else
              continue;
          }

          const auto find_j_local = local_vectors.find(j);
          if (find_j_local != local_vectors.end())
            j_vec = &find_j_local->second;
          else
          {
            const auto find_j_received = received_vectors.find(j);
            if (find_j_received != received_vectors.end())
            {
              j_vec = &find_j_received->second;
            }
            else
              continue;
          }

          elem->set_extra_integer(0, 1);
          for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
            _corr_mx[v_ind](i, j) = (*i_vec)[v_ind]->dot(*(*j_vec)[v_ind]);
        }
      };
  Parallel::push_parallel_packed_range(_communicator, send_map, (void *)nullptr, functor);
  functor(processor_id(),
          std::vector<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>());

  gatherSum(_corr_mx[0].get_values());

  for (auto & corr_mx : _corr_mx)
  {
    for (dof_id_type row_i = 0; row_i < corr_mx.m(); ++row_i)
    {
      for (dof_id_type col_i = 0; col_i < corr_mx.n(); ++col_i)
      {
        if (row_i > col_i)
          corr_mx(row_i, col_i) = corr_mx(col_i, row_i);
      }
    }
    _corr_mx[0].print();
  }
}

void
PODReducedBasisTrainer::computeEigenDecomposition()
{
  for (unsigned int v_ind = 0; v_ind < _corr_mx.size(); ++v_ind)
  {
    unsigned int no_snaps = _corr_mx[v_ind].n();

    // Initializing temprary objects for the eigenvalues and eigenvectors since
    // evd_left() returns an unordered vector of eigenvalues.
    DenseVector<Real> eigenvalues(no_snaps);
    DenseMatrix<Real> eigenvectors(no_snaps, no_snaps);

    // Creating a temporary placeholder for the imaginary parts of the eigenvalues
    DenseVector<Real> eigenvalues_imag(no_snaps);

    // Performing the eigenvalue decomposition
    _corr_mx[v_ind].evd_left(eigenvalues, eigenvalues_imag, eigenvectors);

    // Sorting the eigenvectors and eigenvalues based on the magnitude of
    // the eigenvalues
    std::vector<unsigned int> idx(eigenvalues.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::vector<Real> & v = eigenvalues.get_values();

    // Getting the indices to be able to copy the corresponding eigenvector too.
    std::stable_sort(
        idx.begin(), idx.end(), [&v](unsigned int i, unsigned int j) { return v[i] > v[j]; });

    // Getting a cutoff for the number of modes. The functio nrequires a sorted list,
    // thus the temporary vector is sorted.
    std::stable_sort(v.begin(), v.end(), std::greater<Real>());
    unsigned int cutoff = determineNumberOfModes(_en_limits[v_ind], v);

    // Initializing the actual containers for the eigenvectors and eigenvalues.
    _eigenvalues[v_ind] = DenseVector<Real>(cutoff);
    _eigenvectors[v_ind] = DenseMatrix<Real>(eigenvectors.m(), cutoff);

    // Copying the kept eigenvalues and eigenvectors in a sorted container.
    for (unsigned int j = 0; j < cutoff; ++j)
    {
      _eigenvalues[v_ind](j) = eigenvalues(j);
      for (unsigned int k = 0; k < _eigenvectors[v_ind].m(); ++k)
      {
        _eigenvectors[v_ind](k, j) = eigenvectors(k, idx[j]);
      }
    }
  }
}

unsigned int
PODReducedBasisTrainer::determineNumberOfModes(Real limit, std::vector<Real> & inp_vec)
{
  Real sum = std::accumulate(inp_vec.begin(), inp_vec.end(), 0.0);

  Real part_sum = 0.0;
  for (unsigned int i = 0; i < inp_vec.size(); ++i)
  {
    part_sum += inp_vec[i];
    if (part_sum / sum > limit)
      return (i + 1);
  }
  return (inp_vec.size());
}

void
PODReducedBasisTrainer::computeBasisVectors()
{
  // Looping over all the variables.
  for (dof_id_type var_i = 0; var_i < _eigenvectors.size(); ++var_i)
  {
    unsigned int no_bases = _eigenvalues[var_i].size();
    dof_id_type no_snaps = _snapshots[var_i].getNumberOfLocalSamples();

    _base[var_i].resize(no_bases);

    // Filling the containers using the snapshots and the eigenvalues and
    // eigenvectors of the correlation matrices.
    for (dof_id_type base_i = 0; base_i < no_bases; ++base_i)
    {
      _base[var_i][base_i].resize(_snapshots[var_i].getLocalSample(0)->size());

      for (dof_id_type loc_i = 0; loc_i < no_snaps; ++loc_i)
      {
        dof_id_type glob_i = _snapshots[var_i].getGlobalIndex(loc_i);
        const DenseVector<Real> & snapshot = *_snapshots[var_i].getLocalSample(loc_i);

        for (dof_id_type i = 0; i < _base[var_i][base_i].size(); ++i)
        {
          _base[var_i][base_i](i) += _eigenvectors[var_i](glob_i, base_i) * snapshot(i);
        }
      }

      gatherSum(_base[var_i][base_i].get_values());
      _base[var_i][base_i].scale(1.0 / sqrt(_eigenvalues[var_i](base_i)));
    }
  }
}

void
PODReducedBasisTrainer::initReducedOperators()
{
  _red_operators.resize(_tag_names.size());

  // Getting the sum of the ranks of the different bases. Every reduced operator
  // is resized with this number. This way the construction can be more general.
  unsigned int base_num = getSumBaseSize();

  // Initializing each operator (each operator with a tag).
  for (unsigned int tag_i = 0; tag_i < _red_operators.size(); ++tag_i)
  {
    if (_independent[tag_i])
      _red_operators[tag_i].resize(base_num, 1);
    else
      _red_operators[tag_i].resize(base_num, base_num);
  }
}

void
PODReducedBasisTrainer::addToReducedOperator(unsigned int base_i,
                                             unsigned int tag_i,
                                             std::vector<DenseVector<Real>> & residual)
{
  // Computing the elements of the reduced operator using Galerkin projection
  // on the residual.
  unsigned int counter = 0;
  for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
  {
    for (unsigned int base_j = 0; base_j < _base[var_i].size(); ++base_j)
    {
      _red_operators[tag_i](counter, base_i) = residual[var_i].dot(_base[var_i][base_j]);
      counter++;
    }
  }

  _empty_operators = false;
}

dof_id_type
PODReducedBasisTrainer::getSnapsSize(dof_id_type var_i)
{
  dof_id_type val = _snapshots[var_i].getNumberOfGlobalSamples();
  return val;
}

unsigned int
PODReducedBasisTrainer::getSumBaseSize()
{
  unsigned int sum = 0;

  for (unsigned int i = 0; i < _base.size(); ++i)
    sum += _base[i].size();

  return sum;
}

const DenseVector<Real> &
PODReducedBasisTrainer::getBasisVector(unsigned int v_ind, unsigned int b_ind) const
{
  return _base[v_ind][b_ind];
}

const DenseVector<Real> &
PODReducedBasisTrainer::getBasisVector(unsigned int g_index) const
{
  unsigned int counter = 0;
  unsigned int var_counter = 0;
  unsigned int base_counter = 0;
  bool found = false;
  for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
  {
    for (unsigned int base_i = 0; base_i < _base[var_i].size(); ++base_i)
    {
      if (g_index == counter)
      {
        var_counter = var_i;
        base_counter = base_i;
        found = true;
        break;
      }
    }
    if (found)
      break;
  }
  return _base[var_counter][base_counter];
}

unsigned int
PODReducedBasisTrainer::getVariableIndex(unsigned int g_index)
{
  unsigned int counter = 0;
  unsigned int var_counter = 0;
  bool found = false;
  for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
  {
    for (unsigned int base_i = 0; base_i < _base[var_i].size(); ++base_i)
    {
      if (g_index == counter)
      {
        var_counter = var_i;
        found = true;
        break;
      }
    }
    if (found)
      break;
  }
  return var_counter;
}
