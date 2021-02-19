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
                                                     const SystemBase & sys,
                                                     THREAD_ID tid,
                                                     Moose::ElementType element_type,
                                                     const Elem * const & elem)

  : _var(var),
    _fe_type(_var.feType()),
    _var_num(_var.number()),
    _var_name(_var.name()),
    _sys(sys),
    _subproblem(_sys.subproblem()),
    _tid(tid),
    _assembly(_subproblem.assembly(_tid)),
    _dof_map(_sys.dofMap()),
    _element_type(element_type),
    _count(var.count()),
    _ad_zero(0),
    _need_u_old(false),
    _need_u_older(false),
    _need_u_previous_nl(false),
    _need_u_dot(false),
    _need_u_dotdot(false),
    _need_u_dot_old(false),
    _need_u_dotdot_old(false),
    _need_du_dot_du(false),
    _need_du_dotdot_du(false),
    _need_grad_old(false),
    _need_grad_older(false),
    _need_grad_previous_nl(false),
    _need_grad_dot(false),
    _need_grad_dotdot(false),
    _need_second(false),
    _need_second_old(false),
    _need_second_older(false),
    _need_second_previous_nl(false),
    _need_curl(false),
    _need_curl_old(false),
    _need_curl_older(false),
    // for FV variables, they use each other's ad_u values to compute ghost cell
    // values - we don't have any way to propogate these inter-variable-data
    // dependencies. So if something needs an ad_u value, that need isn't
    // propogated through to both the element and the neighbor
    // data structures. So instead just set _need_ad+_need_ad_u values to true always.
    _need_ad(true),
    _need_ad_u(true),
    _need_ad_u_dot(false),
    _need_ad_grad_u(false),
    _need_ad_second_u(false),
    _need_dof_values(false),
    _need_dof_values_old(false),
    _need_dof_values_older(false),
    _need_dof_values_previous_nl(false),
    _need_dof_values_dot(false),
    _need_dof_values_dotdot(false),
    _need_dof_values_dot_old(false),
    _need_dof_values_dotdot_old(false),
    _need_dof_du_dot_du(false),
    _need_dof_du_dotdot_du(false),
    _time_integrator(_sys.getTimeIntegrator()),
    _elem(elem),
    _displaced(dynamic_cast<const DisplacedSystem *>(&_sys) ? true : false),
    _qrule(nullptr)
{
  auto num_vector_tags = _sys.subproblem().numVectorTags();

  _vector_tags_dof_u.resize(num_vector_tags);
  _need_vector_tag_dof_u.resize(num_vector_tags);

  _need_vector_tag_u.resize(num_vector_tags);
  _vector_tag_u.resize(num_vector_tags);

  auto num_matrix_tags = _sys.subproblem().numMatrixTags();

  _matrix_tags_dof_u.resize(num_matrix_tags);
  _need_matrix_tag_dof_u.resize(num_matrix_tags);

  _need_matrix_tag_u.resize(num_matrix_tags);
  _matrix_tag_u.resize(num_matrix_tags);
}

template <typename OutputType>
unsigned int
MooseVariableDataFV<OutputType>::oldestSolutionStateRequested() const
{
  if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_values_older)
    return 2;
  if (_need_u_old || _need_grad_old || _need_second_old || _need_dof_values_old)
    return 1;
  return 0;
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
MooseVariableDataFV<OutputType>::sln(Moose::SolutionState state) const
{
  switch (state)
  {
    case Moose::Current:
      return _u;

    case Moose::Old:
    {
      _need_u_old = true;
      return _u_old;
    }

    case Moose::Older:
    {
      _need_u_older = true;
      return _u_older;
    }

    case Moose::PreviousNL:
    {
      _need_u_previous_nl = true;
      return _u_previous_nl;
    }

    default:
      // We should never get here but gcc requires that we have a default. See
      // htpps://stackoverflow.com/questions/18680378/after-defining-case-for-all-enum-values-compiler-still-says-control-reaches-e
      mooseError("Unknown SolutionState!");
  }
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableValue &
MooseVariableDataFV<OutputType>::uDot() const
{
  if (_sys.solutionUDot())
  {
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
    _need_u_dotdot_old = true;
    return _u_dotdot_old;
  }
  else
    mooseError("MooseVariableFE: Old second time derivative of solution (`u_dotdot_old`) is not "
               "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
               "requesting `u_dotdot_old`");
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableGradient &
MooseVariableDataFV<OutputType>::gradSln(Moose::SolutionState state) const
{
  switch (state)
  {
    case Moose::Current:
      return _grad_u;

    case Moose::Old:
    {
      _need_grad_old = true;
      return _grad_u_old;
    }

    case Moose::Older:
    {
      _need_grad_older = true;
      return _grad_u_older;
    }

    case Moose::PreviousNL:
    {
      _need_grad_previous_nl = true;
      return _grad_u_previous_nl;
    }

    default:
      // We should never get here but gcc requires that we have a default. See
      // htpps://stackoverflow.com/questions/18680378/after-defining-case-for-all-enum-values-compiler-still-says-control-reaches-e
      mooseError("Unknown SolutionState!");
  }
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::FieldVariableGradient &
MooseVariableDataFV<OutputType>::gradSlnDot() const
{
  if (_sys.solutionUDot())
  {
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

namespace
{
template <typename T, typename T2>
void
assignForAllQps(const T & value, T2 & array, const unsigned int nqp)
{
  for (const auto qp : make_range(nqp))
    array[qp] = value;
}
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::initializeSolnVars()
{
  auto && active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
  auto && active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);
  mooseAssert(_qrule, "We should have a non-null qrule");
  const auto nqp = _qrule->n_points();

  _u.resize(nqp);
  assignForAllQps(0, _u, nqp);
  _grad_u.resize(nqp);
  assignForAllQps(0, _grad_u, nqp);

  for (auto tag : active_coupleable_vector_tags)
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

  if (_need_u_previous_nl)
  {
    _u_previous_nl.resize(nqp);
    assignForAllQps(0, _u_previous_nl, nqp);
  }

  if (_need_grad_previous_nl)
  {
    _grad_u_previous_nl.resize(nqp);
    assignForAllQps(0, _grad_u_previous_nl, nqp);
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

    if (_need_u_old)
    {
      _u_old.resize(nqp);
      assignForAllQps(0, _u_old, nqp);
    }

    if (_need_u_older)
    {
      _u_older.resize(nqp);
      assignForAllQps(0, _u_older, nqp);
    }

    if (_need_grad_old)
    {
      _grad_u_old.resize(nqp);
      assignForAllQps(0, _grad_u_old, nqp);
    }

    if (_need_grad_older)
    {
      _grad_u_older.resize(nqp);
      assignForAllQps(0, _grad_u_older, nqp);
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
MooseVariableDataFV<OutputType>::computeGhostValuesFace(
    const FaceInfo & fi, MooseVariableDataFV<OutputType> & other_face)
{
  _has_dirichlet_bc = false;
  initializeSolnVars();

  std::vector<FVDirichletBCBase *> bcs;

  // TODO: this query probably (maybe?)needs to also filter based on the
  // active tags - these currently live in the flux thread loop object and I'm
  // not sure how best to get them here.
  _subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribSystem>("FVDirichletBC")
      .template condition<AttribThread>(_tid)
      .template condition<AttribBoundaries>(fi.boundaryIDs())
      .template condition<AttribVar>(_var_num)
      .template condition<AttribSysNum>(_var.sys().number())
      .queryInto(bcs);
  mooseAssert(bcs.size() <= 1, "cannot have multiple dirichlet BCs on the same boundary");
  _has_dirichlet_bc = bcs.size() > 0;

  // These need to be initialized but we can't use the regular computeAD
  // routine because that routine accesses the solution which doesn't exist
  // for this ghost element. So we do it manually here:
  mooseAssert(_qrule, "We should have a non-null qrule");
  const auto nqp = _qrule->n_points();
  if (_need_ad_u)
    _ad_u.resize(nqp);
  if (_need_ad_grad_u)
    _ad_grad_u.resize(nqp);
  if (_need_ad_u_dot)
    _ad_u_dot.resize(nqp);
  if (_need_ad_second_u)
    _ad_second_u.resize(nqp);
  if (_need_ad_u_dot)
    _ad_u_dot.resize(nqp);

  mooseAssert(
      _need_ad_u_dot && !other_face._need_ad_u_dot
          ? _element_type == Moose::ElementType::Element &&
                other_face._element_type != Moose::ElementType::Element
          : true,
      "I only really understand having a time derivative for this and not having a time "
      "derivative for the other if this is elemental data and the other is not elemental data.");

#ifdef MOOSE_GLOBAL_AD_INDEXING
  const ADReal u_other = _subproblem.currentlyComputingJacobian()
                             ? other_face.adSln()[0]
                             : other_face.sln(Moose::Current)[0];
  const auto & u_face = _var.getBoundaryFaceValue(fi);
  const auto u_ghost = 2 * u_face - u_other;

  if (_need_ad_u)
    assignForAllQps(u_ghost, _ad_u, nqp);
  assignForAllQps(u_ghost.value(), _u, nqp);

  // If people have time derivatives on faces, then they're in trouble (for now) because we do not
  // know the time dependence of u_face. Let's try to do better than silent wrong values at least in
  // debugging modes by tossing some NaNs in
  if (_need_ad_u_dot && other_face._need_ad_u_dot)
    assignForAllQps(std::numeric_limits<typename ADReal::value_type>::quiet_NaN(), _ad_u_dot, nqp);

#else
  if (_has_dirichlet_bc)
  {
    // extrapolate from the boundary element across the boundary face using
    // the given BC face value to determine a ghost cell value for u.  Be sure
    // to propogate AD derivatives through all this.

    auto bc = bcs[0];
    DualReal u_face = bc->boundaryValue(fi);
    // This approach has the problem that need_ad_u isn't marked as true until
    // after the first computeValues has been run - so the AD values haven't
    // been initialized - causing this call to not work.  However, an ugly
    // hack has been implemented by initializing _need_ad_u and friends to 'true'
    // by default.  Consider perhaps a better solution to this problem.
    DualReal u_other;
    if (_subproblem.currentlyComputingJacobian())
      u_other = other_face.adSln()[0];
    else
      u_other = other_face.sln(Moose::Current)[0];

    auto u_ghost = 2 * u_face - u_other;

    if (_need_ad_u)
      assignForAllQps(u_ghost, _ad_u, nqp);
    assignForAllQps(u_ghost.value(), _u, nqp);

    if (_need_ad_u_dot && other_face._need_ad_u_dot)
      // The partial derivative with respect to time is the same as for u_other except with a
      // negative sign. (See the u_ghost formula above)
      assignForAllQps(-other_face.adUDot()[0], _ad_u_dot, nqp);
  }
  else
  {
    // in this case, we are on a boundary for this variable where a
    // flux boundary condition may be applied; in the FVFluxBC class
    // switching between _u and _u_neighbor is tedious so it's best
    // to make the value on elem and neighbor the same
    // TODO: make sure DirichletBC and FluxBC are _not_ defined on
    // the same sideset
    DualReal u_other;
    if (_subproblem.currentlyComputingJacobian())
      u_other = other_face.adSln()[0];
    else
      u_other = other_face.sln(Moose::Current)[0];

    if (_need_ad_u)
      assignForAllQps(u_other, _ad_u, nqp);
    assignForAllQps(u_other.value(), _u, nqp);

    if (_need_ad_u_dot && other_face._need_ad_u_dot)
      // Since we are simply tracking the other face's value, the time derivative is also the same
      assignForAllQps(other_face.adUDot()[0], _ad_u_dot, nqp);
  }
#endif

  // At this point we're only doing const monomials and we're not accessing reconstruction
  // information through no-param-accessors (e.g. adGradSln()), so any spatial derivatives are zero.
  // We need to explicitly assign here to ensure that the values and DualNumber derivatives are
  // properly initialized
  if (_need_ad_grad_u)
    assignForAllQps(0, _ad_grad_u, nqp);
  if (_need_ad_second_u)
    assignForAllQps(0, _ad_second_u, nqp);
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

  mooseAssert(num_dofs == 1 && _dof_values.size() == 1,
              "There should only be one dof per elem for FV variables");

  bool is_transient = _subproblem.isTransient();
  const auto nqp = _qrule->n_points();
  auto && active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
  auto && active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);

  bool second_required =
      _need_second || _need_second_old || _need_second_older || _need_second_previous_nl;
  bool curl_required = _need_curl || _need_curl_old;

  for (const auto qp : make_range(nqp))
  {
    _u[qp] = _dof_values[0];
    _grad_u[qp] = 0;

    if (is_transient)
    {
      if (_need_u_old)
        _u_old[qp] = _dof_values_old[0];

      if (_need_u_older)
        _u_older[qp] = _dof_values_older[0];

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

    for (auto tag : active_coupleable_vector_tags)
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][qp] = _vector_tags_dof_u[tag][0];

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][qp] = _matrix_tags_dof_u[tag][0];

    if (_need_u_previous_nl)
      _u_previous_nl[qp] = _dof_values_previous_nl[0];

    if (_need_grad_previous_nl)
      _grad_u_previous_nl[qp] = _dof_values_previous_nl[0];
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
  std::vector<FVKernel *> ks1;
  std::vector<FVKernel *> ks2;
  _subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribSystem>("FVElementalKernel")
      .queryInto(ks1);
  _subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribSystem>("FVFluxKernel")
      .queryInto(ks2);
  if (ks1.size() == 0 && ks2.size() == 0)
    return;

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

#ifndef MOOSE_GLOBAL_AD_INDEXING
  auto ad_offset = Moose::adOffset(
      _var_num, _sys.getMaxVarNDofsPerElem(), _element_type, _sys.system().n_vars());
  mooseAssert(_var.kind() == Moose::VarKindType::VAR_AUXILIARY || ad_offset || !_var_num,
              "Either this is the zeroth variable or we should have an offset");

#ifndef MOOSE_SPARSE_AD
  if (ad_offset + num_dofs > MOOSE_AD_MAX_DOFS_PER_ELEM)
    mooseError("Current number of dofs per element ",
               ad_offset + num_dofs,
               " is greater than AD_MAX_DOFS_PER_ELEM of ",
               MOOSE_AD_MAX_DOFS_PER_ELEM,
               ". You can run `configure --with-derivative-size=<n>` to request a larger "
               "derivative container.");
#endif
#endif

  if (_need_ad_second_u)
    assignForAllQps(0, _ad_second_u, nqp);

  if (_need_ad_u_dot)
    assignForAllQps(_ad_zero, _ad_u_dot, nqp);

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (_var.kind() == Moose::VAR_NONLINEAR && ADReal::do_derivatives)
#ifdef MOOSE_GLOBAL_AD_INDEXING
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);
#else
      Moose::derivInsert(_ad_dof_values[i].derivatives(), ad_offset + i, 1.);
#endif

    if (_need_ad_u_dot && _time_integrator && _time_integrator->dt())
    {
      _ad_dofs_dot[i] = _ad_dof_values[i];
      _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i], _dof_indices[i]);
    }
  }

  if (_need_ad_u)
    assignForAllQps(_ad_dof_values[0], _ad_u, nqp);

  if (_need_ad_grad_u)
    assignForAllQps(
#ifdef MOOSE_GLOBAL_AD_INDEXING
        _var.adGradSln(_elem),
#else
        _ad_zero,
#endif
        _ad_grad_u,
        nqp);

  if (_need_ad_u_dot && _time_integrator)
    assignForAllQps(_ad_dofs_dot[0], _ad_u_dot, nqp);

  if (_need_ad_u_dot && !_time_integrator)
    assignForAllQps(_u_dot[0], _ad_u_dot, nqp);
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::setDofValue(const OutputData & value, unsigned int index)
{
  mooseAssert(index == 0, "We only ever have one dof value locally");
  _dof_values[index] = value;

  // Update the qp values as well
  for (const auto qp : make_range(_u.size()))
    _u[qp] = value;

  if (_need_ad_u)
    for (const auto qp : make_range(_ad_u.size()))
      _ad_u[qp] = value;
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::setDofValues(const DenseVector<OutputData> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    _dof_values[i] = values(i);
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
void
MooseVariableDataFV<OutputType>::insert(NumericVector<Number> & residual)
{
  initDofIndices();
  residual.insert(&_dof_values[0], _dof_indices);
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValues() const
{
  _need_dof_values = true;
  return _dof_values;
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValuesOld() const
{
  _need_dof_values_old = true;
  return _dof_values_old;
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValuesOlder() const
{
  _need_dof_values_older = true;
  return _dof_values_older;
}

template <typename OutputType>
const typename MooseVariableDataFV<OutputType>::DoFValue &
MooseVariableDataFV<OutputType>::dofValuesPreviousNL() const
{
  _need_dof_values_previous_nl = true;
  return _dof_values_previous_nl;
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
MooseVariableDataFV<OutputType>::fetchDoFValues()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  _dof_values.resize(n);
  _sys.currentSolution()->get(_dof_indices, &_dof_values[0]);

  if (is_transient)
  {
    if (_need_u_old || _need_grad_old || _need_second_old || _need_curl_old || _need_dof_values_old)
    {
      _dof_values_old.resize(n);
      _sys.solutionOld().get(_dof_indices, &_dof_values_old[0]);
    }
    if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_values_older)
    {
      _dof_values_older.resize(n);
      _sys.solutionOlder().get(_dof_indices, &_dof_values_older[0]);
    }
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

  if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
      _need_dof_values_previous_nl)
  {
    _dof_values_previous_nl.resize(n);
    _sys.solutionPreviousNewton()->get(_dof_indices, &_dof_values_previous_nl[0]);
  }

  auto & active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
  for (auto tag : active_coupleable_vector_tags)
    if (_need_vector_tag_u[tag] || _need_vector_tag_dof_u[tag])
      if ((_sys.subproblem().vectorTagType(tag) == Moose::VECTOR_TAG_RESIDUAL &&
           _sys.subproblem().safeAccessTaggedVectors()) ||
          _sys.subproblem().vectorTagType(tag) == Moose::VECTOR_TAG_SOLUTION)
        if (_sys.hasVector(tag) && _sys.getVector(tag).closed())
        {
          auto & vec = _sys.getVector(tag);
          _vector_tags_dof_u[tag].resize(n);
          vec.get(_dof_indices, &_vector_tags_dof_u[tag][0]);
        }

  if (_sys.subproblem().safeAccessTaggedMatrices())
  {
    auto & active_coupleable_matrix_tags =
        _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);
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
      _dof_du_dot_du[i] = _sys.duDotDu();
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
MooseVariableDataFV<OutputType>::fetchADDoFValues()
{
  auto n = _dof_indices.size();
  libmesh_assert(n);
  _ad_dof_values.resize(n);
#ifndef MOOSE_GLOBAL_AD_INDEXING
  auto ad_offset = _var_num * _sys.getMaxVarNDofsPerNode();
#endif

  for (decltype(n) i = 0; i < n; ++i)
  {
    _ad_dof_values[i] = _dof_values[i];
    if (_var.kind() == Moose::VAR_NONLINEAR)
#ifdef MOOSE_GLOBAL_AD_INDEXING
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);
#else
      Moose::derivInsert(_ad_dof_values[i].derivatives(), ad_offset + i, 1.);
#endif
  }
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::zeroSizeDofValues()
{
  _dof_values.resize(0);
  if (_need_dof_values_previous_nl)
    _dof_values_previous_nl.resize(0);
  if (_subproblem.isTransient())
  {
    _dof_values_old.resize(0);
    _dof_values_older.resize(0);
    _dof_values_dot.resize(0);
    _dof_values_dotdot.resize(0);
    _dof_values_dot_old.resize(0);
    _dof_values_dotdot_old.resize(0);
    _dof_du_dot_du.resize(0);
    _dof_du_dotdot_du.resize(0);
  }
}

template <typename OutputType>
void
MooseVariableDataFV<OutputType>::prepareIC()
{
  // TODO: implement this function
  initDofIndices();
  _dof_values.resize(_dof_indices.size());

  mooseAssert(_qrule, "We should have a non-null qrule");
  const auto nqp = _qrule->n_points();
  _u.resize(nqp);
}

template class MooseVariableDataFV<Real>;
// TODO: implement vector fv variable support. This will require some template
// specializations for various member functions in this and the FV variable
// classes. And then you will need to uncomment out the line below:
// template class MooseVariableDataFV<RealVectorValue>;
