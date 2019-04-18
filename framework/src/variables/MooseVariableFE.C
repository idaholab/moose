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

template <typename OutputType>
MooseVariableFE<OutputType>::MooseVariableFE(unsigned int var_num,
                                             const FEType & fe_type,
                                             SystemBase & sys,
                                             const Assembly & assembly,
                                             Moose::VarKindType var_kind,
                                             THREAD_ID tid)
  : MooseVariableFEBase(var_num, fe_type, sys, var_kind, tid), _assembly(assembly)
{
  _element_data = libmesh_make_unique<MooseVariableData<OutputType>>(*this,
                                                                     sys,
                                                                     tid,
                                                                     Moose::ElementType::Element,
                                                                     _assembly.qRule(),
                                                                     _assembly.qRuleFace(),
                                                                     _assembly.node(),
                                                                     _assembly.elem());
  _neighbor_data =
      libmesh_make_unique<MooseVariableData<OutputType>>(*this,
                                                         sys,
                                                         tid,
                                                         Moose::ElementType::Neighbor,
                                                         _assembly.qRuleNeighbor(), // Place holder
                                                         _assembly.qRuleNeighbor(),
                                                         _assembly.nodeNeighbor(),
                                                         _assembly.neighbor());
  _lower_data =
      libmesh_make_unique<MooseVariableData<OutputType>>(*this,
                                                         sys,
                                                         tid,
                                                         Moose::ElementType::Lower,
                                                         _assembly.qRuleFace(),
                                                         _assembly.qRuleFace(), // Place holder
                                                         _assembly.node(),      // Place holder
                                                         _assembly.lowerDElem());
}

template <typename OutputType>
const std::set<SubdomainID> &
MooseVariableFE<OutputType>::activeSubdomains() const
{
  return _sys.system().variable(_var_num).active_subdomains();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::activeOnSubdomain(SubdomainID subdomain) const
{
  return _sys.system().variable(_var_num).active_on_subdomain(subdomain);
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
  _element_data->hasDofValues(false);
  _neighbor_data->hasDofValues(false);
  _lower_data->hasDofValues(false);
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
Number
MooseVariableFE<OutputType>::getNodalValue(const Node & node)
{
  return _element_data->getNodalValue(node, Moose::Current);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getNodalValueOld(const Node & node)
{
  return _element_data->getNodalValue(node, Moose::Old);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getNodalValueOlder(const Node & node)
{
  return _element_data->getNodalValue(node, Moose::Older);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getElementalValue(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Current, idx);
}

template <typename OutputType>
Number
MooseVariableFE<OutputType>::getElementalValueOld(const Elem * elem, unsigned int idx) const
{
  return _element_data->getElementalValue(elem, Moose::Old, idx);
}

template <typename OutputType>
Number
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
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValue()
{
  mooseDeprecated("Use dofValues instead of dofValue");
  return dofValues();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValues()
{
  return _element_data->dofValues();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOld()
{
  return _element_data->dofValuesOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOlder()
{
  return _element_data->dofValuesOlder();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesPreviousNL()
{
  return _element_data->dofValuesPreviousNL();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesNeighbor()
{
  return _neighbor_data->dofValues();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOldNeighbor()
{
  return _neighbor_data->dofValuesOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesOlderNeighbor()
{
  return _neighbor_data->dofValuesOlder();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesPreviousNLNeighbor()
{
  return _neighbor_data->dofValuesPreviousNL();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDot()
{
  return _element_data->dofValuesDot();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDot()
{
  return _element_data->dofValuesDotDot();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotOld()
{
  return _element_data->dofValuesDotOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDotOld()
{
  return _element_data->dofValuesDotDotOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotNeighbor()
{
  return _neighbor_data->dofValuesDot();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDotNeighbor()
{
  return _neighbor_data->dofValuesDotDot();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotOldNeighbor()
{
  return _neighbor_data->dofValuesDotOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDotDotOldNeighbor()
{
  return _neighbor_data->dofValuesDotDotOld();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDu()
{
  return _element_data->dofValuesDuDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDotDu()
{
  return _element_data->dofValuesDuDotDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDuNeighbor()
{
  return _neighbor_data->dofValuesDuDotDu();
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableFE<OutputType>::dofValuesDuDotDotDuNeighbor()
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
                                      const std::vector<std::vector<OutputType>> & phi) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  OutputType value = 0;
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = (*_sys.currentSolution())(dof_indices[0]);
  }

  return value;
}

template <typename OutputType>
typename OutputTools<OutputType>::OutputGradient
MooseVariableFE<OutputType>::getGradient(
    const Elem * elem,
    const std::vector<std::vector<typename OutputTools<OutputType>::OutputGradient>> & grad_phi)
    const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  typename OutputTools<OutputType>::OutputGradient value;
  if (isNodal())
  {
    for (unsigned int i = 0; i < dof_indices.size(); ++i)
    {
      // The zero index is because we only have one point that the phis are evaluated at
      value += grad_phi[i][0] * (*_sys.currentSolution())(dof_indices[i]);
    }
  }
  else
  {
    mooseAssert(dof_indices.size() == 1, "Wrong size for dof indices");
    value = 0.0;
  }

  return value;
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValue()
{
  return _element_data->nodalValue(Moose::Current);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueNeighbor()
{
  return _neighbor_data->nodalValue(Moose::Current);
}

template <typename OutputType>
const MooseArray<Real> &
MooseVariableFE<OutputType>::nodalVectorTagValue(TagID tag)
{
  return _element_data->nodalVectorTagValue(tag);
}

template <typename OutputType>
const MooseArray<Real> &
MooseVariableFE<OutputType>::nodalMatrixTagValue(TagID tag)
{
  return _element_data->nodalMatrixTagValue(tag);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOld()
{
  return _element_data->nodalValue(Moose::Old);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOldNeighbor()
{
  return _neighbor_data->nodalValue(Moose::Old);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOlder()
{
  return _element_data->nodalValue(Moose::Older);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueOlderNeighbor()
{
  return _neighbor_data->nodalValue(Moose::Older);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValuePreviousNL()
{
  return _element_data->nodalValue(Moose::PreviousNL);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValuePreviousNLNeighbor()
{
  return _neighbor_data->nodalValue(Moose::PreviousNL);
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDot()
{
  return _element_data->nodalValueDot();
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotDot()
{
  return _element_data->nodalValueDotDot();
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotOld()
{
  return _element_data->nodalValueDotOld();
}

template <typename OutputType>
const OutputType &
MooseVariableFE<OutputType>::nodalValueDotDotOld()
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
MooseVariableFE<OutputType>::setDofValues(const DenseVector<Number> & values)
{
  _element_data->setDofValues(values);
}

template <typename OutputType>
void
MooseVariableFE<OutputType>::setNodalValue(OutputType value, unsigned int idx /* = 0*/)
{
  _element_data->setNodalValue(value, idx);
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::isVector() const
{
  return std::is_same<OutputType, RealVectorValue>::value;
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhi()
{
  return _element_data->secondPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhi()
{
  return _element_data->curlPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiFace()
{
  return _element_data->secondPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiFace()
{
  return _element_data->curlPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiNeighbor()
{
  return _neighbor_data->secondPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiNeighbor()
{
  return _neighbor_data->curlPhi();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiSecond &
MooseVariableFE<OutputType>::secondPhiFaceNeighbor()
{
  return _neighbor_data->secondPhiFace();
}

template <typename OutputType>
const typename MooseVariableFE<OutputType>::FieldVariablePhiCurl &
MooseVariableFE<OutputType>::curlPhiFaceNeighbor()
{
  return _neighbor_data->curlPhiFace();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::usesSecondPhi()
{
  return _element_data->usesSecondPhi();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::usesSecondPhiNeighbor()
{
  return _neighbor_data->usesSecondPhi();
}

template <typename OutputType>
bool
MooseVariableFE<OutputType>::computingCurl()
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

template class MooseVariableFE<Real>;
template class MooseVariableFE<RealVectorValue>;
