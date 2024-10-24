//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableFE.h"
#include <typeinfo>
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"
#include "DisplacedSystem.h"
#include "Assembly.h"
#include "MooseVariableData.h"
#include "ArbitraryQuadrature.h"

#include "libmesh/quadrature_monomial.h"

using namespace libMesh;

template <>
InputParameters
MooseVariableFE<Real>::validParams()
{
  auto params = MooseVariableField<Real>::validParams();
  params.addClassDescription(
      "Represents standard field variables, e.g. Lagrange, Hermite, or non-constant Monomials");
  return params;
}

template <>
InputParameters
MooseVariableFE<RealVectorValue>::validParams()
{
  auto params = MooseVariableField<RealVectorValue>::validParams();
  params.addClassDescription(
      "Represents vector field variables, e.g. Vector Lagrange, Nedelec or Raviart-Thomas");
  return params;
}

template <>
InputParameters
MooseVariableFE<RealEigenVector>::validParams()
{
  auto params = MooseVariableField<RealEigenVector>::validParams();
  params.addClassDescription(
      "Used for grouping standard field variables with the same finite element family and order");
  return params;
}

template <typename OutputType>
MooseVariableFE<OutputType>::MooseVariableFE(const InputParameters & parameters)
  : MooseVariableField<OutputType>(parameters)
{
  _element_data = std::make_unique<MooseVariableData<OutputType>>(*this,
                                                                  _sys,
                                                                  _tid,
                                                                  Moose::ElementType::Element,
                                                                  this->_assembly.qRule(),
                                                                  this->_assembly.qRuleFace(),
                                                                  this->_assembly.node(),
                                                                  this->_assembly.elem());
  _neighbor_data = std::make_unique<MooseVariableData<OutputType>>(
      *this,
      _sys,
      _tid,
      Moose::ElementType::Neighbor,
      this->_assembly.qRuleNeighbor(), // Place holder
      this->_assembly.qRuleNeighbor(),
      this->_assembly.nodeNeighbor(),
      this->_assembly.neighbor());
  _lower_data =
      std::make_unique<MooseVariableData<OutputType>>(*this,
                                                      _sys,
                                                      _tid,
                                                      Moose::ElementType::Lower,
                                                      this->_assembly.qRuleFace(),
                                                      this->_assembly.qRuleFace(), // Place holder
                                                      this->_assembly.node(),      // Place holder
                                                      this->_assembly.lowerDElem());
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::clearDofIndices()
{
  _element_data->clearDofIndices();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepare()
{
  _element_data->prepare();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareNeighbor()
{
  _neighbor_data->prepare();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareLowerD()
{
  _lower_data->prepare();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareAux()
{
  _element_data->prepareAux();
  _neighbor_data->prepareAux();
  _lower_data->prepareAux();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitNode()
{
  _element_data->reinitNode();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitAux()
{
  _element_data->reinitAux();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitAuxNeighbor()
{
  _neighbor_data->reinitAux();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitNodes(const std::vector<dof_id_type> & nodes)
{
  _element_data->reinitNodes(nodes);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::reinitNodesNeighbor(const std::vector<dof_id_type> & nodes)
{
  _neighbor_data->reinitNodes(nodes);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::getDofIndices(const Elem * elem,
                                           std::vector<dof_id_type> & dof_indices) const
{
  _element_data->getDofIndices(elem, dof_indices);
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::OutputData
MooseVariableFE<OutputType>::getNodalValue(const Node & node) const
{
  return _element_data->getNodalValue(node, Moose::Current);
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::OutputData
MooseVariableFE<OutputType>::getNodalValueOld(const Node & node) const
{
  return _element_data->getNodalValue(node, Moose::Old);
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::OutputData
MooseVariableFE<OutputType>::getNodalValueOlder(const Node & node) const
{
  return _element_data->getNodalValue(node, Moose::Older);
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::OutputData
MooseVariableFE<OutputType>::getElementalValue(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Current, idx);
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::OutputData
MooseVariableFE<OutputType>::getElementalValueOld(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Old, idx);
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::OutputData
MooseVariableFE<OutputType>::getElementalValueOlder(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Older, idx);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::insert(NumericVector<Number> & vector)
{
  _element_data->insert(vector);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::insertLower(NumericVector<Number> & vector)
{
  _lower_data->insert(vector);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::add(NumericVector<Number> & vector)
{
  _element_data->add(vector);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::addSolution(const DenseVector<Number> & v)
{
  _element_data->addSolution(this->_sys.solution(), v);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::addSolutionNeighbor(const DenseVector<Number> & v)
{
  _neighbor_data->addSolution(this->_sys.solution(), v);
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValue() const
{
  mooseDeprecated("Use dofValues instead of dofValue");
  return dofValues();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValues() const
{
  return _element_data->dofValues();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesOld() const
{
  return _element_data->dofValuesOld();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesOlder() const
{
  return _element_data->dofValuesOlder();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesPreviousNL() const
{
  return _element_data->dofValuesPreviousNL();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesNeighbor() const
{
  return _neighbor_data->dofValues();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesOldNeighbor() const
{
  return _neighbor_data->dofValuesOld();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesOlderNeighbor() const
{
  return _neighbor_data->dofValuesOlder();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesPreviousNLNeighbor() const
{
  return _neighbor_data->dofValuesPreviousNL();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDot() const
{
  return _element_data->dofValuesDot();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDotDot() const
{
  return _element_data->dofValuesDotDot();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDotOld() const
{
  return _element_data->dofValuesDotOld();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDotDotOld() const
{
  return _element_data->dofValuesDotDotOld();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDotNeighbor() const
{
  return _neighbor_data->dofValuesDot();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDotDotNeighbor() const
{
  return _neighbor_data->dofValuesDotDot();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDotOldNeighbor() const
{
  return _neighbor_data->dofValuesDotOld();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::dofValuesDotDotOldNeighbor() const
{
  return _neighbor_data->dofValuesDotDotOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDu() const
{
  return _element_data->dofValuesDuDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDotDu() const
{
  return _element_data->dofValuesDuDotDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDuNeighbor() const
{
  return _neighbor_data->dofValuesDuDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDotDuNeighbor() const
{
  return _neighbor_data->dofValuesDuDotDotDu();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::prepareIC()
{
  _element_data->prepareIC();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeElemValues()
{
  _element_data->setGeometry(Moose::Volume);
  _element_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeElemValuesFace()
{
  _element_data->setGeometry(Moose::Face);
  _element_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNeighborValuesFace()
{
  _neighbor_data->setGeometry(Moose::Face);
  _neighbor_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNeighborValues()
{
  _neighbor_data->setGeometry(Moose::Volume);
  _neighbor_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeLowerDValues()
{
  _lower_data->setGeometry(Moose::Volume);
  _lower_data->computeValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeIncrementAtQps(const NumericVector<Number> & increment_vec)
{
  _element_data->computeIncrementAtQps(increment_vec);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeIncrementAtNode(const NumericVector<Number> & increment_vec)
{
  _element_data->computeIncrementAtNode(increment_vec);
}

template <typename OutputType>
OutputType
MooseVariableFE<OutputType>::getValue(const Elem * elem,
                                      const std::vector<std::vector<OutputShape>> & phi) const
{
  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);

  OutputType value = 0;
  if (isNodal())
  {
    mooseAssert(dof_indices.size() == phi.size(),
                "The number of shapes does not match the number of dof indices on the elem");

    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += phi[i][0] * (*this->_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = (*this->_sys.currentSolution())(dof_indices[0]);
  }

  return value;
}

template <>
RealEigenVector
MooseVariableFE<RealEigenVector>::getValue(const Elem * elem,
                                           const std::vector<std::vector<Real>> & phi) const
{
  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);

  RealEigenVector value(_count);
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
      for (unsigned int j = 0; j < _count; j++)
      {
        // The zero index is because we only have one point that the phis are evaluated at
        value(j) += phi[i][0] * (*this->_sys.currentSolution())(dof_indices[i] + j);
      }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    unsigned int n = 0;
    for (unsigned int j = 0; j < _count; j++)
    {
      value(j) = (*this->_sys.currentSolution())(dof_indices[0] + n);
      n += this->_dof_indices.size();
    }
  }

  return value;
}

template <typename OutputType>
typename OutputTools<OutputType>::OutputGradient
MooseVariableFE<OutputType>::getGradient(
    const Elem * elem,
    const std::vector<std::vector<typename OutputTools<OutputType>::OutputShapeGradient>> &
        grad_phi) const
{
  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);

  typename OutputTools<OutputType>::OutputGradient value;
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += grad_phi[i][0] * (*this->_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = 0.0;
  }

  return value;
}

template <>
RealVectorArrayValue
MooseVariableFE<RealEigenVector>::getGradient(
    const Elem * elem, const std::vector<std::vector<RealVectorValue>> & grad_phi) const
{
  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);

  RealVectorArrayValue value(_count, LIBMESH_DIM);
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
      for (unsigned int j = 0; j < _count; ++j)
        for (const auto k : make_range(Moose::dim))
        {
          // The zero index is because we only have one point that the phis are evaluated at
          value(j, k) += grad_phi[i][0](k) * (*this->_sys.currentSolution())(dof_indices[i] + j);
        }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
  }

  return value;
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValue() const
{
  return _element_data->nodalValue(Moose::Current);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueNeighbor() const
{
  return _neighbor_data->nodalValue(Moose::Current);
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::nodalVectorTagValue(TagID tag) const
{
  return _element_data->nodalVectorTagValue(tag);
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::DoFValue &
MooseVariableFE<OutputType>::nodalMatrixTagValue(TagID tag) const
{
  return _element_data->nodalMatrixTagValue(tag);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOld() const
{
  return _element_data->nodalValue(Moose::Old);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOldNeighbor() const
{
  return _neighbor_data->nodalValue(Moose::Old);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOlder() const
{
  return _element_data->nodalValue(Moose::Older);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOlderNeighbor() const
{
  return _neighbor_data->nodalValue(Moose::Older);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValuePreviousNL() const
{
  return _element_data->nodalValue(Moose::PreviousNL);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValuePreviousNLNeighbor() const
{
  return _neighbor_data->nodalValue(Moose::PreviousNL);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDot() const
{
  return _element_data->nodalValueDot();
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotDot() const
{
  return _element_data->nodalValueDotDot();
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotOld() const
{
  return _element_data->nodalValueDotOld();
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotDotOld() const
{
  return _element_data->nodalValueDotDotOld();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNodalValues()
{
  _element_data->computeNodalValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::computeNodalNeighborValues()
{
  _neighbor_data->computeNodalValues();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setNodalValue(const OutputType & value, unsigned int idx)
{
  _element_data->setNodalValue(value, idx);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setDofValue(const OutputData & value, unsigned int index)
{
  _element_data->setDofValue(value, index);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setDofValues(const DenseVector<OutputData> & values)
{
  _element_data->setDofValues(values);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setLowerDofValues(const DenseVector<OutputData> & values)
{
  _lower_data->setDofValues(values);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::insertNodalValue(NumericVector<Number> & residual,
                                              const OutputData & v)
{
  _element_data->insertNodalValue(residual, v);
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhi() const
{
  return _element_data->secondPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhi() const
{
  return _element_data->curlPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiDivergence &
MooseVariableFE<OutputType>::divPhi() const
{
  return _element_data->divPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiFace() const
{
  return _element_data->secondPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiFace() const
{
  return _element_data->curlPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiDivergence &
MooseVariableFE<OutputType>::divPhiFace() const
{
  return _element_data->divPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiNeighbor() const
{
  return _neighbor_data->secondPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiNeighbor() const
{
  return _neighbor_data->curlPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiDivergence &
MooseVariableFE<OutputType>::divPhiNeighbor() const
{
  return _neighbor_data->divPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiFaceNeighbor() const
{
  return _neighbor_data->secondPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiFaceNeighbor() const
{
  return _neighbor_data->curlPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiDivergence &
MooseVariableFE<OutputType>::divPhiFaceNeighbor() const
{
  return _neighbor_data->divPhiFace();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::usesSecondPhi() const
{
  return _element_data->usesSecondPhi();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::usesSecondPhiNeighbor() const
{
  return _neighbor_data->usesSecondPhi();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::computingCurl() const
{
  return _element_data->computingCurl();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::computingDiv() const
{
  return _element_data->computingDiv();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::isNodalDefined() const
{
  return _element_data->isNodalDefined();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::isNodalNeighborDefined() const
{
  return _neighbor_data->isNodalDefined();
}

template <typename OutputType>
unsigned int
MooseVariableFE<OutputType>::oldestSolutionStateRequested() const
{
  unsigned int state = 0;
  state = std::max(state, _element_data->oldestSolutionStateRequested());
  state = std::max(state, _neighbor_data->oldestSolutionStateRequested());
  state = std::max(state, _lower_data->oldestSolutionStateRequested());
  return state;
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::clearAllDofIndices()
{
  _element_data->clearDofIndices();
  _neighbor_data->clearDofIndices();
  _lower_data->clearDofIndices();
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::ValueType
MooseVariableFE<OutputType>::evaluate(const NodeArg & node_arg, const StateArg & state) const
{
  mooseAssert(node_arg.node, "Must have a node");
  mooseAssert(this->hasBlocks(node_arg.subdomain_id),
              "Our variable should be defined on the requested subdomain ID");
  const Node & node = *node_arg.node;
  mooseAssert(node.n_dofs(this->_sys.number(), this->number()),
              "Our variable must have dofs on the requested node");
  const auto & soln = this->getSolution(state);
  if constexpr (std::is_same<OutputType, Real>::value)
  {
    const auto dof_number = node.dof_number(this->_sys.number(), this->number(), 0);
    ValueType ret = soln(dof_number);
    if (Moose::doDerivatives(_subproblem, _sys))
      Moose::derivInsert(ret.derivatives(), dof_number, 1);
    return ret;
  }
  else if constexpr (std::is_same<OutputType, RealVectorValue>::value)
  {
    ValueType ret;
    const auto do_derivatives = Moose::doDerivatives(_subproblem, _sys);
    for (const auto d : make_range(this->_mesh.dimension()))
    {
      const auto dof_number = node.dof_number(this->_sys.number(), this->number(), d);
      auto & component = ret(d);
      component = soln(dof_number);
      if (do_derivatives)
        Moose::derivInsert(component.derivatives(), dof_number, 1);
    }
    return ret;
  }
  else
    mooseError("RealEigenVector not yet supported for functors");
}

namespace
{
template <typename OutputType>
struct FEBaseHelper
{
  typedef FEBase type;
};

template <>
struct FEBaseHelper<RealVectorValue>
{
  typedef FEVectorBase type;
};
}

template <typename OutputType>
template <typename Shapes, typename Solution, typename GradShapes, typename GradSolution>
void
MooseVariableFE<OutputType>::computeSolution(const Elem * const elem,
                                             const unsigned int n_qp,
                                             const StateArg & state,
                                             const Shapes & phi,
                                             Solution & local_soln,
                                             const GradShapes & grad_phi,
                                             GradSolution & grad_local_soln,
                                             Solution & dot_local_soln,
                                             GradSolution & grad_dot_local_soln) const
{
  std::vector<dof_id_type> dof_indices;
  this->_dof_map.dof_indices(elem, dof_indices, _var_num);
  std::vector<ADReal> dof_values;
  std::vector<ADReal> dof_values_dot;
  dof_values.reserve(dof_indices.size());

  const bool computing_dot = _time_integrator && _time_integrator->dt();
  if (computing_dot)
    dof_values_dot.reserve(dof_indices.size());

  const bool do_derivatives = Moose::doDerivatives(_subproblem, _sys);
  const auto & global_soln = getSolution(state);
  for (const auto dof_index : dof_indices)
  {
    dof_values.push_back(ADReal(global_soln(dof_index)));
    if (do_derivatives && state.state == 0)
      Moose::derivInsert(dof_values.back().derivatives(), dof_index, 1.);
    if (computing_dot)
    {
      if (_var_kind == Moose::VAR_SOLVER)
      {
        dof_values_dot.push_back(dof_values.back());
        _time_integrator->computeADTimeDerivatives(
            dof_values_dot.back(), dof_index, _ad_real_dummy);
      }
      else
        dof_values_dot.push_back((*this->_sys.solutionUDot())(dof_index));
    }
  }

  local_soln.resize(n_qp);
  grad_local_soln.resize(n_qp);
  if (computing_dot)
  {
    dot_local_soln.resize(n_qp);
    grad_dot_local_soln.resize(n_qp);
  }

  for (const auto qp : make_range(n_qp))
  {
    local_soln[qp] = 0;
    grad_local_soln[qp] = 0;
    if (computing_dot)
    {
      dot_local_soln[qp] = 0;
      grad_dot_local_soln[qp] = GradientType{};
    }
    for (const auto i : index_range(dof_indices))
    {
      local_soln[qp] += dof_values[i] * phi[i][qp];
      grad_local_soln[qp] += dof_values[i] * grad_phi[i][qp];
      if (computing_dot)
      {
        dot_local_soln[qp] += dof_values_dot[i] * phi[i][qp];
        grad_dot_local_soln[qp] += dof_values_dot[i] * grad_phi[i][qp];
      }
    }
  }
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::evaluateOnElement(const ElemQpArg & elem_qp,
                                               const StateArg & state,
                                               const bool cache_eligible) const
{
  mooseAssert(this->hasBlocks(elem_qp.elem->subdomain_id()),
              "Variable " + this->name() + " doesn't exist on block " +
                  std::to_string(elem_qp.elem->subdomain_id()));

  const Elem * const elem = elem_qp.elem;
  if (!cache_eligible || (elem != _current_elem_qp_functor_elem))
  {
    const QBase * const qrule_template = elem_qp.qrule;

    using FEBaseType = typename FEBaseHelper<OutputType>::type;
    std::unique_ptr<FEBaseType> fe(FEBaseType::build(elem->dim(), _fe_type));
    auto qrule = qrule_template->clone();

    const auto & phi = fe->get_phi();
    const auto & dphi = fe->get_dphi();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem);

    computeSolution(elem,
                    qrule->n_points(),
                    state,
                    phi,
                    _current_elem_qp_functor_sln,
                    dphi,
                    _current_elem_qp_functor_gradient,
                    _current_elem_qp_functor_dot,
                    _current_elem_qp_functor_grad_dot);
  }
  if (cache_eligible)
    _current_elem_qp_functor_elem = elem;
  else
    // These evaluations are not eligible for caching, e.g. maybe this is a single point quadrature
    // rule evaluation at an arbitrary point and we don't want those evaluations to potentially be
    // re-used when this function is called with a standard quadrature rule or a different point
    _current_elem_qp_functor_elem = nullptr;
}

template <>
void
MooseVariableFE<RealEigenVector>::evaluateOnElement(const ElemQpArg &, const StateArg &, bool) const
{
  mooseError("evaluate not implemented for array variables");
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::ValueType
MooseVariableFE<OutputType>::evaluate(const ElemQpArg & elem_qp, const StateArg & state) const
{
  evaluateOnElement(elem_qp, state, /*query_cache=*/true);
  const auto qp = elem_qp.qp;
  mooseAssert(qp < _current_elem_qp_functor_sln.size(),
              "The requested " << qp << " is outside our solution size");
  return _current_elem_qp_functor_sln[qp];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::ValueType
MooseVariableFE<OutputType>::evaluate(const ElemArg & elem_arg, const StateArg & state) const
{
  const QMonomial qrule(elem_arg.elem->dim(), CONSTANT);
  // We can use whatever we want for the point argument since it won't be used
  const ElemQpArg elem_qp_arg{elem_arg.elem, /*qp=*/0, &qrule, Point(0, 0, 0)};
  evaluateOnElement(elem_qp_arg, state, /*cache_eligible=*/false);
  return _current_elem_qp_functor_sln[0];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::ValueType
MooseVariableFE<OutputType>::faceEvaluate(const FaceArg & face_arg,
                                          const StateArg & state,
                                          const std::vector<ValueType> & cache_data) const
{
  const QMonomial qrule(face_arg.fi->elem().dim() - 1, CONSTANT);
  auto side_evaluate =
      [this, &qrule, &state, &cache_data](const Elem * const elem, const unsigned int side)
  {
    // We can use whatever we want for the point argument since it won't be used
    const ElemSideQpArg elem_side_qp_arg{elem, side, /*qp=*/0, &qrule, Point(0, 0, 0)};
    evaluateOnElementSide(elem_side_qp_arg, state, /*cache_eligible=*/false);
    return cache_data[0];
  };

  const auto continuity = this->getContinuity();
  const bool on_elem = !face_arg.face_side || (face_arg.face_side == face_arg.fi->elemPtr());
  const bool on_neighbor =
      !face_arg.face_side || (face_arg.face_side == face_arg.fi->neighborPtr());
  if (on_neighbor)
    mooseAssert(
        face_arg.fi->neighborPtr(),
        "If we are signaling we should evaluate on the neighbor, we better have a neighbor");

  // Only do multiple evaluations if we are not continuous and we are on an internal face
  if ((continuity != C_ZERO && continuity != C_ONE) && on_elem && on_neighbor)
    return (side_evaluate(face_arg.fi->elemPtr(), face_arg.fi->elemSideID()) +
            side_evaluate(face_arg.fi->neighborPtr(), face_arg.fi->neighborSideID())) /
           2;
  else if (on_elem)
    return side_evaluate(face_arg.fi->elemPtr(), face_arg.fi->elemSideID());
  else if (on_neighbor)
    return side_evaluate(face_arg.fi->neighborPtr(), face_arg.fi->neighborSideID());
  else
    mooseError(
        "Attempted to evaluate a moose finite element variable on a face where it is not defined");
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::ValueType
MooseVariableFE<OutputType>::evaluate(const FaceArg & face_arg, const StateArg & state) const
{
  return faceEvaluate(face_arg, state, _current_elem_side_qp_functor_sln);
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::ValueType
MooseVariableFE<OutputType>::evaluate(const ElemPointArg & elem_point_arg,
                                      const StateArg & state) const
{
  mooseAssert(elem_point_arg.elem, "We need an Elem");
  const Elem & elem = *elem_point_arg.elem;
  const auto dim = elem.dim();
  ArbitraryQuadrature qrule(dim);
  const std::vector<Point> ref_point = {FEMap::inverse_map(dim, &elem, elem_point_arg.point)};
  qrule.setPoints(ref_point);
  // We can use whatever we want for the point argument since it won't be used
  const ElemQpArg elem_qp_arg{elem_point_arg.elem, /*qp=*/0, &qrule, elem_point_arg.point};
  evaluateOnElement(elem_qp_arg, state, /*cache_eligible=*/false);
  return _current_elem_qp_functor_sln[0];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::GradientType
MooseVariableFE<OutputType>::evaluateGradient(const ElemQpArg & elem_qp,
                                              const StateArg & state) const
{
  evaluateOnElement(elem_qp, state, /*query_cache=*/true);
  const auto qp = elem_qp.qp;
  mooseAssert(qp < _current_elem_qp_functor_gradient.size(),
              "The requested " << qp << " is outside our gradient size");
  return _current_elem_qp_functor_gradient[qp];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::GradientType
MooseVariableFE<OutputType>::evaluateGradient(const ElemArg & elem_arg,
                                              const StateArg & state) const
{
  const QMonomial qrule(elem_arg.elem->dim(), CONSTANT);
  // We can use whatever we want for the point argument since it won't be used
  const ElemQpArg elem_qp_arg{elem_arg.elem, /*qp=*/0, &qrule, Point(0, 0, 0)};
  evaluateOnElement(elem_qp_arg, state, /*cache_eligible=*/false);
  return _current_elem_qp_functor_gradient[0];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::DotType
MooseVariableFE<OutputType>::evaluateDot(const ElemQpArg & elem_qp, const StateArg & state) const
{
  mooseAssert(_time_integrator,
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");
  mooseAssert(_time_integrator->dt(),
              "A time derivative is being requested but the time integrator wants to perform a 0s "
              "time step");
  evaluateOnElement(elem_qp, state, /*query_cache=*/true);
  const auto qp = elem_qp.qp;
  mooseAssert(qp < _current_elem_qp_functor_dot.size(),
              "The requested " << qp << " is outside our dot size");
  return _current_elem_qp_functor_dot[qp];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::DotType
MooseVariableFE<OutputType>::evaluateDot(const ElemArg & elem_arg, const StateArg & state) const
{
  mooseAssert(_time_integrator,
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");
  mooseAssert(_time_integrator->dt(),
              "A time derivative is being requested but the time integrator wants to perform a 0s "
              "time step");
  const QMonomial qrule(elem_arg.elem->dim(), CONSTANT);
  // We can use whatever we want for the point argument since it won't be used
  const ElemQpArg elem_qp_arg{elem_arg.elem, /*qp=*/0, &qrule, Point(0, 0, 0)};
  evaluateOnElement(elem_qp_arg, state, /*cache_eligible=*/false);
  return _current_elem_qp_functor_dot[0];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::GradientType
MooseVariableFE<OutputType>::evaluateGradDot(const ElemArg & elem_arg, const StateArg & state) const
{
  mooseAssert(_time_integrator,
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");
  mooseAssert(_time_integrator->dt(),
              "A time derivative is being requested but the time integrator wants to perform a 0s "
              "time step");
  const QMonomial qrule(elem_arg.elem->dim(), CONSTANT);
  // We can use whatever we want for the point argument since it won't be used
  const ElemQpArg elem_qp_arg{elem_arg.elem, /*qp=*/0, &qrule, Point(0, 0, 0)};
  evaluateOnElement(elem_qp_arg, state, /*cache_eligible=*/false);
  return _current_elem_qp_functor_grad_dot[0];
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::evaluateOnElementSide(const ElemSideQpArg & elem_side_qp,
                                                   const StateArg & state,
                                                   const bool cache_eligible) const
{
  mooseAssert(this->hasBlocks(elem_side_qp.elem->subdomain_id()),
              "Variable " + this->name() + " doesn't exist on block " +
                  std::to_string(elem_side_qp.elem->subdomain_id()));

  const Elem * const elem = elem_side_qp.elem;
  const auto side = elem_side_qp.side;
  if (!cache_eligible || elem != _current_elem_side_qp_functor_elem_side.first ||
      side != _current_elem_side_qp_functor_elem_side.second)
  {
    const QBase * const qrule_template = elem_side_qp.qrule;

    using FEBaseType = typename FEBaseHelper<OutputType>::type;
    std::unique_ptr<FEBaseType> fe(FEBaseType::build(elem->dim(), _fe_type));
    auto qrule = qrule_template->clone();

    const auto & phi = fe->get_phi();
    const auto & dphi = fe->get_dphi();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem, side);

    computeSolution(elem,
                    qrule->n_points(),
                    state,
                    phi,
                    _current_elem_side_qp_functor_sln,
                    dphi,
                    _current_elem_side_qp_functor_gradient,
                    _current_elem_side_qp_functor_dot,
                    _current_elem_side_qp_functor_grad_dot);
  }
  if (cache_eligible)
    _current_elem_side_qp_functor_elem_side = std::make_pair(elem, side);
  else
    // These evaluations are not eligible for caching, e.g. maybe this is a single point quadrature
    // rule evaluation at an arbitrary point and we don't want those evaluations to potentially be
    // re-used when this function is called with a standard quadrature rule or a different point
    _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
}

template <>
void
MooseVariableFE<RealEigenVector>::evaluateOnElementSide(const ElemSideQpArg &,
                                                        const StateArg &,
                                                        bool) const
{
  mooseError("evaluate not implemented for array variables");
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::ValueType
MooseVariableFE<OutputType>::evaluate(const ElemSideQpArg & elem_side_qp,
                                      const StateArg & state) const
{
  evaluateOnElementSide(elem_side_qp, state, true);
  const auto qp = elem_side_qp.qp;
  mooseAssert(qp < _current_elem_side_qp_functor_sln.size(),
              "The requested " << qp << " is outside our solution size");
  return _current_elem_side_qp_functor_sln[qp];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::GradientType
MooseVariableFE<OutputType>::evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                              const StateArg & state) const
{
  evaluateOnElementSide(elem_side_qp, state, true);
  const auto qp = elem_side_qp.qp;
  mooseAssert(qp < _current_elem_side_qp_functor_gradient.size(),
              "The requested " << qp << " is outside our gradient size");
  return _current_elem_side_qp_functor_gradient[qp];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::DotType
MooseVariableFE<OutputType>::evaluateDot(const ElemSideQpArg & elem_side_qp,
                                         const StateArg & state) const
{
  mooseAssert(_time_integrator && _time_integrator->dt(),
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");
  evaluateOnElementSide(elem_side_qp, state, true);
  const auto qp = elem_side_qp.qp;
  mooseAssert(qp < _current_elem_side_qp_functor_dot.size(),
              "The requested " << qp << " is outside our dot size");
  return _current_elem_side_qp_functor_dot[qp];
}

template <typename OutputType>
typename MooseVariableFE<OutputType>::DotType
MooseVariableFE<OutputType>::evaluateDot(const FaceArg & face_arg, const StateArg & state) const
{
  mooseAssert(_time_integrator && _time_integrator->dt(),
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");
  return faceEvaluate(face_arg, state, _current_elem_side_qp_functor_dot);
}

template <>
typename MooseVariableFE<RealEigenVector>::ValueType
MooseVariableFE<RealEigenVector>::evaluate(const ElemQpArg &, const StateArg &) const
{
  mooseError(
      "MooseVariableFE::evaluate(ElemQpArg &, const StateArg &) overload not implemented for "
      "array variables");
}

template <>
typename MooseVariableFE<RealEigenVector>::ValueType
MooseVariableFE<RealEigenVector>::evaluate(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("MooseVariableFE::evaluate(ElemSideQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableFE<RealEigenVector>::GradientType
MooseVariableFE<RealEigenVector>::evaluateGradient(const ElemQpArg &, const StateArg &) const
{
  mooseError("MooseVariableFE::evaluateGradient(ElemQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableFE<RealEigenVector>::GradientType
MooseVariableFE<RealEigenVector>::evaluateGradient(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("MooseVariableFE::evaluateGradient(ElemSideQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableFE<RealEigenVector>::DotType
MooseVariableFE<RealEigenVector>::evaluateDot(const ElemQpArg &, const StateArg &) const
{
  mooseError("MooseVariableFE::evaluateDot(ElemQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableFE<RealEigenVector>::DotType
MooseVariableFE<RealEigenVector>::evaluateDot(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("MooseVariableFE::evaluateDot(ElemSideQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::meshChanged()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  MooseVariableField<OutputType>::meshChanged();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::residualSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  MooseVariableField<OutputType>::residualSetup();
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::jacobianSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  MooseVariableField<OutputType>::jacobianSetup();
}

template class MooseVariableFE<Real>;
template class MooseVariableFE<RealVectorValue>;
template class MooseVariableFE<RealEigenVector>;
