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
  params.addClassDescription("Represents vector field variables, e.g. Vector Lagrange or Nedelec");
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
Moose::VarFieldType
MooseVariableFE<OutputType>::fieldType() const
{
  if (std::is_same<OutputType, Real>::value)
    return Moose::VarFieldType::VAR_FIELD_STANDARD;
  else if (std::is_same<OutputType, RealVectorValue>::value)
    return Moose::VarFieldType::VAR_FIELD_VECTOR;
  else if (std::is_same<OutputType, RealEigenVector>::value)
    return Moose::VarFieldType::VAR_FIELD_ARRAY;
  else
    mooseError("Unknown variable field type");
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
MooseVariableFE<OutputType>::insert(NumericVector<Number> & residual)
{
  _element_data->insert(residual);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::add(NumericVector<Number> & residual)
{
  _element_data->add(residual);
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
MooseVariableFE<OutputType>::insertNodalValue(NumericVector<Number> & residual,
                                              const OutputData & v)
{
  _element_data->insertNodalValue(residual, v);
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::isArray() const
{
  return std::is_same<OutputType, RealEigenVector>::value;
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::isVector() const
{
  return std::is_same<OutputType, RealVectorValue>::value;
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

template class MooseVariableFE<Real>;
template class MooseVariableFE<RealVectorValue>;
template class MooseVariableFE<RealEigenVector>;
