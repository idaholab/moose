//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  // Before we go, check if we have enough sample points for solving the least square fitting
  if (_q_point.size() * elem_ids.size() < _q)
    mooseError("There are not enough sample points to recover the nodal value, try reducing the "
               "polynomial order or using a higher-order quadrature scheme.");

  // Assemble the least squares problem over the patch
  RealEigenMatrix A = RealEigenMatrix::Zero(_q, _q);
  RealEigenVector b = RealEigenVector::Zero(_q);
  for (auto elem_id : elem_ids)
  {
    A += _Ae.at(elem_id);
    b += _be.at(elem_id);
  }

  // Solve the least squares fitting
  RealEigenVector coef = A.completeOrthogonalDecomposition().solve(b);

  // Compute the fitted nodal value
  RealEigenVector p = evaluateBasisFunctions(x);
  return p.dot(coef);
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
NodalPatchRecoveryBase::finalize()
{
  // When calling nodalPatchRecovery, we may need to know _Ae and _be on algebraically ghosted
  // elements. However, this userobject is only run on local elements, so we need to query those
  // information from other processors in this finalize() method.

  // Populate algebraically ghosted elements to query
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> query_ids;
  const ConstElemRange evaluable_elem_range = _fe_problem.getEvaluableElementRange();
  for (auto elem : evaluable_elem_range)
    if (elem->processor_id() != processor_id())
      query_ids[elem->processor_id()].push_back(elem->id());

  // Answer queries received from other processors
  auto gather_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            std::vector<std::vector<Real>> & data_to_fill)
  {
    data_to_fill.resize(elem_ids.size());
    for (const auto i : index_range(elem_ids))
    {
      const auto elem_id = elem_ids[i];
      auto Ait = _Ae.find(elem_id);
      if (Ait != _Ae.end())
      {
        auto bit = _be.find(elem_id);
        mooseAssert(bit != _be.end(), "We found _Ae[elem_id] but not _be[elem_id]");
        data_to_fill[i].insert(
            data_to_fill[i].end(), Ait->second.data(), Ait->second.data() + _q * _q);
        data_to_fill[i].insert(data_to_fill[i].end(), bit->second.data(), bit->second.data() + _q);
      }
    }
  };

  // Gather answers received from other processors
  auto act_on_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            const std::vector<std::vector<Real>> & filled_data)
  {
    for (const auto i : index_range(elem_ids))
    {
      const auto elem_id = elem_ids[i];
      if (!filled_data[i].empty())
      {
        RealEigenMatrix Ae(_q, _q);
        RealEigenVector be(_q);
        std::copy(filled_data[i].begin(), filled_data[i].begin() + _q * _q, Ae.data());
        std::copy(filled_data[i].begin() + _q * _q, filled_data[i].end(), be.data());
        _Ae[elem_id] = Ae;
        _be[elem_id] = be;
      }
    }
  };

  const std::vector<Real> * ex = nullptr;
  libMesh::Parallel::pull_parallel_vector_data(
      _communicator, query_ids, gather_data, act_on_data, ex);
}
