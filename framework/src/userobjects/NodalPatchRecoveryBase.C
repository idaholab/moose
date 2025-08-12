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
    _q(_multi_index.size())
{
}

Real
NodalPatchRecoveryBase::nodalPatchRecovery(const Point & x,
                                           const std::vector<dof_id_type> & elem_ids) const
{
  const RealEigenVector coef = getCoefficients(elem_ids);
  // Compute the fitted nodal value
  RealEigenVector p = evaluateBasisFunctions(x);
  return p.dot(coef);
}

const RealEigenVector
NodalPatchRecoveryBase::getCoefficients(const std::vector<dof_id_type> & elem_ids) const
{
  // Check cache
  std::vector<dof_id_type> key = elem_ids;
  std::sort(key.begin(), key.end());
  if (key == _cached_elem_ids)
    return _cached_coef;
  else
  {
    RealEigenVector coef = RealEigenVector::Zero(_q);
    // Before we go, check if we have enough sample points for solving the least square fitting
    if (_q_point.size() * elem_ids.size() < _q)
      mooseError("There are not enough sample points to recover the nodal value, try reducing the "
                 "polynomial order or using a higher-order quadrature scheme.");

    // Assemble the least squares problem over the patch
    RealEigenMatrix A = RealEigenMatrix::Zero(_q, _q);
    RealEigenVector b = RealEigenVector::Zero(_q);
    for (auto elem_id : elem_ids)
    {
      if (!hasBlocks(_mesh.elemPtr(elem_id)->subdomain_id()))
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
  _synced = false;
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
  if (!_synced)
    sync();
}

std::unordered_map<processor_id_type, std::vector<dof_id_type>>
NodalPatchRecoveryBase::gatherSendList(const std::vector<dof_id_type> & additional_elems)
{
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> query_ids;

  const auto evaluable_elem_range = _fe_problem.getEvaluableElementRange();

  std::vector<dof_id_type> evaluable_elem_ids;
  for (const auto & elem : evaluable_elem_range)
    if (hasBlocks(elem->subdomain_id())) // getEvaluableElementRange is not block restricted
      evaluable_elem_ids.push_back(elem->id());

  for (const auto & elem_id : evaluable_elem_ids)
  {
    const auto * elem = _mesh.elemPtr(elem_id);
    if (elem->processor_id() != processor_id())
      query_ids[elem->processor_id()].push_back(elem_id);
  }

  for (const auto & entry : additional_elems)
  {
    const Elem * elem = _mesh.elemPtr(entry);
    if (elem->processor_id() != processor_id())
      query_ids[elem->processor_id()].push_back(elem->id());
  }

  return query_ids;
}

void
NodalPatchRecoveryBase::sync(const std::vector<dof_id_type> & additional_elems)
{
  const auto query_ids = gatherSendList(additional_elems);

  typedef std::pair<RealEigenMatrix, RealEigenVector> AbPair;

  // Answer queries received from other processors
  auto gather_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            std::vector<AbPair> & ab_pairs)
  {
    for (const auto & elem_id : elem_ids)
      ab_pairs.emplace_back(_Ae.at(elem_id), _be.at(elem_id));
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

  // This is in sync now
  _synced = true;
}
