//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableData.h"
#include "MooseVariableField.h"
#include "Assembly.h"
#include "MooseError.h"
#include "DisplacedSystem.h"
#include "TimeIntegrator.h"
#include "MooseVariableFE.h"
#include "MooseTypes.h"
#include "DualRealOps.h"

#include "libmesh/quadrature.h"
#include "libmesh/fe_base.h"
#include "libmesh/system.h"
#include "libmesh/type_n_tensor.h"

template <typename OutputType>
MooseVariableData<OutputType>::MooseVariableData(const MooseVariableField<OutputType> & var,
                                                 SystemBase & sys,
                                                 THREAD_ID tid,
                                                 Moose::ElementType element_type,
                                                 const QBase * const & qrule_in,
                                                 const QBase * const & qrule_face_in,
                                                 const Node * const & node,
                                                 const Elem * const & elem)

  : MooseVariableDataBase<OutputType>(var, sys, tid),
    _fe_type(_var.feType()),
    _var_num(_var.number()),
    _assembly(_subproblem.assembly(_tid, var.kind() == Moose::VAR_NONLINEAR ? sys.number() : 0)),
    _element_type(element_type),
    _ad_zero(0),
    _need_ad_u_dot(false),
    _need_ad_u_dotdot(false),
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
    _need_ad_grad_u_dot(false),
    _need_ad_second_u(false),
    _has_dof_indices(false),
    _qrule(qrule_in),
    _qrule_face(qrule_face_in),
    _use_dual(var.useDual()),
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
  if (_fe_type.family == NEDELEC_ONE || _fe_type.family == LAGRANGE_VEC ||
      _fe_type.family == MONOMIAL_VEC)
    _continuity = _assembly.getVectorFE(_fe_type, _sys.mesh().dimension())->get_continuity();
  else
    _continuity = _assembly.getFE(_fe_type, _sys.mesh().dimension())->get_continuity();

  _is_nodal = (_continuity == C_ZERO || _continuity == C_ONE);

  _time_integrator = _sys.getTimeIntegrator();

  switch (_element_type)
  {
    case Moose::ElementType::Element:
    {
      _phi_assembly_method = &Assembly::fePhi<OutputShape>;
      _phi_face_assembly_method = &Assembly::fePhiFace<OutputShape>;
      _grad_phi_assembly_method = &Assembly::feGradPhi<OutputShape>;
      _grad_phi_face_assembly_method = &Assembly::feGradPhiFace<OutputShape>;
      _second_phi_assembly_method = &Assembly::feSecondPhi<OutputShape>;
      _second_phi_face_assembly_method = &Assembly::feSecondPhiFace<OutputShape>;
      _curl_phi_assembly_method = &Assembly::feCurlPhi<OutputShape>;
      _curl_phi_face_assembly_method = &Assembly::feCurlPhiFace<OutputShape>;
      _ad_grad_phi_assembly_method = &Assembly::feADGradPhi<OutputShape>;
      _ad_grad_phi_face_assembly_method = &Assembly::feADGradPhiFace<OutputShape>;

      _ad_grad_phi = &_ad_grad_phi_assembly_method(_assembly, _fe_type);
      _ad_grad_phi_face = &_ad_grad_phi_face_assembly_method(_assembly, _fe_type);
      break;
    }
    case Moose::ElementType::Neighbor:
    {
      _phi_assembly_method = &Assembly::fePhiNeighbor<OutputShape>;
      _phi_face_assembly_method = &Assembly::fePhiFaceNeighbor<OutputShape>;
      _grad_phi_assembly_method = &Assembly::feGradPhiNeighbor<OutputShape>;
      _grad_phi_face_assembly_method = &Assembly::feGradPhiFaceNeighbor<OutputShape>;
      _second_phi_assembly_method = &Assembly::feSecondPhiNeighbor<OutputShape>;
      _second_phi_face_assembly_method = &Assembly::feSecondPhiFaceNeighbor<OutputShape>;
      _curl_phi_assembly_method = &Assembly::feCurlPhiNeighbor<OutputShape>;
      _curl_phi_face_assembly_method = &Assembly::feCurlPhiFaceNeighbor<OutputShape>;

      _ad_grad_phi = nullptr;
      _ad_grad_phi_face = nullptr;
      break;
    }
    case Moose::ElementType::Lower:
    {
      if (_use_dual)
      {
        _phi_assembly_method = &Assembly::feDualPhiLower<OutputType>;
        _phi_face_assembly_method = &Assembly::feDualPhiLower<OutputType>; // Place holder
        _grad_phi_assembly_method = &Assembly::feGradDualPhiLower<OutputType>;
        _grad_phi_face_assembly_method = &Assembly::feGradDualPhiLower<OutputType>; // Place holder
      }
      else
      {
        _phi_assembly_method = &Assembly::fePhiLower<OutputType>;
        _phi_face_assembly_method = &Assembly::fePhiLower<OutputType>; // Place holder
        _grad_phi_assembly_method = &Assembly::feGradPhiLower<OutputType>;
        _grad_phi_face_assembly_method = &Assembly::feGradPhiLower<OutputType>; // Place holder
      }

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
  auto && active_coupleable_matrix_tags = _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);

  for (auto tag : _required_vector_tags)
  {
    if (_need_vector_tag_u[tag])
      _vector_tag_u[tag].resize(nqp);
    if (_need_vector_tag_grad[tag])
      _vector_tag_grad[tag].resize(nqp);
  }

  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
      _matrix_tag_u[tag].resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_curl)
    _curl_u.resize(nqp);

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

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_curl_old)
      _curl_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    for (auto tag : _required_vector_tags)
    {
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][i] = 0;
      if (_need_vector_tag_grad[tag])
        _vector_tag_grad[tag][i] = 0;
    }

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][i] = 0;

    if (_need_second)
      _second_u[i] = 0;

    if (_need_curl)
      _curl_u[i] = 0;

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

      if (is_transient)
      {
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
          _second_u[qp].add_scaled(d2phi_local, _vector_tags_dof_u[_solution_tag][i]);

        if (_need_second_previous_nl)
          _second_u_previous_nl[qp].add_scaled(d2phi_local,
                                               _vector_tags_dof_u[_previous_nl_solution_tag][i]);

        if (is_transient)
        {
          if (_need_second_old)
            _second_u_old[qp].add_scaled(d2phi_local, _vector_tags_dof_u[_old_solution_tag][i]);

          if (_need_second_older)
            _second_u_older[qp].add_scaled(d2phi_local, _vector_tags_dof_u[_older_solution_tag][i]);
        }
      }

      if (curl_required)
      {
        mooseAssert(_current_curl_phi,
                    "We're requiring a curl calculation but have not set a curl shape function!");
        const OutputType curl_phi_local = (*_current_curl_phi)[i][qp];

        if (_need_curl)
          _curl_u[qp] += curl_phi_local * _vector_tags_dof_u[_solution_tag][i];

        if (is_transient && _need_curl_old)
          _curl_u_old[qp] += curl_phi_local * _vector_tags_dof_u[_old_solution_tag][i];
      }

      for (auto tag : _required_vector_tags)
      {
        if (_sys.hasVector(tag) && _sys.getVector(tag).closed())
        {
          if (_need_vector_tag_u[tag])
            _vector_tag_u[tag][qp] += phi_local * _vector_tags_dof_u[tag][i];
          if (_need_vector_tag_grad[tag])
            _vector_tag_grad[tag][qp].add_scaled(dphi_qp, _vector_tags_dof_u[tag][i]);
        }
      }

      for (auto tag : active_coupleable_matrix_tags)
        if (_need_matrix_tag_u[tag])
          _matrix_tag_u[tag][qp] += phi_local * _matrix_tags_dof_u[tag][i];
    }
  }

  // Automatic differentiation
  if (_need_ad)
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
  auto && active_coupleable_matrix_tags = _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);

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

  for (auto tag : _required_vector_tags)
  {
    if (_need_vector_tag_u[tag])
      _vector_tag_u[tag].resize(nqp);
    if (_need_vector_tag_grad[tag])
      _vector_tag_grad[tag].resize(nqp);
  }

  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
      _matrix_tag_u[tag].resize(nqp);

  if (_need_second)
    _second_u.resize(nqp);

  if (_need_curl)
    _curl_u.resize(nqp);

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

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_curl_old)
      _curl_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  for (unsigned int i = 0; i < nqp; ++i)
  {
    for (auto tag : _required_vector_tags)
    {
      if (_need_vector_tag_u[tag])
        _vector_tag_u[tag][i].setZero(_count);
      if (_need_vector_tag_grad[tag])
        _vector_tag_grad[tag][i].setZero(_count, LIBMESH_DIM);
    }

    for (auto tag : active_coupleable_matrix_tags)
      if (_need_matrix_tag_u[tag])
        _matrix_tag_u[tag][i].setZero(_count);

    if (_need_second)
      _second_u[i].setZero(_count, LIBMESH_DIM * LIBMESH_DIM);

    if (_need_curl)
      _curl_u[i].setZero(_count);

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

      if (is_transient)
      {
        if (_need_u_dot)
          _u_dot[qp] += phi_local * _dof_values_dot[i];

        if (_need_u_dotdot)
          _u_dotdot[qp] += phi_local * _dof_values_dotdot[i];

        if (_need_u_dot_old)
          _u_dot_old[qp] += phi_local * _dof_values_dot_old[i];

        if (_need_u_dotdot_old)
          _u_dotdot_old[qp] += phi_local * _dof_values_dotdot_old[i];

        if (_need_grad_dot)
          for (const auto d : make_range(Moose::dim))
            _grad_u_dot[qp].col(d) += dphi_qp(d) * _dof_values_dot[i];

        if (_need_grad_dotdot)
          for (const auto d : make_range(Moose::dim))
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
              _second_u[qp].col(d++) += d2phi_local(d1, d2) * _vector_tags_dof_u[_solution_tag][i];

        if (_need_second_previous_nl)
          for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
            for (unsigned int d2 = 0; d2 < LIBMESH_DIM; ++d2)
              _second_u_previous_nl[qp].col(d++) +=
                  d2phi_local(d1, d2) * _vector_tags_dof_u[_previous_nl_solution_tag][i];

        if (is_transient)
        {
          if (_need_second_old)
            for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
              for (unsigned int d2 = 0; d2 < LIBMESH_DIM; ++d2)
                _second_u_old[qp].col(d++) +=
                    d2phi_local(d1, d2) * _vector_tags_dof_u[_old_solution_tag][i];

          if (_need_second_older)
            for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
              for (unsigned int d2 = 0; d2 < LIBMESH_DIM; ++d2)
                _second_u_older[qp].col(d++) +=
                    d2phi_local(d1, d2) * _vector_tags_dof_u[_older_solution_tag][i];
        }
      }

      if (curl_required)
      {
        mooseAssert(_current_curl_phi,
                    "We're requiring a curl calculation but have not set a curl shape function!");
        const auto curl_phi_local = (*_current_curl_phi)[i][qp];

        if (_need_curl)
          _curl_u[qp] += curl_phi_local * _vector_tags_dof_u[_solution_tag][i];

        if (is_transient && _need_curl_old)
          _curl_u_old[qp] += curl_phi_local * _vector_tags_dof_u[_old_solution_tag][i];
      }

      for (auto tag : _required_vector_tags)
      {
        if (_need_vector_tag_u[tag])
          _vector_tag_u[tag][qp] += phi_local * _vector_tags_dof_u[tag][i];
        if (_need_vector_tag_grad[tag])
          for (const auto d : make_range(Moose::dim))
            _vector_tag_grad[tag][qp].col(d) += dphi_qp(d) * _vector_tags_dof_u[tag][i];
      }

      for (auto tag : active_coupleable_matrix_tags)
        if (_need_matrix_tag_u[tag])
          _matrix_tag_u[tag][qp] += phi_local * _matrix_tags_dof_u[tag][i];
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

  if (_need_second)
    _second_u.resize(nqp);

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

    if (_need_second_old)
      _second_u_old.resize(nqp);

    if (_need_second_older)
      _second_u_older.resize(nqp);
  }

  if (is_transient)
  {
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
  Real u_dot = 0;
  Real u_dotdot = 0;
  Real u_dot_old = 0;
  Real u_dotdot_old = 0;
  const Real & du_dot_du = _sys.duDotDu();
  const Real & du_dotdot_du = _sys.duDotDotDu();

  if (is_transient)
  {
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
  }

  for (unsigned qp = 1; qp < nqp; ++qp)
  {
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
    }
  }

  auto && active_coupleable_matrix_tags = _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);

  for (auto tag : _required_vector_tags)
  {
    if (_need_vector_tag_u[tag] || _need_vector_tag_grad[tag] || _need_vector_tag_dof_u[tag])
      if ((_subproblem.vectorTagType(tag) == Moose::VECTOR_TAG_RESIDUAL &&
           _subproblem.safeAccessTaggedVectors()) ||
          _subproblem.vectorTagType(tag) == Moose::VECTOR_TAG_SOLUTION)
        // tag is defined on problem but may not be used by a system
        if (_sys.hasVector(tag) && _sys.getVector(tag).closed())
        {
          auto & vec = _sys.getVector(tag);
          _vector_tags_dof_u[tag].resize(1);
          _vector_tags_dof_u[tag][0] = vec(_dof_indices[0]);
        }

    if (_need_vector_tag_u[tag])
    {
      _vector_tag_u[tag].resize(nqp);
      auto v = phi * _vector_tags_dof_u[tag][0];
      for (unsigned int qp = 0; qp < nqp; ++qp)
        _vector_tag_u[tag][qp] = v;
    }
    if (_need_vector_tag_grad[tag])
      _vector_tag_grad[tag].resize(nqp);
  }

  if (_subproblem.safeAccessTaggedMatrices())
  {
    auto & active_coupleable_matrix_tags =
        _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);
    for (auto tag : active_coupleable_matrix_tags)
    {
      _matrix_tags_dof_u[tag].resize(1);
      if (_need_matrix_tag_dof_u[tag] || _need_matrix_tag_u[tag])
        if (_sys.hasMatrix(tag) && _sys.matrixTagActive(tag) && _sys.getMatrix(tag).closed())
        {
          auto & mat = _sys.getMatrix(tag);
          {
            Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
            _matrix_tags_dof_u[tag][0] = mat(_dof_indices[0], _dof_indices[0]);
          }
        }
    }
  }
  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
    {
      _matrix_tag_u[tag].resize(nqp);
      auto v = phi * _matrix_tags_dof_u[tag][0];
      for (unsigned int qp = 0; qp < nqp; ++qp)
        _matrix_tag_u[tag][qp] = v;
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
  // Have to do this because upon construction this won't initialize any of the derivatives
  // (because DualNumber::do_derivatives is false at that time).
  _ad_zero = 0;

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
  if (_need_ad_grad_u_dot)
    _ad_grad_u_dot.resize(nqp);

  if (_need_ad_u_dotdot)
  {
    _ad_dofs_dotdot.resize(num_dofs);
    _ad_u_dotdot.resize(nqp);
  }

  const bool do_derivatives =
      ADReal::do_derivatives && _sys.number() == _subproblem.currentNlSysNum();

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

    if (_need_ad_u_dotdot)
      _ad_u_dotdot[qp] = _ad_zero;

    if (_need_ad_grad_u_dot)
      _ad_grad_u_dot[qp] = _ad_zero;
  }

  for (unsigned int i = 0; i < num_dofs; i++)
  {
    _ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices[i]);

    // NOTE!  You have to do this AFTER setting the value!
    if (do_derivatives)
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);

    if (_need_ad_u_dot && _time_integrator && _time_integrator->dt())
    {
      _ad_dofs_dot[i] = _ad_dof_values[i];
      _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i],
                                                 _dof_indices[i],
                                                 _need_ad_u_dotdot ? _ad_dofs_dotdot[i]
                                                                   : _ad_real_dummy);
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
        // calculation of ad_grad_phi for neighbor and neighbor-face, so if we are in that
        // situation we need to default to using the non-ad grad_phi
        if (_displaced && _current_ad_grad_phi)
          _ad_grad_u[qp] += _ad_dof_values[i] * (*_current_ad_grad_phi)[i][qp];
        else
          _ad_grad_u[qp] += _ad_dof_values[i] * (*_current_grad_phi)[i][qp];
      }

      if (_need_ad_second_u)
        // Note that this will not carry any derivatives with respect to displacements because
        // those calculations have not yet been implemented in Assembly
        _ad_second_u[qp] += _ad_dof_values[i] * (*_current_second_phi)[i][qp];

      if (_need_ad_u_dot && _time_integrator && _time_integrator->dt())
      {
        _ad_u_dot[qp] += (*_current_phi)[i][qp] * _ad_dofs_dot[i];
        if (_need_ad_u_dotdot)
          _ad_u_dotdot[qp] += (*_current_phi)[i][qp] * _ad_dofs_dotdot[i];
      }

      if (_need_ad_grad_u_dot && _time_integrator && _time_integrator->dt())
      {
        // The latter check here is for handling the fact that we have not yet implemented
        // calculation of ad_grad_phi for neighbor and neighbor-face, so if we are in that
        // situation we need to default to using the non-ad grad_phi
        if (_displaced && _current_ad_grad_phi)
          _ad_grad_u_dot[qp] += _ad_dofs_dot[i] * (*_current_ad_grad_phi)[i][qp];
        else
          _ad_grad_u_dot[qp] += _ad_dofs_dot[i] * (*_current_grad_phi)[i][qp];
      }
    }
  }

  if (_need_ad_u_dot && !_time_integrator)
    for (MooseIndex(nqp) qp = 0; qp < nqp; ++qp)
    {
      _ad_u_dot[qp] = _u_dot[qp];
      if (_need_ad_u_dotdot)
        _ad_u_dotdot[qp] = _u_dotdot[qp];
    }

  if (_need_ad_grad_u_dot && !_time_integrator)
    for (MooseIndex(nqp) qp = 0; qp < nqp; ++qp)
      _ad_grad_u_dot[qp] = _grad_u_dot[qp];
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
MooseVariableData<OutputType>::setDofValue(const OutputData & value, unsigned int index)
{
  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  dof_values[index] = value;
  _has_dof_values = true;

  auto & u = _vector_tag_u[_solution_tag];
  for (unsigned int qp = 0; qp < u.size(); qp++)
  {
    u[qp] = (*_phi)[0][qp] * dof_values[0];
    for (unsigned int i = 1; i < dof_values.size(); i++)
      u[qp] += (*_phi)[i][qp] * dof_values[i];
  }
}

template <typename OutputType>
void
MooseVariableData<OutputType>::setDofValues(const DenseVector<OutputData> & values)
{
  auto & dof_values = _vector_tags_dof_u[_solution_tag];
  for (unsigned int i = 0; i < values.size(); i++)
    dof_values[i] = values(i);

  _has_dof_values = true;

  auto & u = _vector_tag_u[_solution_tag];
  for (unsigned int qp = 0; qp < u.size(); qp++)
  {
    u[qp] = (*_phi)[0][qp] * dof_values[0];
    for (unsigned int i = 1; i < dof_values.size(); i++)
      u[qp] += (*_phi)[i][qp] * dof_values[i];
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

    if (_need_ad)
      fetchADDoFValues();
  }
  else
    zeroSizeDofValues();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::fetchADDoFValues()
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
    assignADNodalValue(_ad_dof_values[i], i);
  }
}

template <>
void
MooseVariableData<RealEigenVector>::fetchADDoFValues()
{
  mooseError("I do not know how to support AD with array variables");
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
  _vector_tags_dof_u[_solution_tag].resize(_dof_indices.size());

  unsigned int nqp = _qrule->n_points();
  _vector_tag_u[_solution_tag].resize(nqp);
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
MooseVariableData<OutputType>::reinitNode()
{
  if (std::size_t n_dofs = _node->n_dofs(_sys.number(), _var_num))
  {
    _dof_indices.resize(n_dofs);
    for (std::size_t i = 0; i < n_dofs; ++i)
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

template class MooseVariableData<Real>;
template class MooseVariableData<RealVectorValue>;
template class MooseVariableData<RealEigenVector>;
