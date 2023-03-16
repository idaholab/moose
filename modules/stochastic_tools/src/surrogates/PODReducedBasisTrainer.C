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
  InputParameters params = SurrogateTrainerBase::validParams();

  params.addClassDescription("Computes the reduced subspace plus the reduced operators for "
                             "POD-RB surrogate.");
  params.addRequiredParam<std::vector<std::string>>("var_names",
                                                    "Names of variables we want to "
                                                    "extract from solution vectors.");
  params.addRequiredParam<std::vector<Real>>("error_res",
                                             "The errors allowed in the snapshot reconstruction.");
  params.addRequiredParam<std::vector<std::string>>("tag_names",
                                                    "Names of tags for the reduced operators.");
  params.addParam<std::vector<std::string>>(
      "filenames", "Files where the eigenvalues are printed for each variable (if given).");

  params.addRequiredParam<std::vector<std::string>>(
      "tag_types",
      "List of keywords describing if the tags"
      " correspond to independent operators or not. (op/op_dir/src/src_dir)");
  return params;
}

PODReducedBasisTrainer::PODReducedBasisTrainer(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    _var_names(declareModelData<std::vector<std::string>>(
        "_var_names", getParam<std::vector<std::string>>("var_names"))),
    _error_res(getParam<std::vector<Real>>("error_res")),
    _tag_names(declareModelData<std::vector<std::string>>(
        "_tag_names", getParam<std::vector<std::string>>("tag_names"))),
    _tag_types(declareModelData<std::vector<std::string>>(
        "_tag_types", getParam<std::vector<std::string>>("tag_types"))),
    _base(declareModelData<std::vector<std::vector<DenseVector<Real>>>>("_base")),
    _red_operators(declareModelData<std::vector<DenseMatrix<Real>>>("_red_operators")),
    _base_completed(false),
    _empty_operators(true)
{
  if (_error_res.size() != _var_names.size())
    paramError("error_res",
               "The number of elements is not equal to the number"
               " of elements in 'var_names'!");

  if (_tag_names.size() != _tag_types.size())
    paramError("tag_types",
               "The number of elements is not equal to the number"
               " of elements in 'tag_names'!");

  std::vector<std::string> available_names{"op", "src", "op_dir", "src_dir"};
  for (auto tag_type : _tag_types)
  {
    auto it = std::find(available_names.begin(), available_names.end(), tag_type);
    if (it == available_names.end())
      paramError("tag_types",
                 "Tag type '",
                 tag_type,
                 "' is not valid, available names are:",
                 " op, op_dir, src, src_dir");
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
PODReducedBasisTrainer::execute()
{
  // If the base is not ready yet, create it by performing the POD on the
  // snapshot matrices. Also, initialize the containers for the reduced
  // operators.
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
  // If the operators are already filled on each processor, gather and sum them
  // together. This way every processor has access to every complete reduced
  // operator.
  if (!_empty_operators)
    for (unsigned int tag_i = 0; tag_i < _red_operators.size(); ++tag_i)
    {
      gatherSum(_red_operators[tag_i].get_values());
    }
}

void
PODReducedBasisTrainer::addSnapshot(unsigned int var_i,
                                    unsigned int glob_i,
                                    const std::shared_ptr<DenseVector<Real>> & snapshot)
{
  _snapshots[var_i].addNewEntry(glob_i, snapshot);
}

void
PODReducedBasisTrainer::computeCorrelationMatrix()
{
  // Getting the number of snapshots. It is assumed that every variable has the
  // same number of snapshots. This assumption is used at multiple locations in
  // this source file.
  const auto no_snaps = getSnapsSize(0);

  // Initializing the correlation matrices for each variable.
  for (unsigned int var_i = 0; var_i < _snapshots.size(); ++var_i)
    _corr_mx[var_i] = DenseMatrix<Real>(no_snaps, no_snaps);

  /*
  Creating a simple 2D map that distributes the elements in the correlation
  matrices to each processor. A square mesh with elements corresponding to
  the entries in the correlation matrix is used, since optimal partitioning
  routines are already implemented for it. In this case, the centroids of the
  elements are equivalent to the indices of the correlation matrix. This method
  may seem too convoluted, but is necessary to ensure that the communication
  stays balanced and scales well for runs with a large number of processors.
  */
  ReplicatedMesh mesh(_communicator, 2);
  MeshTools::Generation::build_square(
      mesh, no_snaps, no_snaps, -0.5, no_snaps - 0.5, -0.5, no_snaps - 0.5);

  // A flag is added to each element saying if the result has already been compuuted
  // or not.
  mesh.add_elem_integer("filled");

  // Since the correlation matrix is symmetric, it is enough to compute the
  // entries on the diagonal in addition to the elements above the diagonal.
  // Therefore, the elements in the mesh below the diagonal are deleted.
  for (Elem * elem : mesh.active_element_ptr_range())
  {
    const auto centroid = elem->vertex_average();
    if (centroid(0) > centroid(1))
      mesh.delete_elem(elem);
  }

  // The mesh is distributed among the processors.
  mesh.prepare_for_use();

  /*
  This step restructures the snapshots into an unordered map to make it easier
  to do cross-products in the later stages. Since _snapshots only contains
  pointers to the data, this should not include a considerable amount of copy
  operations.
  */
  std::unordered_map<unsigned int, std::vector<std::shared_ptr<DenseVector<Real>>>> local_vectors;
  for (unsigned int loc_vec_i = 0; loc_vec_i < _snapshots[0].getNumberOfLocalEntries(); ++loc_vec_i)
  {
    const unsigned int glob_vec_i = _snapshots[0].getGlobalIndex(loc_vec_i);
    auto & entry = local_vectors[glob_vec_i];

    for (unsigned int v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
      entry.push_back(_snapshots[v_ind].getLocalEntry(loc_vec_i));
  }

  /*
  Initializing containers for the objects that will be sent to other processors.
  send_vectors is just a temporary object that ensures that the same snapshot is
  sent to other processors only once. send_map, on the other hand, will be used
  by the communicator and contains the required snapshots for each processor
  which is not the the current rank.
  std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>> type
  is a container that uses (global snapshot index, variable index, snapshot) to
  identify and send/receive snapshots during the communication.
  */

  std::unordered_map<processor_id_type, std::set<unsigned int>> send_vectors;
  std::unordered_map<
      processor_id_type,
      std::vector<std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>>>>
      send_map;

  // Fill the send map with snapshots we need to send for each processor. First,
  // we loop over the matrix entries (elements in the mesh)
  for (Elem * elem : mesh.active_element_ptr_range())
    if (elem->processor_id() != processor_id())
    {
      // The centroids in the mesh correspond to the 2D corrdinates of the elements
      // in the mesh.
      const auto centroid = elem->vertex_average();
      const unsigned int i = centroid(0);
      const unsigned int j = centroid(1);

      /*
      Checking if the current processor has the required snapshot.
      We assume that every variable has the same number of snapshots with the
      same distribution among processors. Therefore, it is enough to test
      the first variable only.
      */
      if (_snapshots[0].hasGlobalEntry(i))
      {
        // Continue loop if the snapshot is already being sent.
        if (send_vectors[elem->processor_id()].count(i))
          continue;

        // Add another entry to the map if another processor needs the owned
        // snapshot.
        send_vectors[elem->processor_id()].insert(i);
        for (unsigned int v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
          send_map[elem->processor_id()].emplace_back(
              i, v_ind, _snapshots[v_ind].getGlobalEntry(i));
      }
      else if (_snapshots[0].hasGlobalEntry(j))
      {
        // Continue loop if the snapshot is already being sent.
        if (send_vectors[elem->processor_id()].count(j))
          continue;

        // Add another entry to the map if another processor needs the owned
        // snapshot.
        send_vectors[elem->processor_id()].insert(j);
        for (unsigned int v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
          send_map[elem->processor_id()].emplace_back(
              j, v_ind, _snapshots[v_ind].getGlobalEntry(j));
      }
    }
    else
      // Initializing the flag value with 0 (not computed) for each element.
      elem->set_extra_integer(0, 0);

  // Creating container for received data. In this case the map contains the snapshots
  // for each variable for each required global snapshot index.
  std::unordered_map<unsigned int, std::vector<std::shared_ptr<DenseVector<Real>>>>
      received_vectors;

  // Converting function to functor to be able to pass to push packed range.
  auto functor =
      [this, &mesh, &received_vectors, &local_vectors](
          processor_id_type pid,
          const std::vector<
              std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>>> & vectors)
  { PODReducedBasisTrainer::receiveObjects(mesh, received_vectors, local_vectors, pid, vectors); };

  Parallel::push_parallel_packed_range(_communicator, send_map, (void *)nullptr, functor);

  // This extra call is necessary in case a processor has all the elements it needs
  // (hence doesn't receive any).
  functor(
      processor_id(),
      std::vector<std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>>>());

  // Now, the correlation matrices aregathered and  summed to make sure every processor
  // sees them.
  gatherSum(_corr_mx[0].get_values());

  // The lower triangle of the matrices are then filled using symmetry.
  for (auto & corr_mx : _corr_mx)
    for (unsigned int row_i = 1; row_i < corr_mx.m(); ++row_i)
      for (unsigned int col_i = 0; col_i < row_i; ++col_i)
        corr_mx(row_i, col_i) = corr_mx(col_i, row_i);
}

void
PODReducedBasisTrainer::receiveObjects(
    ReplicatedMesh & mesh,
    std::unordered_map<unsigned int, std::vector<std::shared_ptr<DenseVector<Real>>>> &
        received_vectors,
    std::unordered_map<unsigned int, std::vector<std::shared_ptr<DenseVector<Real>>>> &
        local_vectors,
    processor_id_type /*pid*/,
    const std::vector<std::tuple<unsigned int, unsigned int, std::shared_ptr<DenseVector<Real>>>> &
        vectors)
{
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

  // Looping over the locally owned entries in the matrix and computing the
  // values.
  for (Elem * elem : mesh.active_local_element_ptr_range())
  {
    // If the matrix entry is already filled, the loop skips this element.
    if (elem->get_extra_integer(0))
      continue;

    // Getting pointers to the necessary snapshots for the matrix entry.
    // This points to a vector of size (number of variables).
    const unsigned int i = elem->vertex_average()(0);
    const unsigned int j = elem->vertex_average()(1);
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

    // Coputing the available matrix entries for every variable.
    for (unsigned int v_ind = 0; v_ind < _snapshots.size(); ++v_ind)
      _corr_mx[v_ind](i, j) = (*i_vec)[v_ind]->dot(*(*j_vec)[v_ind]);

    // Set the 'filled' flag to 1 (true) to make sure it is not recomputed.
    elem->set_extra_integer(0, 1);
  }
}

void
PODReducedBasisTrainer::computeEigenDecomposition()
{
  for (unsigned int v_ind = 0; v_ind < _corr_mx.size(); ++v_ind)
  {
    unsigned int no_snaps = _corr_mx[v_ind].n();

    // Initializing temporary objects for the eigenvalues and eigenvectors since
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

    // Getting a cutoff for the number of modes. The function requires a sorted list,
    // thus the temporary vector is sorted.
    std::stable_sort(v.begin(), v.end(), std::greater<Real>());
    unsigned int cutoff = determineNumberOfModes(_error_res[v_ind], v);

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

    printEigenvalues();
  }
}

unsigned int
PODReducedBasisTrainer::determineNumberOfModes(Real error, const std::vector<Real> & inp_vec) const
{
  Real sum = std::accumulate(inp_vec.begin(), inp_vec.end(), 0.0);

  Real part_sum = 0.0;
  for (unsigned int i = 0; i < inp_vec.size(); ++i)
  {
    part_sum += inp_vec[i];
    if (part_sum / sum > 1 - error)
      return (i + 1);
  }
  return (inp_vec.size());
}

void
PODReducedBasisTrainer::computeBasisVectors()
{
  // Looping over all the variables.
  for (unsigned int var_i = 0; var_i < _eigenvectors.size(); ++var_i)
  {
    unsigned int no_bases = _eigenvalues[var_i].size();
    unsigned int no_snaps = _snapshots[var_i].getNumberOfLocalEntries();

    _base[var_i].resize(no_bases);

    // Filling the containers using the local snapshots and the eigenvalues and
    // eigenvectors of the correlation matrices.
    for (unsigned int base_i = 0; base_i < no_bases; ++base_i)
    {
      _base[var_i][base_i].resize(_snapshots[var_i].getLocalEntry(0)->size());

      for (unsigned int loc_i = 0; loc_i < no_snaps; ++loc_i)
      {
        unsigned int glob_i = _snapshots[var_i].getGlobalIndex(loc_i);
        const DenseVector<Real> & snapshot = *_snapshots[var_i].getLocalEntry(loc_i);

        for (unsigned int i = 0; i < _base[var_i][base_i].size(); ++i)
        {
          _base[var_i][base_i](i) += _eigenvectors[var_i](glob_i, base_i) * snapshot(i);
        }
      }

      // Gathering and summing the local contributions over all of the processes.
      // This makes sure that every process sees all of the basis functions.
      gatherSum(_base[var_i][base_i].get_values());

      // Normalizing the basis functions to make sure they are orthonormal.
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
    if (_tag_types[tag_i] == "src" || _tag_types[tag_i] == "src_dir")
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
  // on the residual. This is done in parallel and the local contributions are
  // gathered in finalize().
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

unsigned int
PODReducedBasisTrainer::getSnapsSize(unsigned int var_i) const
{
  return _snapshots[var_i].getNumberOfGlobalEntries();
}

unsigned int
PODReducedBasisTrainer::getSumBaseSize() const
{
  unsigned int sum = 0;

  for (unsigned int i = 0; i < _base.size(); ++i)
    sum += _base[i].size();

  return sum;
}

const DenseVector<Real> &
PODReducedBasisTrainer::getBasisVector(unsigned int var_i, unsigned int base_i) const
{
  return _base[var_i][base_i];
}

const DenseVector<Real> &
PODReducedBasisTrainer::getBasisVector(unsigned int glob_i) const
{
  unsigned int counter = 0;

  for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
    for (unsigned int base_i = 0; base_i < _base[var_i].size(); ++base_i)
    {
      if (glob_i == counter)
        return _base[var_i][base_i];

      counter += 1;
    }

  mooseError("The basis vector with global index ", glob_i, "is not available!");
  return _base[0][0];
}

unsigned int
PODReducedBasisTrainer::getVariableIndex(unsigned int glob_i) const
{
  unsigned int counter = 0;

  for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
    for (unsigned int base_i = 0; base_i < _base[var_i].size(); ++base_i)
    {
      if (glob_i == counter)
        return var_i;

      counter += 1;
    }

  mooseError("Variable with global base index ", glob_i, "is not available!");
  return 0;
}

void
PODReducedBasisTrainer::printEigenvalues()
{
  if (processor_id() == 0 && _tid == 0 && isParamValid("filenames"))
  {
    std::vector<std::string> filenames = getParam<std::vector<std::string>>("filenames");

    if (filenames.size() != _var_names.size())
      paramError("filenames",
                 "The number of file names is not equal to the number of variable names!");

    for (unsigned int var_i = 0; var_i < _var_names.size(); ++var_i)
    {
      std::filebuf fb;
      fb.open(filenames[var_i], std::ios::out);
      std::ostream os(&fb);
      os << "evs" << std::endl;
      _eigenvalues[var_i].print_scientific(os);
    }
  }
}
