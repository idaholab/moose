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
    _base_completed(false)
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
  _snapshots.resize(_var_names.size());

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
    _base_completed = true;
  }
}

void
PODReducedBasisTrainer::finalize()
{
}

void
PODReducedBasisTrainer::addSnapshot(dof_id_type v_ind,
                                    dof_id_type g_ind,
                                    const std::shared_ptr<DenseVector<Real>> & snapshot)
{
  _snapshots[v_ind].push_back(
      std::pair<dof_id_type, std::shared_ptr<DenseVector<Real>>>(g_ind, snapshot));
  std::cerr << processor_id() << " added snaphot for variable " << v_ind << " and vector " << g_ind
            << " at " << snapshot.get() << std::endl;
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
  for (dof_id_type local_vector_ind = 0; local_vector_ind < _snapshots[0].size();
       ++local_vector_ind)
  {
    const dof_id_type global_vector_ind = _snapshots[0][local_vector_ind].first;
    auto & entry = local_vectors[global_vector_ind];
    for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
      entry.push_back(_snapshots[v_ind][local_vector_ind].second);
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
      const auto it =
          std::find_if(_snapshots[0].begin(), _snapshots[0].end(), [&i, &j](const auto & pair) {
            return pair.first == i || pair.first == j;
          });
      if (it != _snapshots[0].end())
      {
        const auto global_vector_ind = it->first;
        if (send_vectors[elem->processor_id()].count(global_vector_ind))
          continue;
        send_vectors[elem->processor_id()].insert(global_vector_ind);
        const auto local_vector_ind = std::distance(_snapshots[0].begin(), it);
        for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
          send_map[elem->processor_id()].emplace_back(
              global_vector_ind, v_ind, _snapshots[v_ind][local_vector_ind].second);
      }
    }
    else
      elem->set_extra_integer(0, 0);

  {
    SerializerGuard sg(_communicator);
    for (const auto & pair : send_vectors)
    {
      std::cerr << processor_id() << "->" << pair.first << ": ";
      for (auto & vec : pair.second)
        std::cerr << vec << " ";
      std::cerr << std::endl;
    }
    std::cerr << processor_id() << " responsible for ";
    for (const Elem * elem : mesh.active_local_element_ptr_range())
      std::cerr << (dof_id_type)elem->centroid()(0) << "," << (dof_id_type)elem->centroid()(1)
                << " ";
    std::cerr << std::endl;
  }

  for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
    _corr_mx[v_ind] = DenseMatrix<Real>(no_snaps, no_snaps);

  _corr_mx[0].print();

  std::stringstream oss;
  // Send and receive the data
  std::unordered_map<dof_id_type, std::vector<std::shared_ptr<DenseVector<Real>>>> received_vectors;
  auto functor =
      [this, &mesh, &received_vectors, &local_vectors, &oss](
          processor_id_type pid,
          const std::vector<
              std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>> & vectors) {
        for (auto & tuple : vectors)
        {
          const auto global_vector_ind = std::get<0>(tuple);
          const auto variable_ind = std::get<1>(tuple);
          const auto & vector = std::get<2>(tuple);

          // The entry that will be filled with all of the variable solutions for a vector
          std::vector<std::shared_ptr<DenseVector<Real>>> & entry =
              received_vectors[global_vector_ind];
          // Size it to the number of variables in case we haven't already
          entry.resize(_snapshots.size(), nullptr);
          // Add this varaible's contribution - this is shared_ptr so we are claiming partial
          // ownership of this vector and do not have to do a copy
          entry[variable_ind] = vector;

          oss << processor_id() << "retreived " << global_vector_ind << " from " << pid << " into "
              << vector.get() << std::endl;
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
              bool have_all = true;
              for (const auto & vec : find_i_received->second)
                if (!vec)
                {
                  have_all = false;
                  break;
                }
              if (have_all)
                i_vec = &find_i_received->second;
              else
                continue;
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
              bool have_all = true;
              for (const auto & vec : find_j_received->second)
                if (!vec)
                {
                  have_all = false;
                  break;
                }
              if (have_all)
                j_vec = &find_j_received->second;
              else
                continue;
            }
            else
              continue;
          }

          elem->set_extra_integer(0, 1);
          for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
            _corr_mx[v_ind](i, j) = (*i_vec)[v_ind]->dot(*(*j_vec)[v_ind]);

          oss << processor_id() << ": " << i << "," << j << " from vectors " << (*i_vec)[0].get()
              << " and " << (*j_vec)[0].get() << std::endl;
        }
      };
  Parallel::push_parallel_packed_range(_communicator, send_map, (void *)nullptr, functor);
  functor(processor_id(),
          std::vector<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>());

  {
    SerializerGuard sg(_communicator);
    std::cerr << oss.str() << std::endl << std::flush;
  }
  gatherSum(_corr_mx[0].get_values());
  _corr_mx[0].print();
  // for (const auto pair : send_to)
  // {
  //   std::cerr << processor_id() << "->" << pair.first << ": ";
  //   for (const auto id : pair.second)
  //     std::cerr << id << " ";
  //   std::cerr << std::endl;
  // }

  // std::cerr << elem->centroid() << " on pid " << elem->processor_id() << std::endl;
  _communicator.barrier();
  mooseError("we suck at coding");
  // Looping over all the variables.
  for (dof_id_type v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
  {
    // Initializing the correlation matrix.
    _corr_mx[v_ind] = DenseMatrix<Real>(no_snaps, no_snaps);

    // Filling the correlation matrix with the inner products between snapshots
    // and utilizing the fact the the correlation matrix is symmetric.
    // for (MooseIndex(no_snaps) j = 0; j < no_snaps; ++j)
    for (dof_id_type j = 0; j < no_snaps; ++j)
    {
      for (dof_id_type k = 0; k < no_snaps; ++k)
      {
        if (j >= k)
          _corr_mx[v_ind](j, k) = _snapshots[v_ind][j].second->dot(*_snapshots[v_ind][k].second);
      }
    }

    for (unsigned int j = 0; j < no_snaps; ++j)
    {
      for (unsigned int k = 0; k < no_snaps; ++k)
      {
        if (j < k)
          _corr_mx[v_ind](j, k) = _corr_mx[v_ind](k, j);
      }
    }
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
  for (unsigned int v_ind = 0; v_ind < _eigenvectors.size(); ++v_ind)
  {
    unsigned int no_bases = _eigenvalues[v_ind].size();

    // Initializing the containers for the basis vectors.
    _base[v_ind] = std::vector<DenseVector<Real>>(no_bases);

    // Filling the containers using the snapshots and the eigenvalues and
    // eigenvectors of the correlation matrices.
    for (unsigned int j = 0; j < no_bases; ++j)
    {
      _base[v_ind][j].resize(_snapshots[v_ind][0].second->size());

      for (unsigned int k = 0; k < _snapshots[v_ind].size(); ++k)
      {
        DenseVector<Real> tmp(*_snapshots[v_ind][k].second);
        tmp.scale(_eigenvectors[v_ind](k, j));

        _base[v_ind][j] += tmp;
      }
      _base[v_ind][j] *= (1.0 / sqrt(_eigenvalues[v_ind](j)));
    }
  }
}

void
PODReducedBasisTrainer::initReducedOperators()
{
  if (!_base_completed)
    mooseError("There are no basis vectors available."
               " This might indicate that a residual transfer is called before"
               " the base generation procedure is finished.");

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
}

dof_id_type
PODReducedBasisTrainer::getSnapsSize(dof_id_type var_i)
{
  dof_id_type val = _snapshots[var_i].size();
  gatherSum(val);
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
  for (unsigned int v_ind = 0; v_ind < _var_names.size(); ++v_ind)
  {
    for (unsigned int b_ind = 0; b_ind < _base[v_ind].size(); ++b_ind)
    {
      if (g_index == counter)
      {
        var_counter = v_ind;
        base_counter = b_ind;
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
  for (unsigned int v_ind = 0; v_ind < _var_names.size(); ++v_ind)
  {
    for (unsigned int b_ind = 0; b_ind < _base[v_ind].size(); ++b_ind)
    {
      if (g_index == counter)
      {
        var_counter = v_ind;
        found = true;
        break;
      }
    }
    if (found)
      break;
  }
  return var_counter;
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

namespace libMesh
{
namespace Parallel
{
unsigned int
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::packable_size(
    const std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> & object,
    const void *)
{
  unsigned int total_size = 0;
  // ID x 2, size
  total_size += 3;
  // Data
  total_size += std::get<2>(object)->size();
  return total_size;
}
unsigned int
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::packed_size(
    typename std::vector<Real>::const_iterator in)
{
  unsigned int total_size = 0;
  // Data size
  total_size += *in++;
  // ID x 2, size
  total_size += 3;
  return total_size;
}
template <>
void
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::pack(
    const std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> & object,
    std::back_insert_iterator<std::vector<Real>> data_out,
    const void *)
{
  // Data size
  const auto & dense_vector = std::get<2>(object);
  data_out = dense_vector->size();
  // IDs
  data_out = std::get<0>(object);
  data_out = std::get<1>(object);
  // Data
  const auto & vector = dense_vector->get_values();
  for (std::size_t i = 0; i < dense_vector->size(); ++i)
    data_out = vector[i];
}
template <>
std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>
Packing<std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>>>::unpack(
    std::vector<Real>::const_iterator in, void *)
{
  // Number of points
  const std::size_t data_size = *in++;
  std::tuple<dof_id_type, dof_id_type, std::shared_ptr<DenseVector<Real>>> object;
  // IDs
  std::get<0>(object) = *in++;
  std::get<1>(object) = *in++;
  // Data
  auto & dense_vector = std::get<2>(object);
  dense_vector = std::make_shared<DenseVector<Real>>(data_size);
  auto & vector_values = dense_vector->get_values();
  for (std::size_t i = 0; i < data_size; ++i)
    vector_values[i] = *in++;
  return object;
}
} // namespace Parallel
} // namespace libMesh
