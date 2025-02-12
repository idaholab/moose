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

#include "libmesh/quadrature.h"
#include "libmesh/fe_base.h"
#include "libmesh/system.h"
#include "libmesh/type_n_tensor.h"
#include "libmesh/fe_interface.h"

using namespace libMesh;

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
    _fe_type(var.feType()),
    _var_num(var.number()),
    _assembly(_subproblem.assembly(_tid, var.kind() == Moose::VAR_SOLVER ? sys.number() : 0)),
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
    _need_div(false),
    _need_div_old(false),
    _need_div_older(false),
    _need_ad(false),
    _need_ad_u(false),
    _need_ad_grad_u(false),
    _need_ad_grad_u_dot(false),
    _need_ad_second_u(false),
    _need_ad_curl_u(false),
    _has_dof_indices(false),
    _qrule(qrule_in),
    _qrule_face(qrule_face_in),
    _use_dual(var.useDual()),
    _second_phi_assembly_method(nullptr),
    _second_phi_face_assembly_method(nullptr),
    _curl_phi_assembly_method(nullptr),
    _curl_phi_face_assembly_method(nullptr),
    _div_phi_assembly_method(nullptr),
    _div_phi_face_assembly_method(nullptr),
    _ad_grad_phi_assembly_method(nullptr),
    _ad_grad_phi_face_assembly_method(nullptr),
    _time_integrator(nullptr),
    _node(node),
    _elem(elem),
    _displaced(dynamic_cast<const DisplacedSystem *>(&_sys) ? true : false),
    _current_side(_assembly.side())
{
  _continuity = FEInterface::get_continuity(_fe_type);

  _is_nodal = (_continuity == C_ZERO || _continuity == C_ONE);

  _time_integrator = _sys.queryTimeIntegrator(_var_num);

  // Initialize AD zero with zero derivatives
  const auto old_do = ADReal::do_derivatives;
  ADReal::do_derivatives = true;
  _ad_zero = 0.;
  ADReal::do_derivatives = old_do;

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
      _div_phi_assembly_method = &Assembly::feDivPhi<OutputShape>;
      _div_phi_face_assembly_method = &Assembly::feDivPhiFace<OutputShape>;
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
      _div_phi_assembly_method = &Assembly::feDivPhiNeighbor<OutputShape>;
      _div_phi_face_assembly_method = &Assembly::feDivPhiFaceNeighbor<OutputShape>;

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
      _current_div_phi = _div_phi;
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
      _current_div_phi = _div_phi_face;
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
const typename MooseVariableData<OutputType>::FieldVariableDivergence &
MooseVariableData<OutputType>::divSln(Moose::SolutionState state) const
{
  divPhi();
  divPhiFace();
  switch (state)
  {
    case Moose::Current:
    {
      _need_div = true;
      return _div_u;
    }

    case Moose::Old:
    {
      _need_div_old = true;
      return _div_u_old;
    }

    case Moose::Older:
    {
      _need_div_older = true;
      return _div_u_older;
    }

    default:
      mooseError("We don't currently support divergence from the previous non-linear iteration");
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
const typename MooseVariableData<OutputType>::FieldVariablePhiDivergence &
MooseVariableData<OutputType>::divPhi() const
{
  _div_phi = &_div_phi_assembly_method(_assembly, _fe_type);
  return *_div_phi;
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::FieldVariablePhiDivergence &
MooseVariableData<OutputType>::divPhiFace() const
{
  _div_phi_face = &_div_phi_face_assembly_method(_assembly, _fe_type);
  return *_div_phi_face;
}

template <typename OutputType>
template <bool monomial>
void
MooseVariableData<OutputType>::computeValuesInternal()
{
  const auto num_dofs = _dof_indices.size();
  if (num_dofs > 0)
    fetchDoFValues();

  const bool is_transient = _subproblem.isTransient();
  const auto nqp = _current_qrule->n_points();
  auto && active_coupleable_matrix_tags = _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);

  // Map grad_phi using Eigen so that we can perform array operations easier
  if constexpr (std::is_same_v<OutputType, RealEigenVector>)
  {
    if (_qrule == _current_qrule)
    {
      _mapped_grad_phi.resize(num_dofs);
      for (const auto i : make_range(num_dofs))
      {
        _mapped_grad_phi[i].resize(nqp, Eigen::Map<RealDIMValue>(nullptr));
        for (const auto qp : make_range(nqp))
          // Note: this does NOT do any allocation.  It is "reconstructing" the object in place
          new (&_mapped_grad_phi[i][qp])
              Eigen::Map<RealDIMValue>(const_cast<Real *>(&(*_current_grad_phi)[i][qp](0)));
      }
    }
    else
    {
      _mapped_grad_phi_face.resize(num_dofs);
      for (const auto i : make_range(num_dofs))
      {
        _mapped_grad_phi_face[i].resize(nqp, Eigen::Map<RealDIMValue>(nullptr));
        for (const auto qp : make_range(nqp))
          // Note: this does NOT do any allocation.  It is "reconstructing" the object in place
          new (&_mapped_grad_phi_face[i][qp])
              Eigen::Map<RealDIMValue>(const_cast<Real *>(&(*_current_grad_phi)[i][qp](0)));
      }
    }
  }

  mooseAssert(
      !(_need_second || _need_second_old || _need_second_older || _need_second_previous_nl) ||
          _current_second_phi,
      "We're requiring a second calculation but have not set a second shape function!");
  mooseAssert(!(_need_curl || _need_curl_old) || _current_curl_phi,
              "We're requiring a curl calculation but have not set a curl shape function!");
  mooseAssert(!(_need_div || _need_div_old) || _current_div_phi,
              "We're requiring a divergence calculation but have not set a div shape function!");

  // Helper for filling values
  const auto fill = [this, nqp, num_dofs](auto & dest, const auto & phi, const auto & dof_values)
  {
    if constexpr (monomial)
      libmesh_ignore(num_dofs);

    // Deduce OutputType
    constexpr bool is_real = std::is_same_v<OutputType, Real>;
    constexpr bool is_real_vector = std::is_same_v<OutputType, RealVectorValue>;
    constexpr bool is_eigen = std::is_same_v<OutputType, RealEigenVector>;
    static_assert(is_real || is_real_vector || is_eigen, "Unsupported type");

    // this is only used in the RealEigenVector case to get this->_count
    if constexpr (!is_eigen)
      libmesh_ignore(this);

    // Deduce type of value within dest MooseArray
    using dest_array_type = typename std::remove_reference_t<decltype(dest)>::value_type;
    constexpr bool is_value = std::is_same_v<dest_array_type, OutputType>;
    constexpr bool is_gradient = std::is_same_v<dest_array_type, OutputGradient>;
    constexpr bool is_second = std::is_same_v<dest_array_type, OutputSecond>;
    constexpr bool is_divergence = std::is_same_v<dest_array_type, OutputDivergence>;
    static_assert(is_value || is_gradient || is_second || is_divergence,
                  "Unsupported destination array type");

    // Sets a value to zero at a quadrature point
    const auto set_zero = [this, &dest](const auto qp)
    {
      if constexpr (!is_eigen)
        libmesh_ignore(this);

      if constexpr (is_real || is_real_vector)
        dest[qp] = 0;
      else if constexpr (is_eigen)
      {
        if constexpr (is_value)
          dest[qp].setZero(this->_count);
        else if constexpr (is_gradient)
          dest[qp].setZero(this->_count, LIBMESH_DIM);
        else if constexpr (is_second)
          dest[qp].setZero(this->_count, LIBMESH_DIM * LIBMESH_DIM);
        else
          static_assert(Moose::always_false<OutputType, dest_array_type>, "Unsupported type");
      }
      else
        static_assert(Moose::always_false<OutputType, dest_array_type>, "Unsupported type");
    };

    // Accumulates a value
    const auto accumulate = [&dest, &phi, &dof_values](const auto i, const auto qp)
    {
      if constexpr (is_real || is_real_vector || (is_eigen && is_value))
      {
        if constexpr (is_value || is_divergence)
          dest[qp] += phi[i][qp] * dof_values[i];
        else if constexpr (is_gradient || is_second)
          dest[qp].add_scaled(phi[i][qp], dof_values[i]);
        else
          static_assert(Moose::always_false<OutputType, dest_array_type>, "Unsupported type");
      }
      else if constexpr (is_eigen)
      {
        if constexpr (is_gradient)
        {
          for (const auto d : make_range(Moose::dim))
            dest[qp].col(d) += phi[i][qp](d) * dof_values[i];
        }
        else if constexpr (is_second)
        {
          for (unsigned int d = 0, d1 = 0; d1 < LIBMESH_DIM; ++d1)
            for (const auto d2 : make_range(Moose::dim))
              dest[qp].col(d++) += phi[i][qp](d1, d2) * dof_values[i];
        }
        else
          static_assert(Moose::always_false<OutputType, dest_array_type>, "Unsupported type");
      }
      else
        static_assert(Moose::always_false<OutputType, dest_array_type>, "Unsupported type");
    };

    dest.resize(nqp);

    // Monomial case, accumulate dest[0] and set dest[>0] to dest[0]
    if constexpr (monomial)
    {
      mooseAssert(num_dofs == 1, "Should have only one dof");
      set_zero(0);
      accumulate(0, 0);
      for (unsigned int qp = 1; qp < nqp; ++qp)
        dest[qp] = dest[0];
    }
    // Non monomial case
    else
    {
      for (const auto qp : make_range(nqp))
        set_zero(qp);
      for (const auto i : make_range(num_dofs))
        for (const auto qp : make_range(nqp))
          accumulate(i, qp);
    }
  };

  // Curl
  if (_need_curl)
    fill(_curl_u, *_current_curl_phi, _vector_tags_dof_u[_solution_tag]);
  if (_need_curl_old)
    fill(_curl_u_old, *_current_curl_phi, _vector_tags_dof_u[_old_solution_tag]);

  // Div
  if (_need_div)
    fill(_div_u, *_current_div_phi, _vector_tags_dof_u[_solution_tag]);
  if (is_transient && _need_div_old)
    fill(_div_u_old, *_current_div_phi, _vector_tags_dof_u[_old_solution_tag]);

  // Second
  if (_need_second)
    fill(_second_u, *_current_second_phi, _vector_tags_dof_u[_solution_tag]);
  if (_need_second_previous_nl)
    fill(
        _second_u_previous_nl, *_current_second_phi, _vector_tags_dof_u[_previous_nl_solution_tag]);

  // Vector tags
  for (auto tag : _required_vector_tags)
  {
    if (_need_vector_tag_u[tag] && _sys.hasVector(tag) && _sys.getVector(tag).closed())
      fill(_vector_tag_u[tag], *_current_phi, _vector_tags_dof_u[tag]);
    if (_need_vector_tag_grad[tag] && _sys.hasVector(tag) && _sys.getVector(tag).closed())
      fill(_vector_tag_grad[tag], *_current_grad_phi, _vector_tags_dof_u[tag]);
  }

  // Matrix tags
  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
      fill(_matrix_tag_u[tag], *_current_phi, _matrix_tags_dof_u[tag]);

  // Derivatives and old values
  if (is_transient)
  {
    if (_need_second_old)
      fill(_second_u_old, *_current_second_phi, _vector_tags_dof_u[_old_solution_tag]);
    if (_need_second_older)
      fill(_second_u_older, *_current_second_phi, _vector_tags_dof_u[_older_solution_tag]);
    if (_need_u_dot)
      fill(_u_dot, *_current_phi, _dof_values_dot);
    if (_need_u_dotdot)
      fill(_u_dotdot, *_current_phi, _dof_values_dotdot);
    if (_need_u_dot_old)
      fill(_u_dot_old, *_current_phi, _dof_values_dot_old);
    if (_need_u_dotdot_old)
      fill(_u_dotdot_old, *_current_phi, _dof_values_dotdot_old);

    if (_need_du_dot_du)
    {
      _du_dot_du.resize(nqp);
      for (const auto qp : make_range(nqp))
        _du_dot_du[qp] = 0.;
      for (const auto i : make_range(num_dofs))
        for (const auto qp : make_range(nqp))
          _du_dot_du[qp] = _dof_du_dot_du[i];
    }
    if (_need_du_dotdot_du)
    {
      _du_dotdot_du.resize(nqp);
      for (const auto qp : make_range(nqp))
        _du_dotdot_du[qp] = 0.;
      for (const auto i : make_range(num_dofs))
        for (const auto qp : make_range(nqp))
          _du_dotdot_du[qp] = _dof_du_dotdot_du[i];
    }

    if (_need_grad_dot)
      fill(_grad_u_dot, *_current_grad_phi, _dof_values_dot);
    if (_need_grad_dotdot)
      fill(_grad_u_dotdot, *_current_grad_phi, _dof_values_dotdot);
  }

  // Automatic differentiation, not for eigen
  if constexpr (!std::is_same_v<OutputType, RealEigenVector>)
    if (_need_ad)
      computeAD(num_dofs, nqp);
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeValues()
{
  computeValuesInternal</* monomial = */ false>();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeMonomialValues()
{
  if (_dof_indices.size() == 0)
    return;

  // Monomial optimizations are not appropriate after p-refinement
  if (_elem->p_level())
    computeValues();
  else
    computeValuesInternal</* monomial = */ true>();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeAD(const unsigned int num_dofs, const unsigned int nqp)
{
  const bool do_derivatives = Moose::doDerivatives(_subproblem, _sys);

  _ad_dof_values.resize(num_dofs);
  for (const auto i : make_range(num_dofs))
    _ad_dof_values[i] = (*_sys.currentSolution())(_dof_indices[i]);
  // NOTE!  You have to do this AFTER setting the value!
  if (do_derivatives)
    for (const auto i : make_range(num_dofs))
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);

  if (_need_ad_u)
  {
    _ad_u.resize(nqp);
    for (const auto qp : make_range(nqp))
      _ad_u[qp] = _ad_zero;

    for (const auto i : make_range(num_dofs))
      for (const auto qp : make_range(nqp))
        _ad_u[qp] += _ad_dof_values[i] * (*_current_phi)[i][qp];
  }

  if (_need_ad_grad_u)
  {
    _ad_grad_u.resize(nqp);
    for (const auto qp : make_range(nqp))
      _ad_grad_u[qp] = _ad_zero;

    // The latter check here is for handling the fact that we have not yet implemented
    // calculation of ad_grad_phi for neighbor and neighbor-face, so if we are in that
    // situation we need to default to using the non-ad grad_phi
    if (_displaced && _current_ad_grad_phi)
      for (const auto i : make_range(num_dofs))
        for (const auto qp : make_range(nqp))
          _ad_grad_u[qp] += _ad_dof_values[i] * (*_current_ad_grad_phi)[i][qp];
    else
      for (const auto i : make_range(num_dofs))
        for (const auto qp : make_range(nqp))
          _ad_grad_u[qp] += _ad_dof_values[i] * (*_current_grad_phi)[i][qp];
  }

  if (_need_ad_second_u)
  {
    _ad_second_u.resize(nqp);
    for (const auto qp : make_range(nqp))
      _ad_second_u[qp] = _ad_zero;

    for (const auto i : make_range(num_dofs))
      for (const auto qp : make_range(nqp))
        // Note that this will not carry any derivatives with respect to displacements because
        // those calculations have not yet been implemented in Assembly
        _ad_second_u[qp] += _ad_dof_values[i] * (*_current_second_phi)[i][qp];
  }

  if (_need_ad_curl_u)
  {
    _ad_curl_u.resize(nqp);
    for (const auto qp : make_range(nqp))
      _ad_curl_u[qp] = _ad_zero;

    for (const auto i : make_range(num_dofs))
      for (const auto qp : make_range(nqp))
      {
        mooseAssert(_current_curl_phi,
          "We're requiring a curl calculation but have not set a curl shape function!");

        // Note that the current version of _ad_curl_u is not yet implemented for mesh displacement
        _ad_curl_u[qp] += _ad_dof_values[i] * (*_current_curl_phi)[i][qp];
      }
  }

  bool is_transient = _subproblem.isTransient();
  if (is_transient)
  {
    if (_need_ad_u_dot)
    {
      _ad_dofs_dot.resize(num_dofs);
      if (_need_ad_u_dotdot)
        _ad_dofs_dotdot.resize(num_dofs);
      _ad_u_dot.resize(nqp);
      for (const auto qp : make_range(nqp))
        _ad_u_dot[qp] = _ad_zero;

      if (_time_integrator && _time_integrator->dt())
      {
        for (const auto i : make_range(num_dofs))
          _ad_dofs_dot[i] = _ad_dof_values[i];
        for (const auto i : make_range(num_dofs))
          _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i],
                                                     _dof_indices[i],
                                                     _need_ad_u_dotdot ? _ad_dofs_dotdot[i]
                                                                       : _ad_real_dummy);

        for (const auto i : make_range(num_dofs))
          for (const auto qp : make_range(nqp))
            _ad_u_dot[qp] += (*_current_phi)[i][qp] * _ad_dofs_dot[i];
      }
      else if (!_time_integrator)
      {
        for (const auto i : make_range(num_dofs))
          _ad_dofs_dot[i] = _dof_values_dot[i];
        for (const auto qp : make_range(nqp))
          _ad_u_dot[qp] = _u_dot[qp];
      }
    }

    if (_need_ad_u_dotdot)
    {
      _ad_u_dotdot.resize(nqp);
      for (const auto qp : make_range(nqp))
        _ad_u_dotdot[qp] = _ad_zero;

      if (_time_integrator && _time_integrator->dt())
        for (const auto i : make_range(num_dofs))
          for (const auto qp : make_range(nqp))
            _ad_u_dotdot[qp] += (*_current_phi)[i][qp] * _ad_dofs_dotdot[i];
      else if (!_time_integrator)
        for (const auto qp : make_range(nqp))
          _ad_u_dotdot[qp] = _u_dotdot[qp];
    }

    if (_need_ad_grad_u_dot)
    {
      _ad_grad_u_dot.resize(nqp);
      for (const auto qp : make_range(nqp))
        _ad_grad_u_dot[qp] = _ad_zero;

      if (_time_integrator && _time_integrator->dt())
      {
        // The latter check here is for handling the fact that we have not yet implemented
        // calculation of ad_grad_phi for neighbor and neighbor-face, so if we are in that
        // situation we need to default to using the non-ad grad_phi
        if (_displaced && _current_ad_grad_phi)
          for (const auto i : make_range(num_dofs))
            for (const auto qp : make_range(nqp))
              _ad_grad_u_dot[qp] += _ad_dofs_dot[i] * (*_current_ad_grad_phi)[i][qp];
        else
          for (const auto i : make_range(num_dofs))
            for (const auto qp : make_range(nqp))
              _ad_grad_u_dot[qp] += _ad_dofs_dot[i] * (*_current_grad_phi)[i][qp];
      }
      else if (!_time_integrator)
        for (const auto qp : make_range(nqp))
          _ad_grad_u_dot[qp] = _grad_u_dot[qp];
    }
  }
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
  const auto nqps = u.size();
  const auto ndofs = dof_values.size();
  for (const auto qp : make_range(nqps))
    u[qp] *= 0.;
  for (const auto qp : make_range(nqps))
    for (const auto i : make_range(ndofs))
      u[qp] += (*_phi)[i][qp] * dof_values[i];
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
  const auto nqps = u.size();
  const auto ndofs = dof_values.size();
  for (const auto qp : make_range(nqps))
    u[qp] *= 0.;
  for (const auto qp : make_range(nqps))
    for (const auto i : make_range(ndofs))
      u[qp] += (*_phi)[i][qp] * dof_values[i];
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
  for (const auto j : make_range(_count))
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
  const auto num_dofs = _dof_indices.size();
  for (const auto j : make_range(_count))
  {
    unsigned int inc = (isNodal() ? j : j * num_dofs);
    for (const auto i : make_range(num_dofs))
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
  for (const auto qp : make_range(nqp))
  {
    _increment[qp] = 0.;
    for (const auto i : make_range(num_dofs))
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
    for (const auto qp : make_range(nqp))
    {
      for (const auto i : make_range(num_dofs))
        for (const auto j : make_range(_count))
          _increment[qp](j) += (*_phi)[i][qp] * increment_vec(_dof_indices[i] + j);
    }
  }
  else
  {
    for (const auto qp : make_range(nqp))
    {
      unsigned int n = 0;
      for (const auto j : make_range(_count))
        for (const auto i : make_range(num_dofs))
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
    const auto n_dof_indices = _dof_indices.size();
    for (const auto j : make_range(_count))
    {
      _increment[0](j) = increment_vec(_dof_indices[0] + n);
      n += n_dof_indices;
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
               var().name(),
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
               var().name(),
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
               var().name(),
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
               var().name(),
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
      fetchADNodalValues();
  }
  else
    zeroSizeDofValues();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::fetchADNodalValues()
{
  auto n = _dof_indices.size();
  libmesh_assert(n);
  _ad_dof_values.resize(n);

  if (_need_ad_u_dot)
    _ad_dofs_dot.resize(n);
  if (_need_ad_u_dotdot)
    _ad_dofs_dotdot.resize(n);

  const bool do_derivatives =
      ADReal::do_derivatives && _sys.number() == _subproblem.currentNlSysNum();

  for (decltype(n) i = 0; i < n; ++i)
  {
    _ad_dof_values[i] = _vector_tags_dof_u[_solution_tag][i];
    if (do_derivatives)
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);
    assignADNodalValue(_ad_dof_values[i], i);

    if (_need_ad_u_dot)
    {
      if (_time_integrator && _time_integrator->dt())
      {
        _ad_dofs_dot[i] = _ad_dof_values[i];
        _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i],
                                                   _dof_indices[i],
                                                   _need_ad_u_dotdot ? _ad_dofs_dotdot[i]
                                                                     : _ad_real_dummy);
      }
      // Executing something with a time derivative at initial should not put a NaN
      else if (_time_integrator && !_time_integrator->dt())
        _ad_dofs_dot[i] = 0.;
      else
        mooseError("AD nodal time derivatives not implemented for variables without a time "
                   "integrator (auxiliary variables)");
    }
  }
}

template <>
void
MooseVariableData<RealEigenVector>::fetchADNodalValues()
{
  mooseError("I do not know how to support AD with array variables");
}

template <>
void
MooseVariableData<Real>::assignADNodalValue(const ADReal & value, const unsigned int &)
{
  _ad_nodal_value = value;
}

template <>
void
MooseVariableData<RealVectorValue>::assignADNodalValue(const ADReal & value,
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
  _has_dof_indices = _dof_indices.size();
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

      const auto num_dofs = _dof_indices.size();
      for (auto & dof_u : _vector_tags_dof_u)
        dof_u.resize(num_dofs);

      for (auto & dof_u : _matrix_tags_dof_u)
        dof_u.resize(num_dofs);

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

  if (!_dof_indices.empty())
    _has_dof_indices = true;
  else
    _has_dof_indices = false;
}

template class MooseVariableData<Real>;
template class MooseVariableData<RealVectorValue>;
template class MooseVariableData<RealEigenVector>;
