//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableData.h"
#include "Assembly.h"
#include "MooseError.h"
#include "DisplacedSystem.h"
#include "TimeIntegrator.h"
#include "MooseVariableFE.h"
#include "MooseTypes.h"

#include "libmesh/quadrature.h"
#include "libmesh/fe_base.h"
#include "libmesh/system.h"
#include "libmesh/type_n_tensor.h"

template <typename OutputType>
MooseVariableData<OutputType>::MooseVariableData(const MooseVariableFE<OutputType> & var,
                                                 const SystemBase & sys,
                                                 THREAD_ID tid,
                                                 Moose::ElementType element_type,
                                                 const QBase * const & qrule_in,
                                                 const QBase * const & qrule_face_in,
                                                 const Node * const & node,
                                                 const Elem * const & elem)

  : _var(var),
    _fe_type(_var.feType()),
    _var_num(_var.number()),
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
    _need_ad_u_dot(false),
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
    _need_ad(false),
    _need_ad_u(false),
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
    _has_dof_indices(false),
    _has_dof_values(false),
    _qrule(qrule_in),
    _qrule_face(qrule_face_in),
    _second_phi_assembly_method(nullptr),
    _second_phi_face_assembly_method(nullptr),
    _curl_phi_assembly_method(nullptr),
    _curl_phi_face_assembly_method(nullptr),
    _ad_grad_phi_assembly_method(nullptr),
    _ad_grad_phi_face_assembly_method(nullptr),
    _time_integrator(nullptr),
    _node(node),
    _elem(elem),
    _displaced(dynamic_cast<const DisplacedSystem *>(&_sys) ? true : false),
    _current_side(_assembly.side())
{
  // FIXME: continuity of FE type seems equivalent with the definition of nodal variables.
  //        Continuity does not depend on the FE dimension, so we just pass in a valid dimension.
  if (_fe_type.family == NEDELEC_ONE || _fe_type.family == LAGRANGE_VEC)
    _continuity = _assembly.getVectorFE(_fe_type, _sys.mesh().dimension())->get_continuity();
  else
    _continuity = _assembly.getFE(_fe_type, _sys.mesh().dimension())->get_continuity();

  _is_nodal = (_continuity == C_ZERO || _continuity == C_ONE);

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

  _time_integrator = _sys.getTimeIntegrator();

  // These MooseArray objects are used by AuxKernelBase for nodal AuxKernel objects, hence the size
  // size is always 1 (i.e, nodal kernels work with _qp=0 only).
  _nodal_value_array.resize(1);
  _nodal_value_old_array.resize(1);
  _nodal_value_older_array.resize(1);

  switch (_element_type)
  {
    case Moose::ElementType::Element:
    {
      _phi_assembly_method = &Assembly::fePhi<OutputType>;
      _phi_face_assembly_method = &Assembly::fePhiFace<OutputType>;
      _grad_phi_assembly_method = &Assembly::feGradPhi<OutputType>;
      _grad_phi_face_assembly_method = &Assembly::feGradPhiFace<OutputType>;
      _second_phi_assembly_method = &Assembly::feSecondPhi<OutputType>;
      _second_phi_face_assembly_method = &Assembly::feSecondPhiFace<OutputType>;
      _curl_phi_assembly_method = &Assembly::feCurlPhi<OutputType>;
      _curl_phi_face_assembly_method = &Assembly::feCurlPhiFace<OutputType>;
      _ad_grad_phi_assembly_method = &Assembly::feADGradPhi<OutputType>;
      _ad_grad_phi_face_assembly_method = &Assembly::feADGradPhiFace<OutputType>;

      _ad_grad_phi = &_ad_grad_phi_assembly_method(_assembly, _fe_type);
      _ad_grad_phi_face = &_ad_grad_phi_face_assembly_method(_assembly, _fe_type);
      break;
    }
    case Moose::ElementType::Neighbor:
    {
      _phi_assembly_method = &Assembly::fePhiNeighbor<OutputType>;
      _phi_face_assembly_method = &Assembly::fePhiFaceNeighbor<OutputType>;
      _grad_phi_assembly_method = &Assembly::feGradPhiNeighbor<OutputType>;
      _grad_phi_face_assembly_method = &Assembly::feGradPhiFaceNeighbor<OutputType>;
      _second_phi_assembly_method = &Assembly::feSecondPhiNeighbor<OutputType>;
      _second_phi_face_assembly_method = &Assembly::feSecondPhiFaceNeighbor<OutputType>;
      _curl_phi_assembly_method = &Assembly::feCurlPhiNeighbor<OutputType>;
      _curl_phi_face_assembly_method = &Assembly::feCurlPhiFaceNeighbor<OutputType>;

      _ad_grad_phi = nullptr;
      _ad_grad_phi_face = nullptr;
      break;
    }
    case Moose::ElementType::Lower:
    {
      _phi_assembly_method = &Assembly::fePhiLower<OutputType>;
      _phi_face_assembly_method = &Assembly::fePhiLower<OutputType>; // Place holder
      _grad_phi_assembly_method = &Assembly::feGradPhiLower<OutputType>;
      _grad_phi_face_assembly_method = &Assembly::feGradPhiLower<OutputType>; // Place holder

      _ad_grad_phi = nullptr;
      _ad_grad_phi_face = nullptr;
      break;
    }
  }
  _phi = &_phi_assembly_method(_assembly, _fe_type);
  _phi_face = &_phi_face_assembly_method(_assembly, _fe_type);
  _grad_phi = &_grad_phi_assembly_method(_assembly, _fe_type);
  _grad_phi_face = &_grad_phi_face_assembly_method(_assembly, _fe_type);
}

template <typename OutputType>
void
MooseVariableData<OutputType>::setGeometry(Moose::GeometryType gm_type)
{
  switch (gm_type)
  {
    case Moose::Volume:
    {
      _current_qrule = _qrule;
      _current_phi = _phi;
      _current_grad_phi = _grad_phi;
      _current_second_phi = _second_phi;
      _current_curl_phi = _curl_phi;
      _current_ad_grad_phi = _ad_grad_phi;
      break;
    }
    case Moose::Face:
    {
      _current_qrule = _qrule_face;
      _current_phi = _phi_face;
      _current_grad_phi = _grad_phi_face;
      _current_second_phi = _second_phi_face;
      _current_curl_phi = _curl_phi_face;
      _current_ad_grad_phi = _ad_grad_phi_face;
      break;
    }
  }
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::FieldVariableValue &
MooseVariableData<OutputType>::sln(Moose::SolutionState state) const
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
const typename MooseVariableData<OutputType>::FieldVariableValue &
MooseVariableData<OutputType>::uDot() const
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
const typename MooseVariableData<OutputType>::FieldVariableValue &
MooseVariableData<OutputType>::uDotDot() const
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
const typename MooseVariableData<OutputType>::FieldVariableValue &
MooseVariableData<OutputType>::uDotOld() const
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
const typename MooseVariableData<OutputType>::FieldVariableValue &
MooseVariableData<OutputType>::uDotDotOld() const
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
const typename MooseVariableData<OutputType>::FieldVariableGradient &
MooseVariableData<OutputType>::gradSln(Moose::SolutionState state) const
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
const typename MooseVariableData<OutputType>::FieldVariableGradient &
MooseVariableData<OutputType>::gradSlnDot() const
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
const typename MooseVariableData<OutputType>::FieldVariableGradient &
MooseVariableData<OutputType>::gradSlnDotDot() const
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
const typename MooseVariableData<OutputType>::FieldVariableSecond &
MooseVariableData<OutputType>::secondSln(Moose::SolutionState state) const
{
  secondPhi();
  secondPhiFace();
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
const typename MooseVariableData<OutputType>::FieldVariableCurl &
MooseVariableData<OutputType>::curlSln(Moose::SolutionState state) const
{
  curlPhi();
  curlPhiFace();
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
const typename MooseVariableData<OutputType>::FieldVariablePhiSecond &
MooseVariableData<OutputType>::secondPhi() const
{
  _second_phi = &_second_phi_assembly_method(_assembly, _fe_type);
  return *_second_phi;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::FieldVariablePhiSecond &
MooseVariableData<OutputType>::secondPhiFace() const
{
  _second_phi_face = &_second_phi_face_assembly_method(_assembly, _fe_type);
  return *_second_phi_face;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::FieldVariablePhiCurl &
MooseVariableData<OutputType>::curlPhi() const
{
  _curl_phi = &_curl_phi_assembly_method(_assembly, _fe_type);
  return *_curl_phi;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::FieldVariablePhiCurl &
MooseVariableData<OutputType>::curlPhiFace() const
{
  _curl_phi_face = &_curl_phi_face_assembly_method(_assembly, _fe_type);
  return *_curl_phi_face;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeValues()
{
  unsigned int num_dofs = _dof_indices.size();

  if (num_dofs > 0)
    fetchDoFValues();

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _current_qrule->n_points();
  auto && active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
  auto && active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);

  _u.resize(nqp);
  _grad_u.resize(nqp);

  for (auto tag : active_coupleable_vector_tags)
    if (_need_vector_tag_u[tag])
      _vector_tag_u[tag].resize(nqp);

  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
      _matrix_tag_u[tag].resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_curl)
    _curl_u.resize(nqp);

  if (_need_u_previous_nl)
    _u_previous_nl.resize(nqp);

  if (_need_grad_previous_nl)
    _grad_u_previous_nl.resize(nqp);

  if (_need_second_previous_nl)
    _second_u_previous_nl.resize(nqp);

  if (is_transient)
  {
    if (_need_u_dot)
      _u_dot.resize(nqp);

    if (_need_u_dotdot)
      _u_dotdot.resize(nqp);

    if (_need_u_dot_old)
      _u_dot_old.resize(nqp);

    if (_need_u_dotdot_old)
      _u_dotdot_old.resize(nqp);

    if (_need_du_dot_du)
      _du_dot_du.resize(nqp);

    if (_need_du_dotdot_du)
      _du_dotdot_du.resize(nqp);

    if (_need_grad_dot)
      _grad_u_dot.resize(nqp);

    if (_need_grad_dotdot)
      _grad_u_dotdot.resize(nqp);

    if (_need_u_old)
      _u_old.resize(nqp);

    if (_need_u_older)
      _u_older.resize(nqp);

    if (_need_grad_old)
      _grad_u_old.resize(nqp);

    if (_need_grad_older)
      _grad_u_older.resize(nqp);

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_curl_old)
      _curl_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i] = 0;
    _grad_u[i] = 0;

    for (auto tag : active_coupleable_vector_tags)
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][i] = 0;

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][i] = 0;

    if (_need_second)
      _second_u[i] = 0;

    if (_need_curl)
      _curl_u[i] = 0;

    if (_need_u_previous_nl)
      _u_previous_nl[i] = 0;

    if (_need_grad_previous_nl)
      _grad_u_previous_nl[i] = 0;

    if (_need_second_previous_nl)
      _second_u_previous_nl[i] = 0;

    if (is_transient)
    {
      if (_need_u_dot)
        _u_dot[i] = 0;

      if (_need_u_dotdot)
        _u_dotdot[i] = 0;

      if (_need_u_dot_old)
        _u_dot_old[i] = 0;

      if (_need_u_dotdot_old)
        _u_dotdot_old[i] = 0;

      if (_need_du_dot_du)
        _du_dot_du[i] = 0;

      if (_need_du_dotdot_du)
        _du_dotdot_du[i] = 0;

      if (_need_grad_dot)
        _grad_u_dot[i] = 0;

      if (_need_grad_dotdot)
        _grad_u_dotdot[i] = 0;

      if (_need_u_old)
        _u_old[i] = 0;

      if (_need_u_older)
        _u_older[i] = 0;

      if (_need_grad_old)
        _grad_u_old[i] = 0;

      if (_need_grad_older)
        _grad_u_older[i] = 0;

      if (_need_second_old)
        _second_u_old[i] = 0;

      if (_need_second_older)
        _second_u_older[i] = 0;

      if (_need_curl_old)
        _curl_u_old[i] = 0;
    }
  }

  bool second_required =
      _need_second || _need_second_old || _need_second_older || _need_second_previous_nl;
  bool curl_required = _need_curl || _need_curl_old;

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      const OutputType phi_local = (*_current_phi)[i][qp];
      const typename OutputTools<OutputType>::OutputGradient dphi_qp = (*_current_grad_phi)[i][qp];

      _u[qp] += phi_local * _dof_values[i];

      _grad_u[qp].add_scaled(dphi_qp, _dof_values[i]);

      if (is_transient)
      {
        if (_need_u_old)
          _u_old[qp] += phi_local * _dof_values_old[i];

        if (_need_u_older)
          _u_older[qp] += phi_local * _dof_values_older[i];

        if (_need_grad_old)
          _grad_u_old[qp].add_scaled(dphi_qp, _dof_values_old[i]);

        if (_need_grad_older)
          _grad_u_older[qp].add_scaled(dphi_qp, _dof_values_older[i]);

        if (_need_u_dot)
          _u_dot[qp] += phi_local * _dof_values_dot[i];

        if (_need_u_dotdot)
          _u_dotdot[qp] += phi_local * _dof_values_dotdot[i];

        if (_need_u_dot_old)
          _u_dot_old[qp] += phi_local * _dof_values_dot_old[i];

        if (_need_u_dotdot_old)
          _u_dotdot_old[qp] += phi_local * _dof_values_dotdot_old[i];

        if (_need_grad_dot)
          _grad_u_dot[qp].add_scaled(dphi_qp, _dof_values_dot[i]);

        if (_need_grad_dotdot)
          _grad_u_dotdot[qp].add_scaled(dphi_qp, _dof_values_dotdot[i]);

        if (_need_du_dot_du)
          _du_dot_du[qp] = _dof_du_dot_du[i];

        if (_need_du_dotdot_du)
          _du_dotdot_du[qp] = _dof_du_dotdot_du[i];
      }

      if (second_required)
      {
        mooseAssert(
            _current_second_phi,
            "We're requiring a second calculation but have not set a second shape function!");
        const typename OutputTools<OutputType>::OutputSecond d2phi_local =
            (*_current_second_phi)[i][qp];

        if (_need_second)
          _second_u[qp].add_scaled(d2phi_local, _dof_values[i]);

        if (_need_second_previous_nl)
          _second_u_previous_nl[qp].add_scaled(d2phi_local, _dof_values_previous_nl[i]);

        if (is_transient)
        {
          if (_need_second_old)
            _second_u_old[qp].add_scaled(d2phi_local, _dof_values_old[i]);

          if (_need_second_older)
            _second_u_older[qp].add_scaled(d2phi_local, _dof_values_older[i]);
        }
      }

      if (curl_required)
      {
        mooseAssert(_current_curl_phi,
                    "We're requiring a curl calculation but have not set a curl shape function!");
        const OutputType curl_phi_local = (*_current_curl_phi)[i][qp];

        if (_need_curl)
          _curl_u[qp] += curl_phi_local * _dof_values[i];

        if (is_transient && _need_curl_old)
          _curl_u_old[qp] += curl_phi_local * _dof_values_old[i];
      }

      for (auto tag : active_coupleable_vector_tags)
        if (_need_vector_tag_u[tag])
          _vector_tag_u[tag][qp] += phi_local * _vector_tags_dof_u[tag][i];

      for (auto tag : active_coupleable_matrix_tags)
        if (_need_matrix_tag_u[tag])
          _matrix_tag_u[tag][qp] += phi_local * _matrix_tags_dof_u[tag][i];

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += phi_local * _dof_values_previous_nl[i];

      if (_need_grad_previous_nl)
        _grad_u_previous_nl[qp].add_scaled(dphi_qp, _dof_values_previous_nl[i]);
    }
  }

  // Automatic differentiation
  if (_need_ad && _subproblem.currentlyComputingJacobian())
    computeAD(num_dofs, nqp);
}

template <>
void
MooseVariableData<RealEigenVector>::computeValues()
{
  unsigned int num_dofs = _dof_indices.size();

  if (num_dofs > 0)
    fetchDoFValues();

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _current_qrule->n_points();
  auto && active_coupleable_vector_tags =
      _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
  auto && active_coupleable_matrix_tags =
      _sys.subproblem().getActiveFEVariableCoupleableMatrixTags(_tid);

  // Map grad_phi using Eigen so that we can perform array operations easier
  if (_qrule == _current_qrule)
  {
    _mapped_grad_phi.resize(num_dofs);
    for (unsigned int i = 0; i < num_dofs; i++)
    {
      _mapped_grad_phi[i].resize(nqp, Eigen::Map<RealDIMValue>(nullptr));
      for (unsigned int qp = 0; qp < nqp; qp++)
        // Note: this does NOT do any allocation.  It is "reconstructing" the object in place
        new (&_mapped_grad_phi[i][qp])
            Eigen::Map<RealDIMValue>(const_cast<Real *>(&(*_current_grad_phi)[i][qp](0)));
    }
  }
  else
  {
    _mapped_grad_phi_face.resize(num_dofs);
    for (unsigned int i = 0; i < num_dofs; i++)
    {
      _mapped_grad_phi_face[i].resize(nqp, Eigen::Map<RealDIMValue>(nullptr));
      for (unsigned int qp = 0; qp < nqp; qp++)
        // Note: this does NOT do any allocation.  It is "reconstructing" the object in place
        new (&_mapped_grad_phi_face[i][qp])
            Eigen::Map<RealDIMValue>(const_cast<Real *>(&(*_current_grad_phi)[i][qp](0)));
    }
  }

  _u.resize(nqp);
  _grad_u.resize(nqp);

  for (auto tag : active_coupleable_vector_tags)
    if (_need_vector_tag_u[tag])
      _vector_tag_u[tag].resize(nqp);

  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
      _matrix_tag_u[tag].resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_curl)
    _curl_u.resize(nqp);

  if (_need_u_previous_nl)
    _u_previous_nl.resize(nqp);

  if (_need_grad_previous_nl)
    _grad_u_previous_nl.resize(nqp);

  if (_need_second_previous_nl)
    _second_u_previous_nl.resize(nqp);

  if (is_transient)
  {
    if (_need_u_dot)
      _u_dot.resize(nqp);

    if (_need_u_dotdot)
      _u_dotdot.resize(nqp);

    if (_need_u_dot_old)
      _u_dot_old.resize(nqp);

    if (_need_u_dotdot_old)
      _u_dotdot_old.resize(nqp);

    if (_need_du_dot_du)
      _du_dot_du.resize(nqp);

    if (_need_du_dotdot_du)
      _du_dotdot_du.resize(nqp);

    if (_need_grad_dot)
      _grad_u_dot.resize(nqp);

    if (_need_grad_dotdot)
      _grad_u_dotdot.resize(nqp);

    if (_need_u_old)
      _u_old.resize(nqp);

    if (_need_u_older)
      _u_older.resize(nqp);

    if (_need_grad_old)
      _grad_u_old.resize(nqp);

    if (_need_grad_older)
      _grad_u_older.resize(nqp);

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_curl_old)
      _curl_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    _u[i].setZero(_count);
    _grad_u[i].setZero(_count, LIBMESH_DIM);

    for (auto tag : active_coupleable_vector_tags)
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][i].setZero(_count);

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][i].setZero(_count);

    if (_need_second)
      _second_u[i].setZero(_count, LIBMESH_DIM * LIBMESH_DIM);

    if (_need_curl)
      _curl_u[i].setZero(_count);

    if (_need_u_previous_nl)
      _u_previous_nl[i].setZero(_count);

    if (_need_grad_previous_nl)
      _grad_u_previous_nl[i].setZero(_count, LIBMESH_DIM);

    if (_need_second_previous_nl)
      _second_u_previous_nl[i].setZero(_count, LIBMESH_DIM * LIBMESH_DIM);

    if (is_transient)
    {
      if (_need_u_dot)
        _u_dot[i].setZero(_count);

      if (_need_u_dotdot)
        _u_dotdot[i].setZero(_count);

      if (_need_u_dot_old)
        _u_dot_old[i].setZero(_count);

      if (_need_u_dotdot_old)
        _u_dotdot_old[i].setZero(_count);

      if (_need_du_dot_du)
        _du_dot_du[i] = 0;

      if (_need_du_dotdot_du)
        _du_dotdot_du[i] = 0;

      if (_need_grad_dot)
        _grad_u_dot[i].setZero(_count, LIBMESH_DIM);

      if (_need_grad_dotdot)
        _grad_u_dotdot[i].setZero(_count, LIBMESH_DIM);

      if (_need_u_old)
        _u_old[i].setZero(_count);

      if (_need_u_older)
        _u_older[i].setZero(_count);

      if (_need_grad_old)
        _grad_u_old[i].setZero(_count, LIBMESH_DIM);

      if (_need_grad_older)
        _grad_u_older[i].setZero(_count, LIBMESH_DIM);

      if (_need_second_old)
        _second_u_old[i].setZero(_count, LIBMESH_DIM * LIBMESH_DIM);

      if (_need_second_older)
        _second_u_older[i].setZero(_count, LIBMESH_DIM * LIBMESH_DIM);

      if (_need_curl_old)
        _curl_u_old[i].setZero(_count);
    }
  }

  bool second_required =
      _need_second || _need_second_old || _need_second_older || _need_second_previous_nl;
  bool curl_required = _need_curl || _need_curl_old;

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      const OutputShape phi_local = (*_current_phi)[i][qp];
      const OutputShapeGradient dphi_qp = (*_current_grad_phi)[i][qp];

      _u[qp] += phi_local * _dof_values[i];

      for (unsigned int d = 0; d < LIBMESH_DIM; ++d)
        _grad_u[qp].col(d) += dphi_qp(d) * _dof_values[i];

      if (is_transient)
      {
        if (_need_u_old)
          _u_old[qp] += phi_local * _dof_values_old[i];

        if (_need_u_older)
          _u_older[qp] += phi_local * _dof_values_older[i];

        if (_need_grad_old)
          for (unsigned int d = 0; d < LIBMESH_DIM; ++d)
            _grad_u_old[qp].col(d) += dphi_qp(d) * _dof_values_old[i];

        if (_need_grad_older)
          for (unsigned int d = 0; d < LIBMESH_DIM; ++d)
            _grad_u_older[qp].col(d) += dphi_qp(d) * _dof_values_older[i];

        if (_need_u_dot)
          _u_dot[qp] += phi_local * _dof_values_dot[i];

        if (_need_u_dotdot)
          _u_dotdot[qp] += phi_local * _dof_values_dotdot[i];

        if (_need_u_dot_old)
          _u_dot_old[qp] += phi_local * _dof_values_dot_old[i];

        if (_need_u_dotdot_old)
          _u_dotdot_old[qp] += phi_local * _dof_values_dotdot_old[i];

        if (_need_grad_dot)
          for (unsigned int d = 0; d < LIBMESH_DIM; ++d)
            _grad_u_dot[qp].col(d) += dphi_qp(d) * _dof_values_dot[i];

        if (_need_grad_dotdot)
          for (unsigned int d = 0; d < LIBMESH_DIM; ++d)
            _grad_u_dotdot[qp].col(d) += dphi_qp(d) * _dof_values_dotdot[i];

        if (_need_du_dot_du)
          _du_dot_du[qp] = _dof_du_dot_du[i];

        if (_need_du_dotdot_du)
          _du_dotdot_du[qp] = _dof_du_dotdot_du[i];
      }

      if (second_required)
      {
        mooseAssert(
            _current_second_phi,
            "We're requiring a second calculation but have not set a second shape function!");
        const RealTensorValue d2phi_local = (*_current_second_phi)[i][qp];

        if (_need_second)
          for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
            for (unsigned int d2 = 0; d2 < LIBMESH_DIM; ++d2)
              _second_u[qp].col(d++) += d2phi_local(d1, d2) * _dof_values[i];

        if (_need_second_previous_nl)
          for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
            for (unsigned int d2 = 0; d2 < LIBMESH_DIM; ++d2)
              _second_u_previous_nl[qp].col(d++) +=
                  d2phi_local(d1, d2) * _dof_values_previous_nl[i];

        if (is_transient)
        {
          if (_need_second_old)
            for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
              for (unsigned int d2 = 0; d2 < LIBMESH_DIM; ++d2)
                _second_u_old[qp].col(d++) += d2phi_local(d1, d2) * _dof_values_old[i];

          if (_need_second_older)
            for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
              for (unsigned int d2 = 0; d2 < LIBMESH_DIM; ++d2)
                _second_u_older[qp].col(d++) += d2phi_local(d1, d2) * _dof_values_older[i];
        }
      }

      if (curl_required)
      {
        mooseAssert(_current_curl_phi,
                    "We're requiring a curl calculation but have not set a curl shape function!");
        const auto curl_phi_local = (*_current_curl_phi)[i][qp];

        if (_need_curl)
          _curl_u[qp] += curl_phi_local * _dof_values[i];

        if (is_transient && _need_curl_old)
          _curl_u_old[qp] += curl_phi_local * _dof_values_old[i];
      }

      for (auto tag : active_coupleable_vector_tags)
        if (_need_vector_tag_u[tag])
          _vector_tag_u[tag][qp] += phi_local * _vector_tags_dof_u[tag][i];

      for (auto tag : active_coupleable_matrix_tags)
        if (_need_matrix_tag_u[tag])
          _matrix_tag_u[tag][qp] += phi_local * _matrix_tags_dof_u[tag][i];

      if (_need_u_previous_nl)
        _u_previous_nl[qp] += phi_local * _dof_values_previous_nl[i];

      if (_need_grad_previous_nl)
        for (unsigned int d = 0; d < LIBMESH_DIM; ++d)
          _grad_u_previous_nl[qp].col(d) += dphi_qp(d) * _dof_values_previous_nl[i];
    }
  }
  // No AD support for array variable yet.
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeMonomialValues()
{
  if (_dof_indices.size() == 0)
    return;

  bool is_transient = _subproblem.isTransient();
  unsigned int nqp = _current_qrule->n_points();

  _u.resize(nqp);
  _grad_u.resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_u_previous_nl)
    _u_previous_nl.resize(nqp);

  if (_need_grad_previous_nl)
    _grad_u_previous_nl.resize(nqp);

  if (_need_second_previous_nl)
    _second_u_previous_nl.resize(nqp);

  if (is_transient)
  {
    if (_need_u_dot)
      _u_dot.resize(nqp);

    if (_need_u_dotdot)
      _u_dotdot.resize(nqp);

    if (_need_u_dot_old)
      _u_dot_old.resize(nqp);

    if (_need_u_dotdot_old)
      _u_dotdot_old.resize(nqp);

    if (_need_du_dot_du)
      _du_dot_du.resize(nqp);

    if (_need_du_dotdot_du)
      _du_dotdot_du.resize(nqp);

    if (_need_u_old)
      _u_old.resize(nqp);

    if (_need_u_older)
      _u_older.resize(nqp);

    if (_need_grad_old)
      _grad_u_old.resize(nqp);

    if (_need_grad_older)
      _grad_u_older.resize(nqp);

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  if (_need_dof_values)
    _dof_values.resize(1);

  if (_need_dof_values_previous_nl)
    _dof_values_previous_nl.resize(1);

  if (is_transient)
  {
    if (_need_dof_values_old)
      _dof_values_old.resize(1);
    if (_need_dof_values_older)
      _dof_values_older.resize(1);
    if (_need_dof_values_dot)
      _dof_values_dot.resize(1);
    if (_need_dof_values_dotdot)
      _dof_values_dotdot.resize(1);
    if (_need_dof_values_dot_old)
      _dof_values_dot_old.resize(1);
    if (_need_dof_values_dotdot_old)
      _dof_values_dotdot_old.resize(1);
  }

  const dof_id_type & idx = _dof_indices[0];
  const Real & soln = (*_sys.currentSolution())(idx);
  Real soln_old = 0;
  Real soln_older = 0;
  Real soln_previous_nl = 0;
  Real u_dot = 0;
  Real u_dotdot = 0;
  Real u_dot_old = 0;
  Real u_dotdot_old = 0;
  const Real & du_dot_du = _sys.duDotDu();
  const Real & du_dotdot_du = _sys.duDotDotDu();

  if (_need_dof_values)
    _dof_values[0] = soln;

  if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
      _need_dof_values_previous_nl)
    soln_previous_nl = (*_sys.solutionPreviousNewton())(idx);

  if (_need_dof_values_previous_nl)
    _dof_values_previous_nl[0] = soln_previous_nl;

  if (is_transient)
  {
    if (_need_u_old || _need_grad_old || _need_second_old || _need_dof_values_old)
      soln_old = _sys.solutionOld()(idx);

    if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_values_older)
      soln_older = _sys.solutionOlder()(idx);

    if (_need_dof_values_old)
      _dof_values_old[0] = soln_old;

    if (_need_dof_values_older)
      _dof_values_older[0] = soln_older;

    if (_sys.solutionUDot())
      u_dot = (*_sys.solutionUDot())(idx);
    if (_sys.solutionUDotDot())
      u_dotdot = (*_sys.solutionUDotDot())(idx);
    if (_sys.solutionUDotOld())
      u_dot_old = (*_sys.solutionUDotOld())(idx);
    if (_sys.solutionUDotDotOld())
      u_dotdot_old = (*_sys.solutionUDotDotOld())(idx);

    if (_need_dof_values_dot)
      _dof_values_dot[0] = u_dot;

    if (_need_dof_values_dotdot)
      _dof_values_dotdot[0] = u_dotdot;
  }

  auto phi = (*_current_phi)[0][0];

  _u[0] = phi * soln;

  if (_need_u_previous_nl)
    _u_previous_nl[0] = phi * soln_previous_nl;

  if (is_transient)
  {
    if (_need_u_dot)
      _u_dot[0] = phi * u_dot;

    if (_need_u_dotdot)
      _u_dotdot[0] = phi * u_dotdot;

    if (_need_u_dot_old)
      _u_dot_old[0] = phi * u_dot_old;

    if (_need_u_dotdot_old)
      _u_dotdot_old[0] = phi * u_dotdot_old;

    if (_need_du_dot_du)
      _du_dot_du[0] = du_dot_du;

    if (_need_du_dotdot_du)
      _du_dotdot_du[0] = du_dotdot_du;

    if (_need_u_old)
      _u_old[0] = phi * soln_old;

    if (_need_u_older)
      _u_older[0] = phi * soln_older;
  }

  for (unsigned qp = 1; qp < nqp; ++qp)
  {
    _u[qp] = _u[0];

    if (_need_u_previous_nl)
      _u_previous_nl[qp] = _u_previous_nl[0];

    if (is_transient)
    {
      if (_need_u_dot)
        _u_dot[qp] = _u_dot[0];

      if (_need_u_dotdot)
        _u_dotdot[qp] = _u_dotdot[0];

      if (_need_u_dot_old)
        _u_dot_old[qp] = _u_dot_old[0];

      if (_need_u_dotdot_old)
        _u_dotdot_old[qp] = _u_dotdot_old[0];

      if (_need_du_dot_du)
        _du_dot_du[qp] = _du_dot_du[0];

      if (_need_du_dotdot_du)
        _du_dotdot_du[qp] = _du_dotdot_du[0];

      if (_need_u_old)
        _u_old[qp] = _u_old[0];

      if (_need_u_older)
        _u_older[qp] = _u_older[qp];
    }
  }
}

template <>
void
MooseVariableData<RealEigenVector>::computeMonomialValues()
{
  // Fixeme: will think of optimization later
  computeValues();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeAD(const unsigned int num_dofs, const unsigned int nqp)
{
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

  // Derivatives are offset by the variable number
  size_t ad_offset;
  switch (_element_type)
  {
    case Moose::ElementType::Element:
      ad_offset = _var_num * _sys.getMaxVarNDofsPerElem();
      break;

    case Moose::ElementType::Neighbor:
      ad_offset = _var_num * _sys.getMaxVarNDofsPerElem() +
                  (_sys.system().n_vars() * _sys.getMaxVarNDofsPerElem());
      break;

    case Moose::ElementType::Lower:
      // At the time of writing, this Lower case is only used in mortar applications where we are
      // re-init'ing on Element, Neighbor, and Lower dimensional elements
      // simultaneously. Consequently, we make sure here that our offset is greater than the sum of
      // the element and neighbor dofs. I can imagine a future case in which you're not re-init'ing
      // on all three simultaneously in which case this offset could be smaller. Also note that the
      // number of dofs on lower-d elements is guaranteed to be lower than on the higher dimensional
      // element, but we're not using that knowledge here. In the future we could implement
      // something like SystemBase::getMaxVarNDofsPerFace (or *PerLowerDElem)
      ad_offset = 2 * _sys.system().n_vars() * _sys.getMaxVarNDofsPerElem() +
                  _var_num * _sys.getMaxVarNDofsPerElem();
      break;

    default:
      mooseError(
          "Unsupported element type ",
          static_cast<typename std::underlying_type<decltype(_element_type)>::type>(_element_type));
  }
  mooseAssert(_var.kind() == Moose::VarKindType::VAR_AUXILIARY || ad_offset || !_var_num,
              "Either this is the zeroth variable or we should have an offset");

  // Hopefully this problem can go away at some point
  if (ad_offset + num_dofs > AD_MAX_DOFS_PER_ELEM)
    mooseError("Current number of dofs per element is greater than AD_MAX_DOFS_PER_ELEM of ",
               AD_MAX_DOFS_PER_ELEM);

  for (unsigned int qp = 0; qp < nqp; qp++)
  {
    if (_need_ad_u)
      _ad_u[qp] = _ad_zero;

    if (_need_ad_grad_u)
      _ad_grad_u[qp] = _ad_zero;

    if (_need_ad_second_u)
      _ad_second_u[qp] = _ad_zero;

    if (_need_ad_u_dot)
      _ad_u_dot[qp] = _ad_zero;
  }

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (_var.kind() == Moose::VAR_NONLINEAR)
      _ad_dof_values[i].derivatives().insert(ad_offset + i) = 1.0;

    if (_need_ad_u_dot && _time_integrator)
    {
      _ad_dofs_dot[i] = _ad_dof_values[i];
      _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i], _dof_indices[i]);
    }
  }

  // Now build up the solution at each quadrature point:
  for (unsigned int i = 0; i < num_dofs; i++)
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      if (_need_ad_u)
        _ad_u[qp] += _ad_dof_values[i] * (*_current_phi)[i][qp];

      if (_need_ad_grad_u)
      {
        // The latter check here is for handling the fact that we have not yet implemented
        // calculation of ad_grad_phi for neighbor and neighbor-face, so if we are in that situation
        // we need to default to using the non-ad grad_phi
        if (_displaced && _current_ad_grad_phi)
          _ad_grad_u[qp] += _ad_dof_values[i] * (*_current_ad_grad_phi)[i][qp];
        else
          _ad_grad_u[qp] += _ad_dof_values[i] * (*_current_grad_phi)[i][qp];
      }

      if (_need_ad_second_u)
      {
        if (_displaced)
          mooseError("Support for second order shape function derivatives on the displaced mesh "
                     "has not been added yet!");
        else
          _ad_second_u[qp] += _ad_dof_values[i] * (*_current_second_phi)[i][qp];
      }

      if (_need_ad_u_dot && _time_integrator)
        _ad_u_dot[qp] += (*_current_phi)[i][qp] * _ad_dofs_dot[i];
    }
  }

  if (_need_ad_u_dot && !_time_integrator)
    for (MooseIndex(nqp) qp = 0; qp < nqp; ++qp)
      _ad_u_dot[qp] = _u_dot[qp];
}

template <>
void
MooseVariableData<RealEigenVector>::computeAD(const unsigned int /*num_dofs*/,
                                              const unsigned int /*nqp*/)
{
  mooseError("AD for array variable has not been implemented");
}

template <typename OutputType>
void
MooseVariableData<OutputType>::setNodalValue(const OutputType & value, unsigned int idx)
{
  _dof_values[idx] = value; // update variable nodal value
  _has_dof_values = true;
  _nodal_value = value;

  // Update the qp values as well
  for (unsigned int qp = 0; qp < _u.size(); qp++)
    _u[qp] = value;
}

template <>
void
MooseVariableData<RealVectorValue>::setNodalValue(const RealVectorValue & value, unsigned int idx)
{
  for (decltype(idx) i = 0; i < _dof_values.size(); ++i, ++idx)
    _dof_values[idx] = value(i);

  _has_dof_values = true;
  _nodal_value = value;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::setDofValues(const DenseVector<OutputData> & values)
{
  for (unsigned int i = 0; i < values.size(); i++)
    _dof_values[i] = values(i);

  _has_dof_values = true;

  for (unsigned int qp = 0; qp < _u.size(); qp++)
  {
    _u[qp] = (*_phi)[0][qp] * _dof_values[0];
    for (unsigned int i = 1; i < _dof_values.size(); i++)
      _u[qp] += (*_phi)[i][qp] * _dof_values[i];
  }
}

template <typename OutputType>
void
MooseVariableData<OutputType>::insertNodalValue(NumericVector<Number> & residual,
                                                const OutputData & v)
{
  residual.set(_nodal_dof_index, v);
}

template <>
void
MooseVariableData<RealEigenVector>::insertNodalValue(NumericVector<Number> & residual,
                                                     const RealEigenVector & v)
{
  for (unsigned int j = 0; j < _count; ++j)
    residual.set(_nodal_dof_index + j, v(j));
}

template <typename OutputType>
typename MooseVariableData<OutputType>::OutputData
MooseVariableData<OutputType>::getNodalValue(const Node & node, Moose::SolutionState state) const
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num) << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);

  switch (state)
  {
    case Moose::Current:
      return (*_sys.currentSolution())(dof);

    case Moose::Old:
      return _sys.solutionOld()(dof);

    case Moose::Older:
      return _sys.solutionOlder()(dof);

    default:
      mooseError("PreviousNL not currently supported for getNodalValue");
  }
}

template <>
RealEigenVector
MooseVariableData<RealEigenVector>::getNodalValue(const Node & node,
                                                  Moose::SolutionState state) const
{
  mooseAssert(_subproblem.mesh().isSemiLocal(const_cast<Node *>(&node)), "Node is not Semilocal");

  // Make sure that the node has DOFs
  /* Note, this is a reproduction of an assert within libMesh::Node::dof_number, this is done to
   * produce a better error (see misc/check_error.node_value_off_block) */
  mooseAssert(node.n_dofs(_sys.number(), _var_num) > 0,
              "Node " << node.id() << " does not contain any dofs for the "
                      << _sys.system().variable_name(_var_num) << " variable");

  dof_id_type dof = node.dof_number(_sys.number(), _var_num, 0);

  RealEigenVector v(_count);
  switch (state)
  {
    case Moose::Current:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = (*_sys.currentSolution())(dof++);
      break;

    case Moose::Old:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = _sys.solutionOld()(dof++);
      break;

    case Moose::Older:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = _sys.solutionOlder()(dof++);
      break;

    default:
      mooseError("PreviousNL not currently supported for getNodalValue");
  }
  return v;
}

template <typename OutputType>
typename MooseVariableData<OutputType>::OutputData
MooseVariableData<OutputType>::getElementalValue(const Elem * elem,
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

template <>
RealEigenVector
MooseVariableData<RealEigenVector>::getElementalValue(const Elem * elem,
                                                      Moose::SolutionState state,
                                                      unsigned int idx) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);

  dof_id_type dof = dof_indices[idx];

  RealEigenVector v(_count);

  switch (state)
  {
    case Moose::Current:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = (*_sys.currentSolution())(dof++);
      break;

    case Moose::Old:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = _sys.solutionOld()(dof++);
      break;

    case Moose::Older:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = _sys.solutionOlder()(dof++);
      break;

    default:
      mooseError("PreviousNL not currently supported for getElementalValue");
  }
  return v;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::getDofIndices(const Elem * elem,
                                             std::vector<dof_id_type> & dof_indices) const
{
  _dof_map.dof_indices(elem, dof_indices, _var_num);
}

template <typename OutputType>
void
MooseVariableData<OutputType>::insert(NumericVector<Number> & residual)
{
  if (_has_dof_values)
    residual.insert(&_dof_values[0], _dof_indices);
}

template <>
void
MooseVariableData<RealEigenVector>::insert(NumericVector<Number> & residual)
{
  if (_has_dof_values)
  {
    if (isNodal())
    {
      for (unsigned int i = 0; i < _dof_indices.size(); ++i)
        for (unsigned int j = 0; j < _count; ++j)
          residual.set(_dof_indices[i] + j, _dof_values[i](j));
    }
    else
    {
      unsigned int n = 0;
      for (unsigned int j = 0; j < _count; ++j)
      {
        for (unsigned int i = 0; i < _dof_indices.size(); ++i)
          residual.set(_dof_indices[i] + n, _dof_values[i](j));
        n += _dof_indices.size();
      }
    }
  }
}

template <typename OutputType>
void
MooseVariableData<OutputType>::add(NumericVector<Number> & residual)
{
  if (_has_dof_values)
    residual.add_vector(&_dof_values[0], _dof_indices);
}

template <>
void
MooseVariableData<RealEigenVector>::add(NumericVector<Number> & residual)
{
  if (_has_dof_values)
  {
    if (isNodal())
    {
      for (unsigned int i = 0; i < _dof_indices.size(); ++i)
        for (unsigned int j = 0; j < _count; ++j)
          residual.add(_dof_indices[i] + j, _dof_values[i](j));
    }
    else
    {
      unsigned int n = 0;
      for (unsigned int j = 0; j < _count; ++j)
      {
        for (unsigned int i = 0; i < _dof_indices.size(); ++i)
          residual.add(_dof_indices[i] + n, _dof_values[i](j));
        n += _dof_indices.size();
      }
    }
  }
}

template <typename OutputType>
void
MooseVariableData<OutputType>::addSolution(NumericVector<Number> & sol,
                                           const DenseVector<Number> & v) const
{
  sol.add_vector(v, _dof_indices);
}

template <>
void
MooseVariableData<RealEigenVector>::addSolution(NumericVector<Number> & sol,
                                                const DenseVector<Number> & v) const
{
  unsigned int p = 0;
  for (unsigned int j = 0; j < _count; ++j)
  {
    unsigned int inc = (isNodal() ? j : j * _dof_indices.size());
    for (unsigned int i = 0; i < _dof_indices.size(); ++i)
      sol.add(_dof_indices[i] + inc, v(p++));
  }
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValues() const
{
  _need_dof_values = true;
  return _dof_values;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValuesOld() const
{
  _need_dof_values_old = true;
  return _dof_values_old;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValuesOlder() const
{
  _need_dof_values_older = true;
  return _dof_values_older;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValuesPreviousNL() const
{
  _need_dof_values_previous_nl = true;
  return _dof_values_previous_nl;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValuesDot() const
{
  if (_sys.solutionUDot())
  {
    _need_dof_values_dot = true;
    return _dof_values_dot;
  }
  else
    mooseError("MooseVariableData: Time derivative of solution (`u_dot`) is not stored. Please set "
               "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValuesDotDot() const
{
  if (_sys.solutionUDotDot())
  {
    _need_dof_values_dotdot = true;
    return _dof_values_dotdot;
  }
  else
    mooseError("MooseVariableData: Second time derivative of solution (`u_dotdot`) is not stored. "
               "Please set uDotDotRequested() to true in FEProblemBase before requesting "
               "`u_dotdot`.");
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValuesDotOld() const
{
  if (_sys.solutionUDotOld())
  {
    _need_dof_values_dot_old = true;
    return _dof_values_dot_old;
  }
  else
    mooseError("MooseVariableData: Old time derivative of solution (`u_dot_old`) is not stored. "
               "Please set uDotOldRequested() to true in FEProblemBase before requesting "
               "`u_dot_old`.");
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::dofValuesDotDotOld() const
{
  if (_sys.solutionUDotDotOld())
  {
    _need_dof_values_dotdot_old = true;
    return _dof_values_dotdot_old;
  }
  else
    mooseError("MooseVariableData: Old second time derivative of solution (`u_dotdot_old`) is not "
               "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
               "requesting `u_dotdot_old`.");
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableData<OutputType>::dofValuesDuDotDu() const
{
  _need_dof_du_dot_du = true;
  return _dof_du_dot_du;
}

template <typename OutputType>
const MooseArray<Number> &
MooseVariableData<OutputType>::dofValuesDuDotDotDu() const
{
  _need_dof_du_dotdot_du = true;
  return _dof_du_dotdot_du;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeIncrementAtQps(const NumericVector<Number> & increment_vec)
{
  unsigned int nqp = _qrule->n_points();

  _increment.resize(nqp);
  // Compute the increment at each quadrature point
  unsigned int num_dofs = _dof_indices.size();
  for (unsigned int qp = 0; qp < nqp; qp++)
  {
    _increment[qp] = 0;
    for (unsigned int i = 0; i < num_dofs; i++)
      _increment[qp] += (*_phi)[i][qp] * increment_vec(_dof_indices[i]);
  }
}

template <>
void
MooseVariableData<RealEigenVector>::computeIncrementAtQps(
    const NumericVector<Number> & increment_vec)
{
  unsigned int nqp = _qrule->n_points();

  _increment.resize(nqp);
  // Compute the increment at each quadrature point
  unsigned int num_dofs = _dof_indices.size();
  if (isNodal())
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      for (unsigned int i = 0; i < num_dofs; i++)
        for (unsigned int j = 0; j < _count; j++)
          _increment[qp](j) += (*_phi)[i][qp] * increment_vec(_dof_indices[i] + j);
    }
  }
  else
  {
    for (unsigned int qp = 0; qp < nqp; qp++)
    {
      unsigned int n = 0;
      for (unsigned int j = 0; j < _count; j++)
        for (unsigned int i = 0; i < num_dofs; i++)
        {
          _increment[qp](j) += (*_phi)[i][qp] * increment_vec(_dof_indices[i] + n);
          n += num_dofs;
        }
    }
  }
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeIncrementAtNode(const NumericVector<Number> & increment_vec)
{
  if (!isNodal())
    mooseError("computeIncrementAtNode can only be called for nodal variables");

  _increment.resize(1);

  // Compute the increment for the current DOF
  _increment[0] = increment_vec(_dof_indices[0]);
}

template <>
void
MooseVariableData<RealEigenVector>::computeIncrementAtNode(
    const NumericVector<Number> & increment_vec)
{
  if (!isNodal())
    mooseError("computeIncrementAtNode can only be called for nodal variables");

  _increment.resize(1);

  // Compute the increment for the current DOF
  if (isNodal())
    for (unsigned int j = 0; j < _count; j++)
      _increment[0](j) = increment_vec(_dof_indices[0] + j);
  else
  {
    unsigned int n = 0;
    for (unsigned int j = 0; j < _count; j++)
    {
      _increment[0](j) = increment_vec(_dof_indices[0] + n);
      n += _dof_indices.size();
    }
  }
}

template <typename OutputType>
const OutputType &
MooseVariableData<OutputType>::nodalValue(Moose::SolutionState state) const
{
  if (isNodal())
  {
    switch (state)
    {
      case Moose::Current:
      {
        _need_dof_values = true;
        return _nodal_value;
      }

      case Moose::Old:
      {
        _need_dof_values_old = true;
        return _nodal_value_old;
      }

      case Moose::Older:
      {
        _need_dof_values_older = true;
        return _nodal_value_older;
      }

      case Moose::PreviousNL:
      {
        _need_dof_values_previous_nl = true;
        return _nodal_value_previous_nl;
      }

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
MooseVariableData<OutputType>::nodalValueArray(Moose::SolutionState state) const
{
  if (isNodal())
  {
    switch (state)
    {
      case Moose::Current:
      {
        _need_dof_values = true;
        return _nodal_value_array;
      }

      case Moose::Old:
      {
        _need_dof_values_old = true;
        return _nodal_value_old_array;
      }

      case Moose::Older:
      {
        _need_dof_values_older = true;
        return _nodal_value_older_array;
      }

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
const OutputType &
MooseVariableData<OutputType>::nodalValueDot() const
{
  if (isNodal())
  {
    if (_sys.solutionUDot())
    {
      _need_dof_values_dot = true;
      return _nodal_value_dot;
    }
    else
      mooseError(
          "MooseVariableData: Time derivative of solution (`u_dot`) is not stored. Please set "
          "uDotRequested() to true in FEProblemBase before requesting `u_dot`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableData<OutputType>::nodalValueDotDot() const
{
  if (isNodal())
  {
    if (_sys.solutionUDotDot())
    {
      _need_dof_values_dotdot = true;
      return _nodal_value_dotdot;
    }
    else
      mooseError(
          "MooseVariableData: Second time derivative of solution (`u_dotdot`) is not stored. "
          "Please set uDotDotRequested() to true in FEProblemBase before requesting "
          "`u_dotdot`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableData<OutputType>::nodalValueDotOld() const
{
  if (isNodal())
  {
    if (_sys.solutionUDotOld())
    {
      _need_dof_values_dot_old = true;
      return _nodal_value_dot_old;
    }
    else
      mooseError("MooseVariableData: Old time derivative of solution (`u_dot_old`) is not stored. "
                 "Please set uDotOldRequested() to true in FEProblemBase before requesting "
                 "`u_dot_old`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
const OutputType &
MooseVariableData<OutputType>::nodalValueDotDotOld() const
{
  if (isNodal())
  {
    if (_sys.solutionUDotDotOld())
    {
      _need_dof_values_dotdot_old = true;
      return _nodal_value_dotdot_old;
    }
    else
      mooseError(
          "MooseVariableData: Old second time derivative of solution (`u_dotdot_old`) is not "
          "stored. Please set uDotDotOldRequested() to true in FEProblemBase before "
          "requesting `u_dotdot_old`.");
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeNodalValues()
{
  if (_has_dof_indices)
  {
    fetchDoFValues();
    assignNodalValue();

    if (_need_ad && _subproblem.currentlyComputingJacobian())
      fetchADDoFValues();
  }
  else
    zeroSizeDofValues();
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::nodalVectorTagValue(TagID tag) const
{
  if (isNodal())
  {
    _need_vector_tag_dof_u[tag] = true;

    if (_sys.hasVector(tag) && tag < _vector_tags_dof_u.size())
      return _vector_tags_dof_u[tag];
    else
      mooseError("Tag is not associated with any vector or there is no any data for tag ",
                 tag,
                 " for nodal variable ",
                 _var.name());
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DoFValue &
MooseVariableData<OutputType>::nodalMatrixTagValue(TagID tag) const
{
  if (isNodal())
  {
    _need_matrix_tag_dof_u[tag] = true;

    if (_sys.hasMatrix(tag) && tag < _matrix_tags_dof_u.size())
      return _matrix_tags_dof_u[tag];
    else
      mooseError("Tag is not associated with any matrix or there is no any data for tag ",
                 tag,
                 " for nodal variable ",
                 _var.name());
  }
  else
    mooseError("Nodal values can be requested only on nodal variables, variable '",
               _var.name(),
               "' is not nodal.");
}

template <typename OutputType>
void
MooseVariableData<OutputType>::fetchDoFValues()
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

  if (_sys.subproblem().safeAccessTaggedVectors())
  {
    auto & active_coupleable_vector_tags =
        _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
    for (auto tag : active_coupleable_vector_tags)
      if (_need_vector_tag_u[tag] || _need_vector_tag_dof_u[tag])
        if (_sys.hasVector(tag) && _sys.getVector(tag).closed())
        {
          auto & vec = _sys.getVector(tag);
          _vector_tags_dof_u[tag].resize(n);
          vec.get(_dof_indices, &_vector_tags_dof_u[tag][0]);
        }
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

template <>
void
MooseVariableData<RealEigenVector>::fetchDoFValues()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  /*
   * A note here: we might use Eigen::map to remove fetching data, a future work
   * we can consider.
   */

  getArrayDoFValues(*_sys.currentSolution(), n, _dof_values);

  if (is_transient)
  {
    if (_need_u_old || _need_grad_old || _need_second_old || _need_curl_old || _need_dof_values_old)
      getArrayDoFValues(_sys.solutionOld(), n, _dof_values_old);
    if (_need_u_older || _need_grad_older || _need_second_older || _need_dof_values_older)
      getArrayDoFValues(_sys.solutionOlder(), n, _dof_values_older);
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

  if (_need_u_previous_nl || _need_grad_previous_nl || _need_second_previous_nl ||
      _need_dof_values_previous_nl)
    getArrayDoFValues(*_sys.solutionPreviousNewton(), n, _dof_values_previous_nl);

  if (_sys.subproblem().safeAccessTaggedVectors())
  {
    auto & active_coupleable_vector_tags =
        _sys.subproblem().getActiveFEVariableCoupleableVectorTags(_tid);
    for (auto tag : active_coupleable_vector_tags)
      if (_need_vector_tag_u[tag] || _need_vector_tag_dof_u[tag])
        if (_sys.hasVector(tag) && _sys.getVector(tag).closed())
          getArrayDoFValues(_sys.getVector(tag), n, _vector_tags_dof_u[tag]);
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
MooseVariableData<OutputType>::fetchADDoFValues()
{
  auto n = _dof_indices.size();
  libmesh_assert(n);
  _ad_dof_values.resize(n);
  auto ad_offset = _var_num * _sys.getMaxVarNDofsPerNode();

  for (decltype(n) i = 0; i < n; ++i)
  {
    _ad_dof_values[i] = _dof_values[i];
    if (_var.kind() == Moose::VAR_NONLINEAR)
      _ad_dof_values[i].derivatives().insert(ad_offset + i) = 1.;
    assignADNodalValue(_ad_dof_values[i], i);
  }
}

template <>
void
MooseVariableData<RealEigenVector>::fetchADDoFValues()
{
  mooseError("I do not know how to support AD with array variables");
}

template <typename OutputType>
void
MooseVariableData<OutputType>::zeroSizeDofValues()
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
MooseVariableData<OutputType>::getArrayDoFValues(const NumericVector<Number> & sol,
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

template <typename OutputType>
void
MooseVariableData<OutputType>::assignNodalValue()
{
  bool is_transient = _subproblem.isTransient();

  libmesh_assert(_dof_indices.size());

  _nodal_value = _dof_values[0];
  _nodal_value_array[0] = _nodal_value;

  if (is_transient)
  {
    if (_need_dof_values_old)
    {
      _nodal_value_old = _dof_values_old[0];
      _nodal_value_old_array[0] = _nodal_value_old;
    }
    if (_need_dof_values_older)
    {
      _nodal_value_older = _dof_values_older[0];
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
  if (_need_dof_values_previous_nl)
    _nodal_value_previous_nl = _dof_values_previous_nl[0];
}

template <>
void
MooseVariableData<RealVectorValue>::assignNodalValue()
{
  bool is_transient = _subproblem.isTransient();

  auto n = _dof_indices.size();
  libmesh_assert(n);

  for (decltype(n) i = 0; i < n; ++i)
    _nodal_value(i) = _dof_values[i];

  if (is_transient)
  {
    if (_need_dof_values_old)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_old(i) = _dof_values_old[i];
    if (_need_dof_values_older)
      for (decltype(n) i = 0; i < n; ++i)
        _nodal_value_older(i) = _dof_values_older[i];
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
  if (_need_dof_values_previous_nl)
    for (decltype(n) i = 0; i < n; ++i)
      _nodal_value_previous_nl(i) = _dof_values_previous_nl[i];
}

template <>
void
MooseVariableData<Real>::assignADNodalValue(const DualReal & value, const unsigned int &)
{
  _ad_nodal_value = value;
}

template <>
void
MooseVariableData<RealVectorValue>::assignADNodalValue(const DualReal & value,
                                                       const unsigned int & component)
{
  _ad_nodal_value(component) = value;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::prepareIC()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  _dof_values.resize(_dof_indices.size());

  unsigned int nqp = _qrule->n_points();
  _u.resize(nqp);
}

template <typename OutputType>
void
MooseVariableData<OutputType>::prepare()
{
  _dof_map.dof_indices(_elem, _dof_indices, _var_num);
  _has_dof_values = false;

  // FIXME: remove this when the Richard's module is migrated to use the new NodalCoupleable
  // interface.
  if (_dof_indices.size() > 0)
    _has_dof_indices = true;
  else
    _has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::prepareAux()
{
  _has_dof_values = false;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::reinitNode()
{
  if (size_t n_dofs = _node->n_dofs(_sys.number(), _var_num))
  {
    _dof_indices.resize(n_dofs);
    for (size_t i = 0; i < n_dofs; ++i)
      _dof_indices[i] = _node->dof_number(_sys.number(), _var_num, i);
    // For standard variables. _nodal_dof_index is retrieved by nodalDofIndex() which is used in
    // NodalBC for example
    _nodal_dof_index = _dof_indices[0];
    _has_dof_indices = true;
  }
  else
  {
    _dof_indices.clear(); // Clear these so Assembly doesn't think there's dofs here
    _has_dof_indices = false;
  }
}

template <typename OutputType>
void
MooseVariableData<OutputType>::reinitAux()
{
  /* FIXME: this method is only for elemental auxiliary variables, so
   * we may want to rename it */
  if (_elem)
  {
    _dof_map.dof_indices(_elem, _dof_indices, _var_num);
    if (_elem->n_dofs(_sys.number(), _var_num) > 0)
    {
      // FIXME: check if the following is equivalent with '_nodal_dof_index = _dof_indices[0];'?
      _nodal_dof_index = _elem->dof_number(_sys.number(), _var_num, 0);

      fetchDoFValues();

      for (auto & dof_u : _vector_tags_dof_u)
        dof_u.resize(_dof_indices.size());

      for (auto & dof_u : _matrix_tags_dof_u)
        dof_u.resize(_dof_indices.size());

      _has_dof_indices = true;
    }
    else
      _has_dof_indices = false;
  }
  else
    _has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::reinitNodes(const std::vector<dof_id_type> & nodes)
{
  _dof_indices.clear();
  for (const auto & node_id : nodes)
  {
    auto && nd = _subproblem.mesh().getMesh().query_node_ptr(node_id);
    if (nd && (_subproblem.mesh().isSemiLocal(const_cast<Node *>(nd))))
    {
      if (nd->n_dofs(_sys.number(), _var_num) > 0)
      {
        dof_id_type dof = nd->dof_number(_sys.number(), _var_num, 0);
        _dof_indices.push_back(dof);
      }
    }
  }

  if (_dof_indices.size() > 0)
    _has_dof_indices = true;
  else
    _has_dof_indices = false;
}

template <>
template <>
const VariableValue &
MooseVariableData<Real>::adSln<RESIDUAL>() const
{
  return _u;
}

template <>
template <>
const VectorVariableValue &
MooseVariableData<RealVectorValue>::adSln<RESIDUAL>() const
{
  return _u;
}

template <>
template <>
const VariableGradient &
MooseVariableData<Real>::adGradSln<RESIDUAL>() const
{
  return _grad_u;
}

template <>
template <>
const VectorVariableGradient &
MooseVariableData<RealVectorValue>::adGradSln<RESIDUAL>() const
{
  return _grad_u;
}

template <>
template <>
const VariableSecond &
MooseVariableData<Real>::adSecondSln<RESIDUAL>() const
{
  _need_second = true;
  secondPhi();
  secondPhiFace();
  return _second_u;
}

template <>
template <>
const VectorVariableSecond &
MooseVariableData<RealVectorValue>::adSecondSln<RESIDUAL>() const
{
  _need_second = true;
  secondPhi();
  secondPhiFace();
  return _second_u;
}

template <>
template <>
const VariableValue &
MooseVariableData<Real>::adUDot<RESIDUAL>() const
{

  return uDot();
}

template <>
template <>
const VectorVariableValue &
MooseVariableData<RealVectorValue>::adUDot<RESIDUAL>() const
{
  return uDot();
}

template <>
template <>
const MooseArray<Real> &
MooseVariableData<Real>::adDofValues<RESIDUAL>() const
{
  return _dof_values;
}

template <>
template <>
const MooseArray<Real> &
MooseVariableData<RealVectorValue>::adDofValues<RESIDUAL>() const
{
  return _dof_values;
}

template <>
template <>
const Real &
MooseVariableData<Real>::adNodalValue<RESIDUAL>() const
{
  return _nodal_value;
}

template <>
template <>
const RealVectorValue &
MooseVariableData<RealVectorValue>::adNodalValue<RESIDUAL>() const
{
  return _nodal_value;
}

template class MooseVariableData<Real>;
template class MooseVariableData<RealVectorValue>;
template class MooseVariableData<RealEigenVector>;
