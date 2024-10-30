//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableDataLinearFV.h"
#include "MooseVariableDataFV.h"
#include "MooseVariableField.h"
#include "Assembly.h"
#include "MooseError.h"
#include "DisplacedSystem.h"
#include "TimeIntegrator.h"
#include "MooseVariableFV.h"
#include "MooseTypes.h"
#include "MooseMesh.h"
#include "Attributes.h"
#include "FVDirichletBCBase.h"
#include "SubProblem.h"
#include "FVKernel.h"
#include "ADUtils.h"

#include "libmesh/quadrature.h"
#include "libmesh/fe_base.h"
#include "libmesh/system.h"
#include "libmesh/type_n_tensor.h"

template <typename OutputType>
MooseVariableDataLinearFV<OutputType>::MooseVariableDataLinearFV(
    const MooseLinearVariableFV<OutputType> & var,
    SystemBase & sys,
    THREAD_ID tid,
    Moose::ElementType element_type,
    const Elem * const & elem)
  : MooseVariableDataBase<OutputType>(var, sys, tid),
    _var(var),
    _fe_type(_var.feType()),
    _var_num(_var.number()),
    _assembly(_subproblem.assembly(_tid, var.kind() == Moose::VAR_SOLVER ? sys.number() : 0)),
    _element_type(element_type),
    _time_integrator(_sys.queryTimeIntegrator(_var_num)),
    _elem(elem),
    _displaced(dynamic_cast<const DisplacedSystem *>(&_sys) ? true : false),
    _qrule(nullptr)
{
}

template <typename OutputType>
void
MooseVariableDataLinearFV<OutputType>::setGeometry(Moose::GeometryType gm_type)
{
  switch (gm_type)
  {
    case Moose::Volume:
    {
      _qrule = _assembly.qRule();
      // TODO: set integration multiplier to cell volume
      break;
    }
    case Moose::Face:
    {
      _qrule = _assembly.qRuleFace();
      // TODO: set integration multiplier to face area
      break;
    }
  }
}

template <typename OutputType>
void
MooseVariableDataLinearFV<OutputType>::initializeSolnVars()
{
  auto && active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);
  mooseAssert(_qrule, "We should have a non-null qrule");
  const auto nqp = _qrule->n_points();

  for (auto tag : _required_vector_tags)
    if (_need_vector_tag_u[tag])
    {
      _vector_tag_u[tag].resize(nqp);
      assignForAllQps(0, _vector_tag_u[tag], nqp);
    }

  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
    {
      _matrix_tag_u[tag].resize(nqp);
      assignForAllQps(0, _matrix_tag_u[tag], nqp);
    }
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseVariableDataLinearFV<OutputType>::initDofIndices()
{
  Moose::initDofIndices(*this, *_elem);
  return _dof_indices;
}

template <typename OutputType>
void
MooseVariableDataLinearFV<OutputType>::computeValues()
{
  initDofIndices();
  initializeSolnVars();

  unsigned int num_dofs = _dof_indices.size();

  if (num_dofs > 0)
    fetchDoFValues();
  else
    // We don't have any dofs. There's nothing to do
    return;

  mooseAssert(num_dofs == 1 && _vector_tags_dof_u[_solution_tag].size() == 1,
              "There should only be one dof per elem for FV variables");

  const auto nqp = _qrule->n_points();
  auto && active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);

  for (const auto qp : make_range(nqp))
  {
    for (auto tag : _required_vector_tags)
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][qp] = _vector_tags_dof_u[tag][0];

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][qp] = _matrix_tags_dof_u[tag][0];
  }
}

template <typename OutputType>
void
MooseVariableDataLinearFV<OutputType>::setDofValue(const OutputData & value, unsigned int index)
{
  mooseAssert(index == 0, "We only ever have one dof value locally");
  _vector_tags_dof_u[_solution_tag][index] = value;
  _has_dof_values = true;

  auto & u = _vector_tag_u[_solution_tag];
  // Update the qp values as well
  for (const auto qp : index_range(u))
    u[qp] = value;
}

template <typename OutputType>
void
MooseVariableDataLinearFV<OutputType>::setDofValues(const DenseVector<OutputData> & values)
{
  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  for (unsigned int i = 0; i < values.size(); i++)
    dof_values[i] = values(i);
  _has_dof_values = true;
}

template <typename OutputType>
void
MooseVariableDataLinearFV<OutputType>::getDofIndices(const Elem * elem,
                                                     std::vector<dof_id_type> & dof_indices) const
{
  _dof_map.dof_indices(elem, dof_indices, _var_num);
}

template class MooseVariableDataLinearFV<Real>;
// TODO: implement vector fv variable support. This will require some template
// specializations for various member functions in this and the FV variable
// classes. And then you will need to uncomment out the line below:
// template class MooseVariableDataLinearFV<RealVectorValue>;
