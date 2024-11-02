//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
MooseVariableDataFV<OutputType>::MooseVariableDataFV(const MooseVariableFV<OutputType> & var,
                                                     SystemBase & sys,
                                                     THREAD_ID tid,
                                                     Moose::ElementType element_type,
                                                     const Elem * const & elem)

  : MooseVariableDataBase<OutputType>(var, sys, tid),
    MeshChangedInterface(var.parameters()),
    _var(var),
    _fe_type(_var.feType()),
    _var_num(_var.number()),
    _assembly(_subproblem.assembly(_tid, var.kind() == Moose::VAR_SOLVER ? sys.number() : 0)),
    _element_type(element_type),
    _ad_zero(0),
    _need_second(false),
    _need_second_old(false),
    _need_second_older(false),
    _need_second_previous_nl(false),
    _need_curl(false),
    _need_curl_old(false),
    _need_curl_older(false),
    // for FV variables, they use each other's ad_u values to compute ghost cell
    // values - we don't have any way to propagate these inter-variable-data
    // dependencies. So if something needs an ad_u value, that need isn't
    // propagated through to both the element and the neighbor
    // data structures. So instead just set _need_ad+_need_ad_u values to true always.
    _need_ad(true),
    _need_ad_u(true),
    _need_ad_u_dot(false),
    _need_ad_u_dotdot(false),
    _need_ad_grad_u(false),
    _need_ad_grad_u_dot(false),
    _need_ad_second_u(false),
    _time_integrator(_sys.queryTimeIntegrator(_var_num)),
    _elem(elem),
    _displaced(dynamic_cast<const DisplacedSystem *>(&_sys) ? true : false),
    _qrule(nullptr)
{
  _fv_elemental_kernel_query_cache =
      _subproblem.getMooseApp().theWarehouse().query().template condition<AttribSystem>(
          "FVElementalKernel");
  _fv_flux_kernel_query_cache =
      _subproblem.getMooseApp().theWarehouse().query().template condition<AttribSystem>(
          "FVFluxKernel");
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::setGeometry(Moose::GeometryType gm_type)
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
const typename MooseVariableDataFV<OutputType>::FieldVariableValue &
MooseVariableDataFV<OutputType>::uDot() const
{
  if (_sys.solutionUDot())
  {
    _var.requireQpComputations();
    _need_u_dot = true;
    return _u_dot;
  }
  else
    mooseError("MooseVariableFE: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableValue &
MooseVariableDataFV<OutputType>::uDotDot() const
{
  if (_sys.solutionUDotDot())
  {
    _var.requireQpComputations();
    _need_u_dotdot = true;
    return _u_dotdot;
  }
  else
    mooseError("MooseVariableFE: Second time derivative of solution (`u_dotdot`) is not stored. "
               "Please set uDotDotRequested() to true in FEProblemBase before requesting "
               "`u_dotdot`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableValue &
MooseVariableDataFV<OutputType>::uDotOld() const
{
  if (_sys.solutionUDotOld())
  {
    _var.requireQpComputations();
    _need_u_dot_old = true;
    return _u_dot_old;
  }
  else
    mooseError("MooseVariableFE: Old time derivative of solution (`u_dot_old`) is not stored. "
               "Please set uDotOldRequested() to true in FEProblemBase before requesting "
               "`u_dot_old`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableValue &
MooseVariableDataFV<OutputType>::uDotDotOld() const
{
  if (_sys.solutionUDotDotOld())
  {
    _var.requireQpComputations();
    _need_u_dotdot_old = true;
    return _u_dotdot_old;
  }
  else
    mooseError("MooseVariableFE: Old second time derivative of solution (`u_dotdot_old`) is not "
               "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
               "requesting `u_dotdot_old`");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableValue &
MooseVariableDataFV<OutputType>::sln(Moose::SolutionState state) const
{
  _var.requireQpComputations();
  return MooseVariableDataBase<OutputType>::sln(state);
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableGradient &
MooseVariableDataFV<OutputType>::gradSlnDot() const
{
  if (_sys.solutionUDot())
  {
    _var.requireQpComputations();
    _need_grad_dot = true;
    return _grad_u_dot;
  }
  else
    mooseError("MooseVariableFE: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableGradient &
MooseVariableDataFV<OutputType>::gradSlnDotDot() const
{
  if (_sys.solutionUDotDot())
  {
    _var.requireQpComputations();
    _need_grad_dotdot = true;
    return _grad_u_dotdot;
  }
  else
    mooseError("MooseVariableFE: Second time derivative of solution (`u_dotdot`) is not stored. "
               "Please set uDotDotRequested() to true in FEProblemBase before requesting "
               "`u_dotdot`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableSecond &
MooseVariableDataFV<OutputType>::secondSln(Moose::SolutionState state) const
{
  _var.requireQpComputations();
  switch (state)
  {
    case Moose::Current:
    {
      _need_second = true;
      return _second_u;
    }

    case Moose::Old:
    {
      _need_second_old = true;
      return _second_u_old;
    }

    case Moose::Older:
    {
      _need_second_older = true;
      return _second_u_older;
    }

    case Moose::PreviousNL:
    {
      _need_second_previous_nl = true;
      return _second_u_previous_nl;
    }

    default:
      // We should never get here but gcc requires that we have a default. See
      // htpps://stackoverflow.com/questions/18680378/after-defining-case-for-all-enum-values-compiler-still-says-control-reaches-e
      mooseError("Unknown SolutionState!");
  }
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableCurl &
MooseVariableDataFV<OutputType>::curlSln(Moose::SolutionState state) const
{
  _var.requireQpComputations();
  switch (state)
  {
    case Moose::Current:
    {
      _need_curl = true;
      return _curl_u;
    }

    case Moose::Old:
    {
      _need_curl_old = true;
      return _curl_u_old;
    }

    case Moose::Older:
    {
      _need_curl_older = true;
      return _curl_u_older;
    }

    default:
      mooseError("We don't currently support curl from the previous non-linear iteration");
  }
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::initializeSolnVars()
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

  if (_need_second)
  {
    _second_u.resize(nqp);
    assignForAllQps(0, _second_u, nqp);
  }

  if (_need_curl)
  {
    _curl_u.resize(nqp);
    assignForAllQps(0, _curl_u, nqp);
  }

  if (_need_second_previous_nl)
  {
    _second_u_previous_nl.resize(nqp);
    assignForAllQps(0, _second_u_previous_nl, nqp);
  }

  if (_subproblem.isTransient())
  {
    if (_need_u_dot)
    {
      _u_dot.resize(nqp);
      assignForAllQps(0, _u_dot, nqp);
    }

    if (_need_u_dotdot)
    {
      _u_dotdot.resize(nqp);
      assignForAllQps(0, _u_dotdot, nqp);
    }

    if (_need_u_dot_old)
    {
      _u_dot_old.resize(nqp);
      assignForAllQps(0, _u_dot_old, nqp);
    }

    if (_need_u_dotdot_old)
    {
      _u_dotdot_old.resize(nqp);
      assignForAllQps(0, _u_dotdot_old, nqp);
    }

    if (_need_du_dot_du)
    {
      _du_dot_du.resize(nqp);
      assignForAllQps(0, _du_dot_du, nqp);
    }

    if (_need_du_dotdot_du)
    {
      _du_dotdot_du.resize(nqp);
      assignForAllQps(0, _du_dotdot_du, nqp);
    }

    if (_need_grad_dot)
    {
      _grad_u_dot.resize(nqp);
      assignForAllQps(0, _grad_u_dot, nqp);
    }

    if (_need_grad_dotdot)
    {
      _grad_u_dotdot.resize(nqp);
      assignForAllQps(0, _grad_u_dotdot, nqp);
    }

    if (_need_second_old)
    {
      _second_u_old.resize(nqp);
      assignForAllQps(0, _second_u_old, nqp);
    }

    if (_need_curl_old)
    {
      _curl_u_old.resize(nqp);
      assignForAllQps(0, _curl_u_old, nqp);
    }

    if (_need_second_older)
    {
      _second_u_older.resize(nqp);
      assignForAllQps(0, _second_u_older, nqp);
    }
  }
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::computeValuesFace(const FaceInfo & /*fi*/)
{
  _has_dirichlet_bc = false;
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);

  // TODO: compute reconstructed values somehow.  For now, just do the trivial
  // reconstruction where we take the const cell value from the centroid and
  // use that value on the face.  How will users affect the reconstruction
  // method used here?  After reconstruction, we should cache the computed
  // soln/gradients per element so we don't have to recompute them again for
  // other faces that share an element with this face.
  //
  // TODO: Also we need to be able to track (AD) derivatives through the
  // reconstruction process - how will we do that?

  computeValues();

  // TODO: maybe initialize a separate _grad_u_interface here that is
  // only used for diffusion terms that need an interface gradient.  Also -
  // it would need cross-diffusion corrections for non-orthogonal meshes
  // eventually. Or maybe we just leave this alone zero and have users
  // calculate whatever grad_interface value they want and provide some
  // helper functions for cross-diffusion correction.

  // TODO: figure out how to store old/older values of reconstructed
  // solutions/gradient states without having to re-reconstruct them from
  // older dof/soln values.
}

template <typename OutputType>
const std::vector<dof_id_type> &
MooseVariableDataFV<OutputType>::initDofIndices()
{
  Moose::initDofIndices(*this, *_elem);
  return _dof_indices;
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::computeValues()
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

  bool is_transient = _subproblem.isTransient();
  const auto nqp = _qrule->n_points();
  auto && active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);

  bool second_required =
      _need_second || _need_second_old || _need_second_older || _need_second_previous_nl;
  bool curl_required = _need_curl || _need_curl_old;

  for (const auto qp : make_range(nqp))
  {
    if (is_transient)
    {
      if (_need_u_dot)
        _u_dot[qp] = _dof_values_dot[0];

      if (_need_u_dotdot)
        _u_dotdot[qp] = _dof_values_dotdot[0];

      if (_need_u_dot_old)
        _u_dot_old[qp] = _dof_values_dot_old[0];

      if (_need_u_dotdot_old)
        _u_dotdot_old[qp] = _dof_values_dotdot_old[0];

      if (_need_du_dot_du)
        _du_dot_du[qp] = _dof_du_dot_du[0];

      if (_need_du_dotdot_du)
        _du_dotdot_du[qp] = _dof_du_dotdot_du[0];
    }

    if (second_required)
    {
      if (_need_second)
        _second_u[qp] = 0;

      if (_need_second_previous_nl)
        _second_u_previous_nl[qp] = 0;

      if (is_transient)
      {
        if (_need_second_old)
          _second_u_old[qp] = 0;

        if (_need_second_older)
          _second_u_older[qp] = 0;
      }
    }

    if (curl_required)
    {
      if (_need_curl)
        _curl_u[qp] = 0;

      if (is_transient && _need_curl_old)
        _curl_u_old[qp] = 0;
    }

    for (auto tag : _required_vector_tags)
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][qp] = _vector_tags_dof_u[tag][0];

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][qp] = _matrix_tags_dof_u[tag][0];
  }

  // Automatic differentiation
  if (_need_ad)
    computeAD(num_dofs, nqp);
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::computeAD(const unsigned int num_dofs, const unsigned int nqp)
{
  // This query and if check prevent running this code when we have FV
  // variables, but no kernels.  When this happens, maxVarNDofsPerElem is
  // not computed (because no kernels) and is zero giving nonsense offsets for
  // AD stuff.  So we just skip all this when that is the case.  Maybe there
  // is a better way to do this - like just checking if getMaxVarNDofsPerElem
  // returns zero?
  std::vector<FVKernel *> ks;
  _fv_elemental_kernel_query_cache.queryInto(ks);
  if (ks.size() == 0)
  {
    _fv_flux_kernel_query_cache.queryInto(ks);
    if (ks.size() == 0)
      return;
  }

  _ad_dof_values.resize(num_dofs);
  if (_need_ad_u)
    _ad_u.resize(nqp);

  if (_need_ad_grad_u)
    _ad_grad_u.resize(nqp);

  if (_need_ad_second_u)
    _ad_second_u.resize(nqp);

  if (_need_ad_u_dot)
  {
    _ad_dofs_dot.resize(num_dofs);
    _ad_u_dot.resize(nqp);
  }

  if (_need_ad_u_dotdot)
  {
    _ad_dofs_dotdot.resize(num_dofs);
    _ad_u_dotdot.resize(nqp);
  }

  if (_need_ad_second_u)
    assignForAllQps(0, _ad_second_u, nqp);

  if (_need_ad_u_dot)
    assignForAllQps(_ad_zero, _ad_u_dot, nqp);

  if (_need_ad_u_dotdot)
    assignForAllQps(_ad_zero, _ad_u_dotdot, nqp);

  const bool do_derivatives =
      ADReal::do_derivatives && _sys.number() == _subproblem.currentNlSysNum();

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (do_derivatives)
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);

    if (_need_ad_u_dot && safeToComputeADUDot() && _time_integrator->dt())
    {
      _ad_dofs_dot[i] = _ad_dof_values[i];
      _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i],
                                                 _dof_indices[i],
                                                 _need_ad_u_dotdot ? _ad_dofs_dotdot[i]
                                                                   : _ad_real_dummy);
    }
  }

  if (_need_ad_u)
    assignForAllQps(_ad_dof_values[0], _ad_u, nqp);

  if (_need_ad_grad_u)
    assignForAllQps(static_cast<const MooseVariableFV<OutputType> &>(_var).adGradSln(
                        _elem, Moose::currentState()),
                    _ad_grad_u,
                    nqp);

  if (_need_ad_u_dot)
  {
    if (safeToComputeADUDot())
    {
      assignForAllQps(_ad_dofs_dot[0], _ad_u_dot, nqp);
      if (_need_ad_u_dotdot)
        assignForAllQps(_ad_dofs_dotdot[0], _ad_u_dotdot, nqp);
    }
    else
    {
      assignForAllQps(_u_dot[0], _ad_u_dot, nqp);
      if (_need_ad_u_dotdot)
        assignForAllQps(_u_dotdot[0], _ad_u_dotdot, nqp);
    }
  }
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::setDofValue(const OutputData & value, unsigned int index)
{
  mooseAssert(index == 0, "We only ever have one dof value locally");
  _vector_tags_dof_u[_solution_tag][index] = value;
  _has_dof_values = true;

  auto & u = _vector_tag_u[_solution_tag];
  // Update the qp values as well
  for (const auto qp : index_range(u))
    u[qp] = value;

  if (_need_ad_u)
    for (const auto qp : index_range(_ad_u))
      _ad_u[qp] = value;
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::setDofValues(const DenseVector<OutputData> & values)
{
  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  for (unsigned int i = 0; i < values.size(); i++)
    dof_values[i] = values(i);
  _has_dof_values = true;
}

template <typename OutputType>
typename MooseVariableDataFV<OutputType>::OutputData
MooseVariableDataFV<OutputType>::getElementalValue(const Elem * elem,
                                                   Moose::SolutionState state,
                                                   unsigned int idx) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  switch (state)
  {
    case Moose::Current:
      return (*_sys.currentSolution())(dof_indices[idx]);

    case Moose::Old:
      return _sys.solutionOld()(dof_indices[idx]);

    case Moose::Older:
      return _sys.solutionOlder()(dof_indices[idx]);

    default:
      mooseError("PreviousNL not currently supported for getElementalValue");
  }
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::getDofIndices(const Elem * elem,
                                               std::vector<dof_id_type> & dof_indices) const
{
  _dof_map.dof_indices(elem, dof_indices, _var_num);
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValuesDot() const
{
  if (_sys.solutionUDot())
  {
    _need_dof_values_dot = true;
    return _dof_values_dot;
  }
  else
    mooseError(
        "MooseVariableDataFV: Time derivative of solution (`u_dot`) is not stored. Please set "
        "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValuesDotDot() const
{
  if (_sys.solutionUDotDot())
  {
    _need_dof_values_dotdot = true;
    return _dof_values_dotdot;
  }
  else
    mooseError(
        "MooseVariableDataFV: Second time derivative of solution (`u_dotdot`) is not stored. "
        "Please set uDotDotRequested() to true in FEProblemBase before requesting "
        "`u_dotdot`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValuesDotOld() const
{
  if (_sys.solutionUDotOld())
  {
    _need_dof_values_dot_old = true;
    return _dof_values_dot_old;
  }
  else
    mooseError("MooseVariableDataFV: Old time derivative of solution (`u_dot_old`) is not stored. "
               "Please set uDotOldRequested() to true in FEProblemBase before requesting "
               "`u_dot_old`.");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValuesDotDotOld() const
{
  if (_sys.solutionUDotDotOld())
  {
    _need_dof_values_dotdot_old = true;
    return _dof_values_dotdot_old;
  }
  else
    mooseError(
        "MooseVariableDataFV: Old second time derivative of solution (`u_dotdot_old`) is not "
        "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
        "requesting `u_dotdot_old`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableDataFV<OutputType>::dofValuesDuDotDu() const
{
  _need_dof_du_dot_du = true;
  return _dof_du_dot_du;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableDataFV<OutputType>::dofValuesDuDotDotDu() const
{
  _need_dof_du_dotdot_du = true;
  return _dof_du_dotdot_du;
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::fetchADDoFValues()
{
  auto n = _dof_indices.size();
  libmesh_assert(n);
  _ad_dof_values.resize(n);

  const bool do_derivatives =
      ADReal::do_derivatives && _sys.number() == _subproblem.currentNlSysNum();

  for (decltype(n) i = 0; i < n; ++i)
  {
    _ad_dof_values[i] = _vector_tags_dof_u[_solution_tag][i];
    if (do_derivatives)
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);
  }
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::prepareIC()
{
  // TODO: implement this function
  initDofIndices();
  _vector_tags_dof_u[_solution_tag].resize(_dof_indices.size());

  mooseAssert(_qrule, "We should have a non-null qrule");
  const auto nqp = _qrule->n_points();
  _vector_tag_u[_solution_tag].resize(nqp);
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::meshChanged()
{
  _prev_elem = nullptr;
}

template class MooseVariableDataFV<Real>;
// TODO: implement vector fv variable support. This will require some template
// specializations for various member functions in this and the FV variable
// classes. And then you will need to uncomment out the line below:
// template class MooseVariableDataFV<RealVectorValue>;
