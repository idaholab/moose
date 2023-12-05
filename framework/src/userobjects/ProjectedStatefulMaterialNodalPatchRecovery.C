//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectedStatefulMaterialNodalPatchRecovery.h"
#include "ElementUserObject.h"
#include "MaterialBase.h"
#include "MathUtils.h"
#include "Assembly.h"

// TIMPI includes
#include "timpi/communicator.h"
#include "timpi/parallel_sync.h"
#include "libmesh/parallel_eigen.h"

registerMooseObject("MooseApp", ProjectedStatefulMaterialNodalPatchRecoveryReal);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialNodalPatchRecoveryReal);
registerMooseObject("MooseApp", ProjectedStatefulMaterialNodalPatchRecoveryRealVectorValue);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialNodalPatchRecoveryRealVectorValue);
registerMooseObject("MooseApp", ProjectedStatefulMaterialNodalPatchRecoveryRankTwoTensor);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialNodalPatchRecoveryRankTwoTensor);
registerMooseObject("MooseApp", ProjectedStatefulMaterialNodalPatchRecoveryRankFourTensor);
registerMooseObject("MooseApp", ADProjectedStatefulMaterialNodalPatchRecoveryRankFourTensor);

InputParameters
ProjectedStatefulMaterialNodalPatchRecoveryBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription(
      "Prepare patches for use in nodal patch recovery based on a material property for material "
      "property states projected onto nodal variables.");
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 2; });

  return params;
}

template <typename T, bool is_ad>
InputParameters
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::validParams()
{
  InputParameters params = ProjectedStatefulMaterialNodalPatchRecoveryBase::validParams();
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH");
  params.addRequiredParam<MaterialPropertyName>(
      "property", "Name of the material property to perform nodal patch recovery on");
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

template <typename T, bool is_ad>
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::
    ProjectedStatefulMaterialNodalPatchRecoveryTempl(const InputParameters & parameters)
  : ProjectedStatefulMaterialNodalPatchRecoveryBase(parameters),
    _qp(0),
    _n_components(Moose::SerialAccess<T>::size()),
    _patch_polynomial_order(
        static_cast<unsigned int>(getParam<MooseEnum>("patch_polynomial_order"))),
    _multi_index(MathUtils::multiIndex(_mesh.dimension(), _patch_polynomial_order)),
    _q(_multi_index.size()),
    _prop(getGenericMaterialProperty<T, is_ad>("property")),
    _current_subdomain_id(_assembly.currentSubdomainID())
{
}

template <typename T, bool is_ad>
void
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::initialSetup()
{
  // get all material classes that provide properties for this object
  _required_materials = buildRequiredMaterials();
}

template <typename T, bool is_ad>
Real
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::nodalPatchRecovery(
    const Point & x, const std::vector<dof_id_type> & elem_ids, std::size_t component) const
{
  // Before we go, check if we have enough sample points for solving the least square fitting
  if (_q_point.size() * elem_ids.size() < _q)
    mooseError("There are not enough sample points to recover the nodal value, try reducing the "
               "polynomial order or using a higher-order quadrature scheme.");

  // Assemble the least squares problem over the patch
  RealEigenMatrix A = RealEigenMatrix::Zero(_q, _q);
  RealEigenVector b = RealEigenVector::Zero(_q);
  for (const auto elem_id : elem_ids)
  {
    const auto abs = libmesh_map_find(_abs, elem_id);
    A += abs.first;
    b += abs.second[component];
  }

  // Solve the least squares fitting
  const RealEigenVector coef = A.completeOrthogonalDecomposition().solve(b);

  // Compute the fitted nodal value
  const RealEigenVector p = evaluateBasisFunctions(x);
  return p.dot(coef);
}

template <typename T, bool is_ad>
RealEigenVector
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::evaluateBasisFunctions(
    const Point & q_point) const
{
  RealEigenVector p(_q);
  Real polynomial;
  for (const auto r : index_range(_multi_index))
  {
    polynomial = 1.0;
    mooseAssert(_multi_index[r].size() == _mesh.dimension(), "Wrong multi-index size.");
    for (const auto c : index_range(_multi_index[r]))
      polynomial *= MathUtils::pow(q_point(c), _multi_index[r][c]);
    p(r) = polynomial;
  }
  return p;
}

template <typename T, bool is_ad>
void
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::initialize()
{
  _abs.clear();
}

template <typename T, bool is_ad>
void
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::execute()
{
  if (_t_step == 0)
    for (const auto & mat : _required_materials[_current_subdomain_id])
      mat->initStatefulProperties(_qrule->size());

  std::vector<RealEigenVector> bs(_n_components, RealEigenVector::Zero(_q));
  RealEigenMatrix Ae = RealEigenMatrix::Zero(_q, _q);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    RealEigenVector p = evaluateBasisFunctions(_q_point[_qp]);
    Ae += p * p.transpose();

    std::size_t index = 0;
    for (const auto & v : Moose::serialAccess(_prop[_qp]))
      bs[index++] += MetaPhysicL::raw_value(v) * p;
  }

  const dof_id_type elem_id = _current_elem->id();
  _abs[elem_id] = {Ae, bs};
}

template <typename T, bool is_ad>
void
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::threadJoin(const UserObject & uo)
{
  const auto & npr =
      static_cast<const ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad> &>(uo);
  _abs.insert(npr._abs.begin(), npr._abs.end());
}

template <typename T, bool is_ad>
void
ProjectedStatefulMaterialNodalPatchRecoveryTempl<T, is_ad>::finalize()
{
  // When calling nodalPatchRecovery, we may need to know _Ae and _be on algebraically ghosted
  // elements. However, this userobject is only run on local elements, so we need to query those
  // information from other processors in this finalize() method.

  // Populate algebraically ghosted elements to query
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> query_ids;
  const ConstElemRange evaluable_elem_range = _fe_problem.getEvaluableElementRange();
  for (const auto * const elem : evaluable_elem_range)
    if (elem->processor_id() != processor_id())
      query_ids[elem->processor_id()].push_back(elem->id());

  // Answer queries received from other processors
  auto gather_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            std::vector<ElementData> & abs_data)
  {
    for (const auto i : index_range(elem_ids))
      abs_data.emplace_back(libmesh_map_find(_abs, elem_ids[i]));
  };

  // Gather answers received from other processors
  auto act_on_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            const std::vector<ElementData> & abs_data)
  {
    for (const auto i : index_range(elem_ids))
      _abs[elem_ids[i]] = abs_data[i];
  };

  libMesh::Parallel::pull_parallel_vector_data<ElementData>(
      _communicator, query_ids, gather_data, act_on_data, 0);
}

template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<Real, false>;
template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<Real, true>;
template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RealVectorValue, false>;
template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RealVectorValue, true>;
template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankTwoTensor, false>;
template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankTwoTensor, true>;
template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankFourTensor, false>;
template class ProjectedStatefulMaterialNodalPatchRecoveryTempl<RankFourTensor, true>;
