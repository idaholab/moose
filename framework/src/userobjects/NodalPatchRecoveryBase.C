//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPatchRecoveryBase.h"
#include "MathUtils.h"

#include <Eigen/Dense>

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"
#include "libmesh/parallel_eigen.h"
#include <iomanip>

static std::vector<dof_id_type>
deduplicate(const std::vector<dof_id_type> & ids)
{
  std::vector<dof_id_type> key = ids;
  std::sort(key.begin(), key.end());
  key.erase(std::unique(key.begin(), key.end()), key.end());
  return key;
}

InputParameters
NodalPatchRecoveryBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();

  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH");
  params.addRequiredParam<MooseEnum>(
      "patch_polynomial_order",
      orders,
      "Polynomial order used in least squares fitting of material property "
      "over the local patch of elements connected to a given node");

  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 2; });

  params.addParamNamesToGroup("patch_polynomial_order", "Advanced");

  return params;
}

NodalPatchRecoveryBase::NodalPatchRecoveryBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _qp(0),
    _patch_polynomial_order(
        static_cast<unsigned int>(getParam<MooseEnum>("patch_polynomial_order"))),
    _multi_index(MathUtils::multiIndex(_mesh.dimension(), _patch_polynomial_order)),
    _q(_multi_index.size()),
    _distributed_mesh(_mesh.isDistributedMesh()),
    _proc_ids(n_processors())
{
  std::iota(_proc_ids.begin(), _proc_ids.end(), 0);
}

Real
NodalPatchRecoveryBase::nodalPatchRecovery(const Point & x,
                                           const std::vector<dof_id_type> & elem_ids) const
{
  const RealEigenVector coef = getCoefficientsNoCache(elem_ids); // const version
  // Compute the fitted nodal value
  RealEigenVector p = evaluateBasisFunctions(x);
  return p.dot(coef);
}

const RealEigenVector
NodalPatchRecoveryBase::getCoefficientsNoCache(const std::vector<dof_id_type> & elem_ids) const
{
  auto elem_ids_reduced = deduplicate(elem_ids);

  RealEigenVector coef = RealEigenVector::Zero(_q);
  // Before we go, check if we have enough sample points for solving the least square fitting
  if (_q_point.size() * elem_ids_reduced.size() < _q)
    mooseError("There are not enough sample points to recover the nodal value, try reducing the "
               "polynomial order or using a higher-order quadrature scheme.");

  // Assemble the least squares problem over the patch
  RealEigenMatrix A = RealEigenMatrix::Zero(_q, _q);
  RealEigenVector b = RealEigenVector::Zero(_q);
  for (auto elem_id : elem_ids_reduced)
  {
    const auto elem = _mesh.elemPtr(elem_id);
    if (elem /*prevent segmentation fault in distributed mesh*/)
      if (!hasBlocks(elem->subdomain_id()))
        mooseError("Element with id = ",
                   elem_id,
                   " is not in the block. "
                   "Please use nodalPatchRecovery with elements in the block only.");

    if (_Ae.find(elem_id) == _Ae.end())
      mooseError("Missing entry for elem_id = ", elem_id, " in _Ae.");
    if (_be.find(elem_id) == _be.end())
      mooseError("Missing entry for elem_id = ", elem_id, " in _be.");

    A += libmesh_map_find(_Ae, elem_id);
    b += libmesh_map_find(_be, elem_id);
  }

  // Solve the least squares fitting
  coef = A.completeOrthogonalDecomposition().solve(b);

  return coef;
}

const RealEigenVector
NodalPatchRecoveryBase::getCoefficients(const std::vector<dof_id_type> & elem_ids)
{
  // Check cache
  auto key = deduplicate(elem_ids);

  if (key == _cached_elem_ids)
    return _cached_coef;
  else
  {
    const auto coef = getCoefficientsNoCache(key); // const version

    _cached_elem_ids = key; // Update the cached element IDs
    _cached_coef = coef;    // Update the cached coefficients

    return coef;
  }
}

RealEigenVector
NodalPatchRecoveryBase::evaluateBasisFunctions(const Point & q_point) const
{
  RealEigenVector p(_q);
  Real polynomial;
  for (unsigned int r = 0; r < _multi_index.size(); r++)
  {
    polynomial = 1.0;
    mooseAssert(_multi_index[r].size() == _mesh.dimension(), "Wrong multi-index size.");
    for (unsigned int c = 0; c < _multi_index[r].size(); c++)
      for (unsigned int p = 0; p < _multi_index[r][c]; p++)
        polynomial *= q_point(c);
    p(r) = polynomial;
  }
  return p;
}

void
NodalPatchRecoveryBase::initialize()
{
  _Ae.clear();
  _be.clear();
  // make sure to clear the cached coefficients
  // so that the next time we call nodalPatchRecovery, we will recompute the coefficients to make
  // sure _Ae and _be has already been different but we do not use the same coefficients for the
  // same element
  _cached_elem_ids.clear();
  _cached_coef = RealEigenVector::Zero(_q);
}

void
NodalPatchRecoveryBase::execute()
{
  RealEigenMatrix Ae = RealEigenMatrix::Zero(_q, _q);
  RealEigenVector be = RealEigenVector::Zero(_q);
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    RealEigenVector p = evaluateBasisFunctions(_q_point[_qp]);
    Ae += p * p.transpose();
    be += computeValue() * p;
  }

  dof_id_type elem_id = _current_elem->id();

  _Ae[elem_id] = Ae;
  _be[elem_id] = be;
}

void
NodalPatchRecoveryBase::threadJoin(const UserObject & uo)
{
  const auto & npr = static_cast<const NodalPatchRecoveryBase &>(uo);
  _Ae.insert(npr._Ae.begin(), npr._Ae.end());
  _be.insert(npr._be.begin(), npr._be.end());
}

void
NodalPatchRecoveryBase::finalize()
{
  // When calling nodalPatchRecovery, we may need to know _Ae and _be on algebraically ghosted
  // elements. However, this userobject is only run on local elements, so we need to query those
  // information from other processors in this finalize() method.
  sync();
}

std::unordered_map<processor_id_type, std::vector<dof_id_type>>
NodalPatchRecoveryBase::gatherSendList(const std::vector<dof_id_type> & specific_elems)
{
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> query_ids;

  typedef std::pair<processor_id_type, dof_id_type> PidElemPair;
  std::unordered_map<processor_id_type, std::vector<PidElemPair>> push_data;

  for (const auto & entry : specific_elems)
  {
    const auto * elem = _mesh.elemPtr(entry);
    if (_distributed_mesh)
    {
      if (!elem)
        continue; // Prevent segmentation fault in distributed mesh

      if (hasBlocks(elem->subdomain_id()))
        for (processor_id_type pid = 0; pid < n_processors(); ++pid)
          if (pid != processor_id())
            push_data[pid].push_back(std::make_pair(elem->processor_id(), elem->id()));
    }
    else
      // For non-distributed meshes, element information is always accessible (not nullptr and
      // processor ID can be retrieved), even if the element does not belong to the current
      // processor.
      addToQuery(elem, query_ids);
  }

  if (_distributed_mesh)
  {
    auto push_receiver =
        [&](const processor_id_type, const std::vector<PidElemPair> & received_data)
    {
      for (const auto & [pid, id] : received_data)
        query_ids[pid].push_back(id);
    };

    Parallel::push_parallel_vector_data(_mesh.comm(), push_data, push_receiver);
  }

  return query_ids;
}

std::unordered_map<processor_id_type, std::vector<dof_id_type>>
NodalPatchRecoveryBase::gatherSendList()
{
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> query_ids;

  typedef std::pair<processor_id_type, dof_id_type> PidElemPair;
  std::unordered_map<processor_id_type, std::vector<PidElemPair>> push_data;

  for (const auto & elem : _fe_problem.getEvaluableElementRange())
    addToQuery(elem, query_ids);

  return query_ids;
}

void
NodalPatchRecoveryBase::sync()
{
  const auto query_ids = gatherSendList();
  syncHelper(query_ids);
}

void
NodalPatchRecoveryBase::sync(const std::vector<dof_id_type> & specific_elems)
{
  const auto query_ids = gatherSendList(specific_elems);
  syncHelper(query_ids);
}

void
NodalPatchRecoveryBase::syncHelper(
    const std::unordered_map<processor_id_type, std::vector<dof_id_type>> & query_ids)
{
  typedef std::pair<RealEigenMatrix, RealEigenVector> AbPair;

  // Answer queries received from other processors
  auto gather_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            std::vector<AbPair> & ab_pairs)
  {
    for (const auto & elem_id : elem_ids)
      ab_pairs.emplace_back(libmesh_map_find(_Ae, elem_id), libmesh_map_find(_be, elem_id));
  };

  // Gather answers received from other processors
  auto act_on_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            const std::vector<AbPair> & ab_pairs)
  {
    for (const auto i : index_range(elem_ids))
    {
      const auto elem_id = elem_ids[i];
      const auto & [Ae, be] = ab_pairs[i];
      _Ae[elem_id] = Ae;
      _be[elem_id] = be;
    }
  };

  // the send and receive are called inside the pull_parallel_vector_data function
  libMesh::Parallel::pull_parallel_vector_data<AbPair>(
      _communicator, query_ids, gather_data, act_on_data, 0);
}

void
NodalPatchRecoveryBase::addToQuery(
    const libMesh::Elem * elem,
    std::unordered_map<processor_id_type, std::vector<dof_id_type>> & query_ids)
{
  if (hasBlocks(elem->subdomain_id()) && elem->processor_id() != processor_id())
    query_ids[elem->processor_id()].push_back(elem->id());
}
