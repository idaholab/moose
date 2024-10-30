//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableDataBase.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "SystemBase.h"
#include "SubProblem.h"
#include "MooseVariableField.h"

template <typename OutputType>
MooseVariableDataBase<OutputType>::MooseVariableDataBase(const MooseVariableField<OutputType> & var,
                                                         SystemBase & sys,
                                                         THREAD_ID tid)
  : _sys(sys),
    _subproblem(_sys.subproblem()),
    _tid(tid),
    _dof_map(_sys.dofMap()),
    _count(var.count()),
    _has_dof_values(false),
    _max_state(0),
    _solution_tag(_subproblem.getVectorTagID(Moose::SOLUTION_TAG)),
    _old_solution_tag(Moose::INVALID_TAG_ID),
    _older_solution_tag(Moose::INVALID_TAG_ID),
    _previous_nl_solution_tag(Moose::INVALID_TAG_ID),
    _need_u_dot(false),
    _need_u_dotdot(false),
    _need_u_dot_old(false),
    _need_u_dotdot_old(false),
    _need_du_dot_du(false),
    _need_du_dotdot_du(false),
    _need_grad_dot(false),
    _need_grad_dotdot(false),
    _need_dof_values_dot(false),
    _need_dof_values_dotdot(false),
    _need_dof_values_dot_old(false),
    _need_dof_values_dotdot_old(false),
    _need_dof_du_dot_du(false),
    _need_dof_du_dotdot_du(false),
    _var(var)
{
  auto num_vector_tags = _subproblem.numVectorTags();
  // Additional solution tags corresponding to older-than-current solution states may be requested
  // on-the-fly by consumer objects after variable construction. To accomodate this we should
  // reserve the possibly requisite memory. As of now we only support old and older solution states
  // but this could potentially increase in the future
  const auto max_future_num_vector_tags = num_vector_tags + 2;

  _vector_tags_dof_u.reserve(max_future_num_vector_tags);
  _vector_tags_dof_u.resize(num_vector_tags);
  _need_vector_tag_dof_u.reserve(max_future_num_vector_tags);
  _need_vector_tag_dof_u.resize(num_vector_tags, false);

  _need_vector_tag_u.reserve(max_future_num_vector_tags);
  _need_vector_tag_u.resize(num_vector_tags, false);
  _vector_tag_u.reserve(max_future_num_vector_tags);
  _vector_tag_u.resize(num_vector_tags);

  _need_vector_tag_grad.reserve(max_future_num_vector_tags);
  _need_vector_tag_grad.resize(num_vector_tags, false);
  _vector_tag_grad.reserve(max_future_num_vector_tags);
  _vector_tag_grad.resize(num_vector_tags);

  auto num_matrix_tags = _subproblem.numMatrixTags();

  _matrix_tags_dof_u.resize(num_matrix_tags);
  _need_matrix_tag_dof_u.resize(num_matrix_tags, false);

  _need_matrix_tag_u.resize(num_matrix_tags, false);
  _matrix_tag_u.resize(num_matrix_tags);

  // Always fetch the dof values for the solution tag
  const auto soln_tag = _subproblem.getVectorTagID(Moose::SOLUTION_TAG);
  _need_vector_tag_dof_u[soln_tag] = true;
  _need_vector_tag_u[soln_tag] = true;
  insertSolutionTag(soln_tag);

  // These MooseArray objects are used by AuxKernelBase for nodal AuxKernel objects, hence the size
  // size is always 1 (i.e, nodal kernels work with _qp=0 only).
  _nodal_value_array.resize(1);
  _nodal_value_old_array.resize(1);
  _nodal_value_older_array.resize(1);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::nodalVectorTagValue(TagID tag) const
{
  if (isNodal())
  {
    if (tag >= _need_vector_tag_dof_u.size())
      const_cast<MooseVariableDataBase<OutputType> *>(this)->resizeVectorTagData(tag);

    _need_vector_tag_dof_u[tag] = true;

    if (_sys.hasVector(tag))
      return _vector_tags_dof_u[tag];
    else
      mooseError(
          "Tag ", tag, " is not associated with any vector for nodal variable ", _var.name());
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::nodalMatrixTagValue(TagID tag) const
{
  if (isNodal())
  {
    if (tag >= _matrix_tags_dof_u.size())
    {
      _need_matrix_tag_dof_u.resize(tag + 1, false);
      const_cast<MooseVariableDataBase<OutputType> *>(this)->_matrix_tags_dof_u.resize(tag + 1);
    }

    _need_matrix_tag_dof_u[tag] = true;

    if (_sys.hasMatrix(tag))
      return _matrix_tags_dof_u[tag];
    else
      mooseError(
          "Tag ", tag, " is not associated with any matrix for nodal variable ", _var.name());
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::resizeVectorTagData(TagID tag)
{
  mooseAssert(_need_vector_tag_dof_u.size() == _need_vector_tag_u.size() &&
                  _need_vector_tag_dof_u.size() == _need_vector_tag_grad.size() &&
                  _need_vector_tag_dof_u.size() == _vector_tags_dof_u.size() &&
                  _need_vector_tag_dof_u.size() == _vector_tag_u.size() &&
                  _need_vector_tag_dof_u.size() == _vector_tag_grad.size(),
              "These sizes should be in sync.");

  auto check_capacity = [tag](const auto & vector_to_check)
  {
    if (tag + 1 > vector_to_check.capacity())
      mooseError("New size greater than tag capacity. This will cause reallocation which will "
                 "invalidate any stored references.");
  };
  check_capacity(_need_vector_tag_dof_u);
  check_capacity(_need_vector_tag_u);
  check_capacity(_need_vector_tag_grad);
  check_capacity(_vector_tags_dof_u);
  check_capacity(_vector_tag_u);
  check_capacity(_vector_tag_grad);

  _need_vector_tag_dof_u.resize(tag + 1, false);
  _need_vector_tag_u.resize(tag + 1, false);
  _need_vector_tag_grad.resize(tag + 1, false);
  _vector_tags_dof_u.resize(tag + 1);
  _vector_tag_u.resize(tag + 1);
  _vector_tag_grad.resize(tag + 1);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::FieldVariableValue &
MooseVariableDataBase<OutputType>::vectorTagValue(TagID tag) const
{
  if (tag >= _need_vector_tag_u.size())
    const_cast<MooseVariableDataBase<OutputType> *>(this)->resizeVectorTagData(tag);

  _need_vector_tag_u[tag] = true;

  if (_sys.hasVector(tag))
    return _vector_tag_u[tag];
  else
    mooseError("Tag ", tag, " is not associated with any vector for variable ", _var.name());
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::vectorTagDofValue(TagID tag) const
{
  if (tag >= _need_vector_tag_dof_u.size())
    const_cast<MooseVariableDataBase<OutputType> *>(this)->resizeVectorTagData(tag);

  _need_vector_tag_dof_u[tag] = true;

  if (_sys.hasVector(tag))
    return _vector_tags_dof_u[tag];
  else
    mooseError("Tag ", tag, " is not associated with any vector for variable ", _var.name());
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::FieldVariableGradient &
MooseVariableDataBase<OutputType>::vectorTagGradient(TagID tag) const
{
  if (tag >= _need_vector_tag_grad.size())
    const_cast<MooseVariableDataBase<OutputType> *>(this)->resizeVectorTagData(tag);

  _need_vector_tag_grad[tag] = true;

  if (_sys.hasVector(tag))
    return _vector_tag_grad[tag];
  else
    mooseError("Tag ", tag, " is not associated with any vector for variable ", _var.name());
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::FieldVariableValue &
MooseVariableDataBase<OutputType>::matrixTagValue(TagID tag) const
{
  if (tag >= _matrix_tag_u.size())
  {
    _need_matrix_tag_u.resize(tag + 1, false);
    const_cast<MooseVariableDataBase<OutputType> *>(this)->_matrix_tag_u.resize(tag + 1);
  }

  _need_matrix_tag_u[tag] = true;

  if (_sys.hasMatrix(tag))
    return _matrix_tag_u[tag];
  else
    mooseError("Tag ", tag, " is not associated with any matrix for variable ", _var.name());
}

template <typename OutputType>
unsigned int
MooseVariableDataBase<OutputType>::oldestSolutionStateRequested() const
{
  return _max_state;
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::needSolutionState(const unsigned int state)
{
  if (state > _max_state)
  {
    _sys.needSolutionState(state);
    _max_state = state;
  }
}

template <typename OutputType>
template <typename ReturnType, typename Functor>
const ReturnType &
MooseVariableDataBase<OutputType>::stateToTagHelper(const Moose::SolutionState state,
                                                    Functor functor)
{
  if (state > 0)
  {
    // We need to request all states that are between current and the requested state
    stateToTagHelper<ReturnType>(Moose::SolutionState(static_cast<int>(state) - 1), functor);
    needSolutionState(cast_int<unsigned int>(state));
  }

  switch (state)
  {
    case Moose::Current:
      return functor(_subproblem.getVectorTagID(Moose::SOLUTION_TAG));

    case Moose::Old:
    {
      _old_solution_tag = _subproblem.getVectorTagID(Moose::OLD_SOLUTION_TAG);
      insertSolutionTag(_old_solution_tag);
      return functor(_old_solution_tag);
    }

    case Moose::Older:
    {
      _older_solution_tag = _subproblem.getVectorTagID(Moose::OLDER_SOLUTION_TAG);
      insertSolutionTag(_older_solution_tag);
      return functor(_older_solution_tag);
    }

    case Moose::PreviousNL:
    {
      _previous_nl_solution_tag = _subproblem.getVectorTagID(Moose::PREVIOUS_NL_SOLUTION_TAG);
      insertSolutionTag(_previous_nl_solution_tag);
      return functor(_previous_nl_solution_tag);
    }

    default:
      // We should never get here but gcc requires that we have a default. See
      // htpps://stackoverflow.com/questions/18680378/after-defining-case-for-all-enum-values-compiler-still-says-control-reaches-e
      mooseError("Unknown SolutionState!");
  }
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::FieldVariableValue &
MooseVariableDataBase<OutputType>::sln(Moose::SolutionState state) const
{
  auto functor = [this](TagID tag_id) -> const FieldVariableValue &
  { return vectorTagValue(tag_id); };

  return const_cast<MooseVariableDataBase<OutputType> *>(this)
      ->stateToTagHelper<FieldVariableValue>(state, functor);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::FieldVariableGradient &
MooseVariableDataBase<OutputType>::gradSln(Moose::SolutionState state) const
{
  auto functor = [this](TagID tag_id) -> const FieldVariableGradient &
  { return vectorTagGradient(tag_id); };

  return const_cast<MooseVariableDataBase<OutputType> *>(this)
      ->stateToTagHelper<FieldVariableGradient>(state, functor);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::vectorTagDofValue(Moose::SolutionState state) const
{
  auto functor = [this](TagID tag_id) -> const DoFValue & { return vectorTagDofValue(tag_id); };

  return const_cast<MooseVariableDataBase<OutputType> *>(this)->stateToTagHelper<DoFValue>(state,
                                                                                           functor);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::dofValues() const
{
  return vectorTagDofValue(Moose::Current);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::dofValuesOld() const
{
  return vectorTagDofValue(Moose::Old);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::dofValuesOlder() const
{
  return vectorTagDofValue(Moose::Older);
}

template <typename OutputType>
const typename MooseVariableDataBase<OutputType>::DoFValue &
MooseVariableDataBase<OutputType>::dofValuesPreviousNL() const
{
  return vectorTagDofValue(Moose::PreviousNL);
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::setNodalValue(const OutputType & value, unsigned int idx)
{
  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  mooseAssert(idx < dof_values.size(), "idx is out of the bounds of degree of freedom values");
  dof_values[idx] = value; // update variable nodal value
  _has_dof_values = true;
  _nodal_value = value;

  // Update the qp values as well
  auto & u = _vector_tag_u[_solution_tag];
  for (unsigned int qp = 0; qp < u.size(); qp++)
    u[qp] = value;
}

template <>
void
MooseVariableDataBase<RealVectorValue>::setNodalValue(const RealVectorValue & value,
                                                      unsigned int idx)
{
  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  for (decltype(idx) i = 0; i < dof_values.size(); ++i, ++idx)
    dof_values[idx] = value(i);

  _has_dof_values = true;
  _nodal_value = value;
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::insert(NumericVector<Number> & residual)
{
  if (_has_dof_values)
  {
    auto & dof_values = _vector_tags_dof_u[_solution_tag];
    mooseAssert(dof_values.size() == _dof_indices.size(),
                "Degree of freedom values size and degree of freedom indices sizes must match.");
    residual.insert(&dof_values[0], _dof_indices);
  }
}

template <>
void
MooseVariableDataBase<RealEigenVector>::insert(NumericVector<Number> & residual)
{
  if (_has_dof_values)
  {
    auto & dof_values = _vector_tags_dof_u[_solution_tag];
    if (isNodal())
    {
      for (unsigned int i = 0; i < _dof_indices.size(); ++i)
        for (unsigned int j = 0; j < _count; ++j)
          residual.set(_dof_indices[i] + j, dof_values[i](j));
    }
    else
    {
      unsigned int n = 0;
      for (unsigned int j = 0; j < _count; ++j)
      {
        for (unsigned int i = 0; i < _dof_indices.size(); ++i)
          residual.set(_dof_indices[i] + n, dof_values[i](j));
        n += _dof_indices.size();
      }
    }
  }
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::add(NumericVector<Number> & residual)
{
  if (_has_dof_values)
  {
    auto & dof_values = _vector_tags_dof_u[_solution_tag];
    residual.add_vector(&dof_values[0], _dof_indices);
  }
}

template <>
void
MooseVariableDataBase<RealEigenVector>::add(NumericVector<Number> & residual)
{
  if (_has_dof_values)
  {
    auto & dof_values = _vector_tags_dof_u[_solution_tag];
    if (isNodal())
    {
      for (unsigned int i = 0; i < _dof_indices.size(); ++i)
        for (unsigned int j = 0; j < _count; ++j)
          residual.add(_dof_indices[i] + j, dof_values[i](j));
    }
    else
    {
      unsigned int n = 0;
      for (unsigned int j = 0; j < _count; ++j)
      {
        for (unsigned int i = 0; i < _dof_indices.size(); ++i)
          residual.add(_dof_indices[i] + n, dof_values[i](j));
        n += _dof_indices.size();
      }
    }
  }
}

template <typename OutputType>
const OutputType &
MooseVariableDataBase<OutputType>::nodalValue(Moose::SolutionState state) const
{
  if (isNodal())
  {
    // Request the correct solution states and data members
    vectorTagDofValue(state);
    switch (state)
    {
      case Moose::Current:
        return _nodal_value;

      case Moose::Old:
        return _nodal_value_old;

      case Moose::Older:
        return _nodal_value_older;

      case Moose::PreviousNL:
        return _nodal_value_previous_nl;

      default:
        // We should never get here but gcc requires that we have a default. See
        // htpps://stackoverflow.com/questions/18680378/after-defining-case-for-all-enum-values-compiler-still-says-control-reaches-e
        mooseError("Unknown SolutionState!");
    }
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
const MooseArray<OutputType> &
MooseVariableDataBase<OutputType>::nodalValueArray(Moose::SolutionState state) const
{
  if (isNodal())
  {
    // Request the correct solution states and data members
    vectorTagDofValue(state);
    switch (state)
    {
      case Moose::Current:
        return _nodal_value_array;

      case Moose::Old:
        return _nodal_value_old_array;

      case Moose::Older:
        return _nodal_value_older_array;

      default:
        mooseError("No current support for PreviousNL for nodal value array");
    }
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::fetchDoFValues()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  if (is_transient)
  {
    if (_need_u_dot || _need_grad_dot || _need_dof_values_dot)
    {
      libmesh_assert(_sys.solutionUDot());
      _dof_values_dot.resize(n);
      _sys.solutionUDot()->get(_dof_indices, &_dof_values_dot[0]);
    }
    if (_need_u_dotdot || _need_grad_dotdot || _need_dof_values_dotdot)
    {
      libmesh_assert(_sys.solutionUDotDot());
      _dof_values_dotdot.resize(n);
      _sys.solutionUDotDot()->get(_dof_indices, &_dof_values_dotdot[0]);
    }
    if (_need_u_dot_old || _need_dof_values_dot_old)
    {
      libmesh_assert(_sys.solutionUDotOld());
      _dof_values_dot_old.resize(n);
      _sys.solutionUDotOld()->get(_dof_indices, &_dof_values_dot_old[0]);
    }
    if (_need_u_dotdot_old || _need_dof_values_dotdot_old)
    {
      libmesh_assert(_sys.solutionUDotDotOld());
      _dof_values_dotdot_old.resize(n);
      _sys.solutionUDotDotOld()->get(_dof_indices, &_dof_values_dotdot_old[0]);
    }
  }

  for (auto tag : _required_vector_tags)
    if (_need_vector_tag_u[tag] || _need_vector_tag_grad[tag] || _need_vector_tag_dof_u[tag])
      if ((_subproblem.vectorTagType(tag) == Moose::VECTOR_TAG_RESIDUAL &&
           _subproblem.safeAccessTaggedVectors()) ||
          _subproblem.vectorTagType(tag) == Moose::VECTOR_TAG_SOLUTION)
        // tag is defined on problem but may not be used by a system
        // the grain tracker requires being able to read from solution vectors that we are also in
        // the process of writing :-/
        if (_sys.hasVector(tag) /* && _sys.getVector(tag).closed()*/)
        {
          auto & vec = _sys.getVector(tag);
          _vector_tags_dof_u[tag].resize(n);
          vec.get(_dof_indices, &_vector_tags_dof_u[tag][0]);
        }

  if (_subproblem.safeAccessTaggedMatrices())
  {
    auto & active_coupleable_matrix_tags =
        _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);
    for (auto tag : active_coupleable_matrix_tags)
    {
      _matrix_tags_dof_u[tag].resize(n);
      if (_need_matrix_tag_dof_u[tag] || _need_matrix_tag_u[tag])
        if (_sys.hasMatrix(tag) && _sys.matrixTagActive(tag) && _sys.getMatrix(tag).closed())
        {
          auto & mat = _sys.getMatrix(tag);
          for (unsigned i = 0; i < n; i++)
          {
            Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
            _matrix_tags_dof_u[tag][i] = mat(_dof_indices[i], _dof_indices[i]);
          }
        }
    }
  }

  if (_need_du_dot_du || _need_dof_du_dot_du)
  {
    _dof_du_dot_du.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dot_du[i] = _sys.duDotDu(_var.number());
  }
  if (_need_du_dotdot_du || _need_dof_du_dotdot_du)
  {
    _dof_du_dotdot_du.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dotdot_du[i] = _sys.duDotDotDu();
  }
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::getArrayDoFValues(const NumericVector<Number> & sol,
                                                     unsigned int n,
                                                     MooseArray<RealEigenVector> & dof_values) const
{
  dof_values.resize(n);
  if (isNodal())
  {
    for (unsigned int i = 0; i < n; ++i)
    {
      dof_values[i].resize(_count);
      auto dof = _dof_indices[i];
      for (unsigned int j = 0; j < _count; ++j)
        dof_values[i](j) = sol(dof++);
    }
  }
  else
  {
    for (unsigned int i = 0; i < n; ++i)
    {
      dof_values[i].resize(_count);
      auto dof = _dof_indices[i];
      for (unsigned int j = 0; j < _count; ++j)
      {
        dof_values[i](j) = sol(dof);
        dof += n;
      }
    }
  }
}

template <>
void
MooseVariableDataBase<RealEigenVector>::fetchDoFValues()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  if (is_transient)
  {
    if (_need_u_dot || _need_grad_dot || _need_dof_values_dot)
    {
      libmesh_assert(_sys.solutionUDot());
      getArrayDoFValues(*_sys.solutionUDot(), n, _dof_values_dot);
    }
    if (_need_u_dotdot || _need_grad_dotdot || _need_dof_values_dotdot)
    {
      libmesh_assert(_sys.solutionUDotDot());
      getArrayDoFValues(*_sys.solutionUDot(), n, _dof_values_dotdot);
    }
    if (_need_u_dot_old || _need_dof_values_dot_old)
    {
      libmesh_assert(_sys.solutionUDotOld());
      getArrayDoFValues(*_sys.solutionUDotOld(), n, _dof_values_dot_old);
    }
    if (_need_u_dotdot_old || _need_dof_values_dotdot_old)
    {
      libmesh_assert(_sys.solutionUDotDotOld());
      getArrayDoFValues(*_sys.solutionUDotDotOld(), n, _dof_values_dotdot_old);
    }
  }

  for (auto tag : _required_vector_tags)
    if ((_subproblem.vectorTagType(tag) == Moose::VECTOR_TAG_RESIDUAL &&
         _subproblem.safeAccessTaggedVectors()) ||
        _subproblem.vectorTagType(tag) == Moose::VECTOR_TAG_SOLUTION)
      // tag is defined on problem but may not be used by a system
      if (_sys.hasVector(tag) && _sys.getVector(tag).closed())
        getArrayDoFValues(_sys.getVector(tag), n, _vector_tags_dof_u[tag]);

  if (_subproblem.safeAccessTaggedMatrices())
  {
    auto & active_coupleable_matrix_tags =
        _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);
    for (auto tag : active_coupleable_matrix_tags)
    {
      _matrix_tags_dof_u[tag].resize(n);
      if (_need_matrix_tag_dof_u[tag] || _need_matrix_tag_u[tag])
        if (_sys.hasMatrix(tag) && _sys.matrixTagActive(tag) && _sys.getMatrix(tag).closed())
        {
          auto & mat = _sys.getMatrix(tag);
          for (unsigned i = 0; i < n; i++)
          {
            Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
            for (unsigned j = 0; j < _count; j++)
              _matrix_tags_dof_u[tag][i](j) = mat(_dof_indices[i] + j, _dof_indices[i] + j);
          }
        }
    }
  }

  if (_need_du_dot_du || _need_dof_du_dot_du)
  {
    _dof_du_dot_du.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dot_du[i] = _sys.duDotDu(_var.number());
  }
  if (_need_du_dotdot_du || _need_dof_du_dotdot_du)
  {
    _dof_du_dotdot_du.resize(n);
    for (decltype(n) i = 0; i < n; ++i)
      _dof_du_dotdot_du[i] = _sys.duDotDotDu();
  }
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::zeroSizeDofValues()
{
  if (_subproblem.isTransient())
  {
    _dof_values_dot.resize(0);
    _dof_values_dotdot.resize(0);
    _dof_values_dot_old.resize(0);
    _dof_values_dotdot_old.resize(0);
    _dof_du_dot_du.resize(0);
    _dof_du_dotdot_du.resize(0);
  }

  for (auto & dof_values : _vector_tags_dof_u)
    dof_values.resize(0);

  _has_dof_values = false;
}

template <typename OutputType>
void
MooseVariableDataBase<OutputType>::assignNodalValue()
{
  bool is_transient = _subproblem.isTransient();

  libmesh_assert(_dof_indices.size());

  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  _nodal_value = dof_values[0];
  _nodal_value_array[0] = _nodal_value;

  if (is_transient)
  {
    if (oldestSolutionStateRequested() >= 1)
    {
      _nodal_value_old = _vector_tags_dof_u[_old_solution_tag][0];
      _nodal_value_old_array[0] = _nodal_value_old;
    }
    if (oldestSolutionStateRequested() >= 2)
    {
      _nodal_value_older = _vector_tags_dof_u[_older_solution_tag][0];
      _nodal_value_older_array[0] = _nodal_value_older;
    }
    if (_need_dof_values_dot)
      _nodal_value_dot = _dof_values_dot[0];
    if (_need_dof_values_dotdot)
      _nodal_value_dotdot = _dof_values_dotdot[0];
    if (_need_dof_values_dot_old)
      _nodal_value_dot_old = _dof_values_dot_old[0];
    if (_need_dof_values_dotdot_old)
      _nodal_value_dotdot_old = _dof_values_dotdot_old[0];
  }
  if (_previous_nl_solution_tag != Moose::INVALID_TAG_ID)
    _nodal_value_previous_nl = _vector_tags_dof_u[_previous_nl_solution_tag][0];
}

template <>
void
MooseVariableDataBase<RealVectorValue>::assignNodalValue()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  for (decltype(n) i = 0; i < n; ++i)
    _nodal_value(i) = dof_values[i];
  _nodal_value_array[0] = _nodal_value;

  if (is_transient)
  {
    if (oldestSolutionStateRequested() >= 1)
    {
      auto & dof_values_old = _vector_tags_dof_u[_old_solution_tag];
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_old(i) = dof_values_old[i];
    }
    if (oldestSolutionStateRequested() >= 2)
    {
      auto & dof_values_older = _vector_tags_dof_u[_older_solution_tag];
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_older(i) = dof_values_older[i];
    }
    if (_need_dof_values_dot)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dot(i) = _dof_values_dot[i];
    if (_need_dof_values_dotdot)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dotdot(i) = _dof_values_dotdot[i];
    if (_need_dof_values_dot_old)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dot_old(i) = _dof_values_dot_old[i];
    if (_need_dof_values_dotdot_old)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_dotdot_old(i) = _dof_values_dotdot_old[i];
  }
  if (_previous_nl_solution_tag != Moose::INVALID_TAG_ID)
  {
    auto & dof_values_previous_nl = _vector_tags_dof_u[_previous_nl_solution_tag];
    for (decltype(n) i = 0; i < n; ++i)
      _nodal_value_previous_nl(i) = dof_values_previous_nl[i];
  }
}

template class MooseVariableDataBase<Real>;
template class MooseVariableDataBase<RealVectorValue>;
template class MooseVariableDataBase<RealEigenVector>;
