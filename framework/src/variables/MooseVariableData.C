//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
MooseVariableData<OutputType>::MooseVariableData(const MooseVariableFE<OutputType> & var,
                                                 SystemBase & sys,
                                                 THREAD_ID tid,
                                                 Moose::ElementType element_type,
                                                 const QBase * const & qrule_in,
                                                 const QBase * const & qrule_face_in,
                                                 const Node * const & node,
                                                 const Elem * const & elem)

  : MooseVariableDataBase<OutputType>(var, sys, tid),
    _var(var),
    _fe_type(var.feType()),
    _var_num(var.number()),
    _assembly(_subproblem.assembly(_tid, var.kind() == Moose::VAR_SOLVER ? sys.number() : 0)),
    _element_type(element_type),
    _ad_zero(0),
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
template <bool constant_monomial,
          typename DestinationType,
          typename ShapeType,
          typename DofValuesType>
void
MooseVariableData<OutputType>::fill(DestinationType & dest,
                                    const ShapeType & phi,
                                    const DofValuesType & dof_values,
                                    const unsigned int nqp,
                                    const std::size_t num_shapes)
{
  if constexpr (constant_monomial)
    libmesh_ignore(num_shapes);

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
  constexpr bool is_value =
      std::is_same_v<dest_array_type, OutputType> ||
      std::is_same_v<dest_array_type, typename Moose::ADType<OutputType>::type>;
  constexpr bool is_gradient =
      std::is_same_v<dest_array_type, OutputGradient> ||
      std::is_same_v<dest_array_type, typename Moose::ADType<OutputGradient>::type>;
  constexpr bool is_second =
      std::is_same_v<dest_array_type, OutputSecond> ||
      std::is_same_v<dest_array_type, typename Moose::ADType<OutputSecond>::type>;
  constexpr bool is_divergence =
      std::is_same_v<dest_array_type, OutputDivergence> ||
      std::is_same_v<dest_array_type, typename Moose::ADType<OutputDivergence>::type>;
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
  if constexpr (constant_monomial)
  {
    mooseAssert(num_shapes == 1, "Should have only one shape function for a constant monomial");
    set_zero(0);
    accumulate(0, 0);
    for (unsigned int qp = 1; qp < nqp; ++qp)
      dest[qp] = dest[0];
  }
  // Non constant monomial case
  else
  {
    for (const auto qp : make_range(nqp))
      set_zero(qp);
    for (const auto i : make_range(num_shapes))
      for (const auto qp : make_range(nqp))
        accumulate(i, qp);
  }
}

template <typename OutputType>
template <bool constant_monomial>
void
MooseVariableData<OutputType>::computeValuesInternal()
{
  const auto num_dofs = _dof_indices.size();
  const auto num_shapes = num_dofs / _count;

  if (num_dofs > 0)
    fetchDofValues();

  const bool is_transient = _subproblem.isTransient();
  const auto nqp = _current_qrule->n_points();
  const auto & active_coupleable_matrix_tags =
      _subproblem.getActiveFEVariableCoupleableMatrixTags(_tid);

  // Map grad_phi using Eigen so that we can perform array operations easier
  if constexpr (std::is_same_v<OutputType, RealEigenVector>)
  {
    if (_qrule == _current_qrule)
    {
      _mapped_grad_phi.resize(num_shapes);
      for (const auto i : make_range(num_shapes))
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
      _mapped_grad_phi_face.resize(num_shapes);
      for (const auto i : make_range(num_shapes))
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

  // Curl
  if (_need_curl)
    fill<constant_monomial>(
        _curl_u, *_current_curl_phi, _vector_tags_dof_u[_solution_tag], nqp, num_shapes);
  if (is_transient && _need_curl_old)
    fill<constant_monomial>(
        _curl_u_old, *_current_curl_phi, _vector_tags_dof_u[_old_solution_tag], nqp, num_shapes);

  // Div
  if (_need_div)
    fill<constant_monomial>(
        _div_u, *_current_div_phi, _vector_tags_dof_u[_solution_tag], nqp, num_shapes);
  if (is_transient && _need_div_old)
    fill<constant_monomial>(
        _div_u_old, *_current_div_phi, _vector_tags_dof_u[_old_solution_tag], nqp, num_shapes);

  // Second
  if (_need_second)
    fill<constant_monomial>(
        _second_u, *_current_second_phi, _vector_tags_dof_u[_solution_tag], nqp, num_shapes);
  if (_need_second_previous_nl)
    fill<constant_monomial>(_second_u_previous_nl,
                            *_current_second_phi,
                            _vector_tags_dof_u[_previous_nl_solution_tag],
                            nqp,
                            num_shapes);

  // Vector tags
  for (auto tag : _required_vector_tags)
  {
    if (_need_vector_tag_u[tag] && _sys.hasVector(tag))
    {
      mooseAssert(_sys.getVector(tag).closed(), "Vector should be closed");
      fill<constant_monomial>(
          _vector_tag_u[tag], *_current_phi, _vector_tags_dof_u[tag], nqp, num_shapes);
    }
    if (_need_vector_tag_grad[tag] && _sys.hasVector(tag))
    {
      mooseAssert(_sys.getVector(tag).closed(), "Vector should be closed");
      fill<constant_monomial>(
          _vector_tag_grad[tag], *_current_grad_phi, _vector_tags_dof_u[tag], nqp, num_shapes);
    }
  }

  // Matrix tags
  for (auto tag : active_coupleable_matrix_tags)
    if (_need_matrix_tag_u[tag])
      fill<constant_monomial>(
          _matrix_tag_u[tag], *_current_phi, _matrix_tags_dof_u[tag], nqp, num_shapes);

  // Derivatives and old values
  if (is_transient)
  {
    if (_need_second_old)
      fill<constant_monomial>(_second_u_old,
                              *_current_second_phi,
                              _vector_tags_dof_u[_old_solution_tag],
                              nqp,
                              num_shapes);
    if (_need_second_older)
      fill<constant_monomial>(_second_u_older,
                              *_current_second_phi,
                              _vector_tags_dof_u[_older_solution_tag],
                              nqp,
                              num_shapes);
    if (_need_u_dot)
      fill<constant_monomial>(_u_dot, *_current_phi, _dof_values_dot, nqp, num_shapes);
    if (_need_u_dotdot)
      fill<constant_monomial>(_u_dotdot, *_current_phi, _dof_values_dotdot, nqp, num_shapes);
    if (_need_u_dot_old)
      fill<constant_monomial>(_u_dot_old, *_current_phi, _dof_values_dot_old, nqp, num_shapes);
    if (_need_u_dotdot_old)
      fill<constant_monomial>(
          _u_dotdot_old, *_current_phi, _dof_values_dotdot_old, nqp, num_shapes);

    if (_need_du_dot_du)
    {
      _du_dot_du.resize(nqp);
      for (const auto i : make_range(num_shapes))
        for (const auto qp : make_range(nqp))
          _du_dot_du[qp] = _dof_du_dot_du[i];
    }
    if (_need_du_dotdot_du)
    {
      _du_dotdot_du.resize(nqp);
      for (const auto i : make_range(num_shapes))
        for (const auto qp : make_range(nqp))
          _du_dotdot_du[qp] = _dof_du_dotdot_du[i];
    }

    if (_need_grad_dot)
      fill<constant_monomial>(_grad_u_dot, *_current_grad_phi, _dof_values_dot, nqp, num_shapes);
    if (_need_grad_dotdot)
      fill<constant_monomial>(
          _grad_u_dotdot, *_current_grad_phi, _dof_values_dotdot, nqp, num_shapes);
  }

  if (_need_ad)
    computeAD<constant_monomial>(num_dofs, nqp);
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeValues()
{
  computeValuesInternal</* constant_monomial = */ false>();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::computeConstantMonomialValues()
{
  if (_dof_indices.size() == 0)
    return;

  // Monomial optimizations are not appropriate after p-refinement
  if (_elem->p_level())
    computeValues();
  else
    computeValuesInternal</* constant_monomial = */ true>();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::fetchADDofValues()
{
  const auto num_dofs = _dof_indices.size();

  const bool do_derivatives = Moose::doDerivatives(_subproblem, _sys);

  _ad_dof_values.resize(num_dofs);
  for (const auto i : make_range(num_dofs))
    _ad_dof_values[i] = _vector_tags_dof_u[_solution_tag][i];
  // NOTE!  You have to do this AFTER setting the value!
  if (do_derivatives)
    for (const auto i : make_range(num_dofs))
      Moose::derivInsert(_ad_dof_values[i].derivatives(), _dof_indices[i], 1.);

  const bool is_transient = _subproblem.isTransient();
  if (is_transient && _need_ad_u_dot)
  {
    _ad_dofs_dot.resize(num_dofs);
    if (_need_ad_u_dotdot)
      _ad_dofs_dotdot.resize(num_dofs);

    if (_time_integrator)
    {
      if (_time_integrator->dt())
      {
        for (const auto i : make_range(num_dofs))
          _ad_dofs_dot[i] = _ad_dof_values[i];
        for (const auto i : make_range(num_dofs))
          _time_integrator->computeADTimeDerivatives(_ad_dofs_dot[i],
                                                     _dof_indices[i],
                                                     _need_ad_u_dotdot ? _ad_dofs_dotdot[i]
                                                                       : _ad_real_dummy);
      }
      else
        // Executing something with a time derivative at initial should not put a NaN
        for (const auto i : make_range(num_dofs))
        {
          _ad_dofs_dot[i] = 0.;
          if (_need_ad_u_dotdot)
            _ad_dofs_dotdot[i] = 0;
        }
    }
    // We are too early in the setup to have a time integrator, so we are not really using the
    // AD-derivatives. We set the AD value of the derivatives to the nonAD value
    else
      for (const auto i : make_range(num_dofs))
      {
        _ad_dofs_dot[i] = _dof_values_dot[i];
        if (_need_ad_u_dotdot)
          _ad_dofs_dotdot[i] = _dof_values_dotdot[i];
      }
  }
}

template <>
void
MooseVariableData<RealEigenVector>::fetchADDofValues()
{
  const auto num_dofs = _dof_indices.size();

  const bool do_derivatives = Moose::doDerivatives(_subproblem, _sys);
  const auto n_test = num_dofs / _count;
  mooseAssert(num_dofs == _count * n_test,
              "Our assertions around number of dofs, test functions, and count are incorrect");

  _ad_dof_values.resize(n_test);
  // Test is outer, count is inner
  for (const auto i : make_range(n_test))
  {
    _ad_dof_values[i].resize(_count);
    for (const auto j : make_range(_count))
    {
      auto & dual_number = _ad_dof_values[i](j);
      const auto global_dof_index = _dof_indices[j * n_test + i];
      dual_number = (*_sys.currentSolution())(global_dof_index);
      // NOTE!  You have to do this AFTER setting the value!
      if (do_derivatives)
        Moose::derivInsert(dual_number.derivatives(), global_dof_index, 1.);
    }
  }
}

template <typename OutputType>
template <bool constant_monomial>
void
MooseVariableData<OutputType>::computeAD(const unsigned int num_dofs, const unsigned int nqp)
{
  fetchADDofValues();
  const auto n_test = num_dofs / _count;

  // Values
  if (_need_ad_u)
    fill<constant_monomial>(_ad_u, *_current_phi, _ad_dof_values, nqp, n_test);
  // Grad
  if (_need_ad_grad_u)
  {
    // The latter check here is for handling the fact that we have not yet implemented
    // calculation of ad_grad_phi for neighbor and neighbor-face, so if we are in that
    // situation we need to default to using the non-ad grad_phi
    if (_displaced && _current_ad_grad_phi)
      fill<constant_monomial>(_ad_grad_u, *_current_ad_grad_phi, _ad_dof_values, nqp, n_test);
    else
      fill<constant_monomial>(_ad_grad_u, *_current_grad_phi, _ad_dof_values, nqp, n_test);
  }
  // Second
  if constexpr (std::is_same_v<OutputType, Real>)
    if (_need_ad_second_u)
      fill<constant_monomial>(_ad_second_u, *_current_second_phi, _ad_dof_values, nqp, n_test);
  // Curl
  if (_need_ad_curl_u)
    fill<constant_monomial>(_ad_curl_u, *_current_curl_phi, _ad_dof_values, nqp, n_test);

  const bool is_transient = _subproblem.isTransient();
  if (is_transient)
  {
    if (_need_ad_u_dot)
    {
      if (_time_integrator)
        fill<constant_monomial>(_ad_u_dot, *_current_phi, _ad_dofs_dot, nqp, n_test);
      // We are too early in the setup to have a time integrator, so we are not really using the
      // AD-derivatives. We set the AD value of the derivatives to the nonAD value
      else
      {
        _ad_u_dot.resize(nqp);
        for (const auto qp : make_range(nqp))
          _ad_u_dot[qp] = _u_dot[qp];
      }
    }

    if (_need_ad_u_dotdot)
    {
      if (_time_integrator)
        fill<constant_monomial>(_ad_u_dotdot, *_current_phi, _ad_dofs_dotdot, nqp, n_test);
      else
      {
        _ad_u_dotdot.resize(nqp);
        for (const auto qp : make_range(nqp))
          _ad_u_dotdot[qp] = _u_dotdot[qp];
      }
    }

    if (_need_ad_grad_u_dot)
    {
      if (_time_integrator)
      {
        // The latter check here is for handling the fact that we have not yet implemented
        // calculation of ad_grad_phi for neighbor and neighbor-face, so if we are in that
        // situation we need to default to using the non-ad grad_phi
        if (_displaced && _current_ad_grad_phi)
          fill<constant_monomial>(_ad_grad_u_dot, *_current_ad_grad_phi, _ad_dofs_dot, nqp, n_test);
        else
          fill<constant_monomial>(_ad_grad_u_dot, *_current_grad_phi, _ad_dofs_dot, nqp, n_test);
      }
      else
      {
        _ad_grad_u_dot.resize(nqp);
        for (const auto qp : make_range(nqp))
          _ad_grad_u_dot[qp] = _grad_u_dot[qp];
      }
    }
  }
}

template <typename OutputType>
void
MooseVariableData<OutputType>::setDofValue(const DofValue & value, unsigned int index)
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
MooseVariableData<OutputType>::setDofValues(const DenseVector<DofValue> & values)
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
                                                const DofValue & v)
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
typename MooseVariableData<OutputType>::DofValue
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
typename MooseVariableData<OutputType>::DofValue
MooseVariableData<OutputType>::getElementalValue(const Elem * elem,
                                                 const Moose::SolutionState state,
                                                 const unsigned int idx) const
{
  static thread_local std::vector<dof_id_type> dof_indices;
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
                                                      const Moose::SolutionState state,
                                                      const unsigned int idx) const
{
  mooseDeprecated(
      "getElementalValue has a really bad API name. It is retrieving a value from the solution "
      "vector for a particular dof index. Generally speaking it has absolutely no equivalence to "
      "an 'elemental' value, which most people would consider to be something like an element "
      "average value");

  static thread_local std::vector<dof_id_type> dof_indices;
  _dof_map.array_dof_indices(elem, dof_indices, _var_num);
  mooseAssert(dof_indices.size() % _count == 0,
              "The number of array dof indices should divide cleanly by the variable count");
  const auto num_shapes = dof_indices.size() / _count;

  RealEigenVector v(_count);

  switch (state)
  {
    case Moose::Current:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = (*_sys.currentSolution())(dof_indices[i * num_shapes + idx]);
      break;

    case Moose::Old:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = _sys.solutionOld()(dof_indices[i * num_shapes + idx]);
      break;

    case Moose::Older:
      for (unsigned int i = 0; i < _count; ++i)
        v(i) = _sys.solutionOlder()(dof_indices[i * num_shapes + idx]);
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
  if constexpr (std::is_same<OutputType, RealEigenVector>::value)
    _dof_map.array_dof_indices(elem, dof_indices, _var_num);
  else
    _dof_map.dof_indices(elem, dof_indices, _var_num);
}

template <typename OutputType>
void
MooseVariableData<OutputType>::addSolution(NumericVector<Number> & sol,
                                           const DenseVector<Number> & v) const
{
  sol.add_vector(v, _dof_indices);
}

template <typename OutputType>
const typename MooseVariableData<OutputType>::DofValues &
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
const typename MooseVariableData<OutputType>::DofValues &
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
const typename MooseVariableData<OutputType>::DofValues &
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
const typename MooseVariableData<OutputType>::DofValues &
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
    fetchDofValues();
    assignNodalValue();

    if (_need_ad)
    {
      fetchADDofValues();
      assignADNodalValue();
    }
  }
  else
    zeroSizeDofValues();
}

template <typename OutputType>
void
MooseVariableData<OutputType>::assignADNodalValue()
{
  mooseAssert(_ad_dof_values.size(), "The AD dof values container must have size greater than 0");
  _ad_nodal_value = _ad_dof_values[0];
}

template <>
void
MooseVariableData<RealVectorValue>::assignADNodalValue()
{
  const auto num_dofs = _dof_indices.size();
  mooseAssert(_ad_dof_values.size() == num_dofs,
              "Our dof values container size should match the dof indices container size");
  for (const auto i : make_range(num_dofs))
    _ad_nodal_value(i) = _ad_dof_values[i];
}

template <typename OutputType>
void
MooseVariableData<OutputType>::prepareIC()
{
  if constexpr (std::is_same<RealEigenVector, OutputType>::value)
    _dof_map.array_dof_indices(_elem, _dof_indices, _var_num);
  else
    _dof_map.dof_indices(_elem, _dof_indices, _var_num);

  mooseAssert(_dof_indices.size() % _count == 0,
              "The number of dof indices should divide cleanly by the variable count");
  const auto num_shapes = _dof_indices.size() / _count;
  _vector_tags_dof_u[_solution_tag].resize(num_shapes);

  unsigned int nqp = _qrule->n_points();
  _vector_tag_u[_solution_tag].resize(nqp);
}

template <typename OutputType>
void
MooseVariableData<OutputType>::prepare()
{
  if constexpr (std::is_same<OutputType, RealEigenVector>::value)
    _dof_map.array_dof_indices(_elem, _dof_indices, _var_num);
  else
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
  if constexpr (std::is_same<OutputType, RealEigenVector>::value)
    _dof_map.array_dof_indices(_node, _dof_indices, _var_num);
  else
    _dof_map.dof_indices(_node, _dof_indices, _var_num);

  const auto n_dofs = _dof_indices.size();
  if (n_dofs)
  {
    // For standard variables. _nodal_dof_index is retrieved by nodalDofIndex() which is used in
    // NodalBC for example
    _nodal_dof_index = _dof_indices[0];
    _has_dof_indices = true;
  }
  else
    _has_dof_indices = false;
}

template <typename OutputType>
void
MooseVariableData<OutputType>::reinitAux()
{
  /* FIXME: this method is only for elemental auxiliary variables, so
   * we may want to rename it */
  if (_elem)
  {
    if constexpr (std::is_same<RealEigenVector, OutputType>::value)
      _dof_map.array_dof_indices(_elem, _dof_indices, _var_num);
    else
      _dof_map.dof_indices(_elem, _dof_indices, _var_num);
    if (_elem->n_dofs(_sys.number(), _var_num) > 0)
    {
      // FIXME: Setting _nodal_dof_index inside of a method that apparently is only for "elemental"
      // aux variables seems absolutely absurd
      _nodal_dof_index = _dof_indices[0];

      fetchDofValues();

      mooseAssert(_dof_indices.size() % _count == 0,
                  "The number of dof indices should be cleanly divisible by the variable count");
      const auto num_shapes = _dof_indices.size() / _count;
      for (auto & dof_u : _vector_tags_dof_u)
        dof_u.resize(num_shapes);

      for (auto & dof_u : _matrix_tags_dof_u)
        dof_u.resize(num_shapes);

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
