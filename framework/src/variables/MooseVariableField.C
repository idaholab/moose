//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseVariableField.h"
#include "SystemBase.h"
#include "TimeIntegrator.h"

#include "libmesh/fe_base.h"

using namespace Moose;

template <typename OutputType>
InputParameters
MooseVariableField<OutputType>::validParams()
{
  return MooseVariableFieldBase::validParams();
}

template <typename OutputType>
MooseVariableField<OutputType>::MooseVariableField(const InputParameters & parameters)
  : MooseVariableFieldBase(parameters),
    Moose::FunctorBase<typename Moose::ADType<OutputType>::type>(name()),
    MeshChangedInterface(parameters),
    _time_integrator(_sys.getTimeIntegrator())
{
}

namespace
{
template <typename OutputType>
struct FEBaseHelper
{
  typedef FEBase type;
};

template <>
struct FEBaseHelper<RealVectorValue>
{
  typedef FEVectorBase type;
};
}

template <typename OutputType>
template <typename Shapes, typename Solution, typename GradShapes, typename GradSolution>
void
MooseVariableField<OutputType>::computeSolution(const Elem * const elem,
                                                const QBase * const qrule,
                                                const StateArg & state,
                                                const Shapes & phi,
                                                Solution & local_soln,
                                                const GradShapes & grad_phi,
                                                GradSolution & grad_local_soln,
                                                Solution & dot_local_soln) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);
  std::vector<ADReal> dof_values;
  std::vector<ADReal> dof_values_dot;
  dof_values.reserve(dof_indices.size());

  const bool computing_dot = _time_integrator && _time_integrator->dt();
  if (computing_dot)
    dof_values_dot.reserve(dof_indices.size());

  const bool do_derivatives =
      ADReal::do_derivatives && _sys.number() == _subproblem.currentNlSysNum();

  // It's not safe to use solutionState(0) because it returns the libMesh System solution member
  // which is wrong during things like finite difference Jacobian evaluation, e.g. when PETSc
  // perturbs the solution vector we feed these perturbations into the current_local_solution
  // while the libMesh solution is frozen in the non-perturbed state
  const auto & global_soln = (state.state == 0)
                                 ? *_sys.currentSolution()
                                 : _sys.solutionState(state.state, state.iteration_type);
  for (const auto dof_index : dof_indices)
  {
    dof_values.push_back(ADReal(global_soln(dof_index)));
    if (do_derivatives && state.state == 0)
      Moose::derivInsert(dof_values.back().derivatives(), dof_index, 1.);
    if (computing_dot)
    {
      if (_var_kind == Moose::VAR_NONLINEAR)
      {
        dof_values_dot.push_back(dof_values.back());
        _time_integrator->computeADTimeDerivatives(
            dof_values_dot.back(), dof_index, _ad_real_dummy);
      }
      else
        dof_values_dot.push_back((*_sys.solutionUDot())(dof_index));
    }
  }

  const auto n_qp = qrule->n_points();
  local_soln.resize(n_qp);
  grad_local_soln.resize(n_qp);
  if (computing_dot)
    dot_local_soln.resize(n_qp);

  for (const auto qp : make_range(n_qp))
  {
    local_soln[qp] = 0;
    grad_local_soln[qp] = 0;
    if (computing_dot)
      dot_local_soln[qp] = 0;
    for (const auto i : index_range(dof_indices))
    {
      local_soln[qp] += dof_values[i] * phi[i][qp];
      grad_local_soln[qp] += dof_values[i] * grad_phi[i][qp];
      if (computing_dot)
        dot_local_soln[qp] += dof_values_dot[i] * phi[i][qp];
    }
  }
}

template <typename OutputType>
void
MooseVariableField<OutputType>::evaluateOnElement(const ElemQpArg & elem_qp,
                                                  const StateArg & state) const
{
  mooseAssert(this->hasBlocks(std::get<0>(elem_qp)->subdomain_id()),
              "This variable doesn't exist in the requested block!");

  const Elem * const elem = std::get<0>(elem_qp);
  if (elem != _current_elem_qp_functor_elem)
  {
    _current_elem_qp_functor_elem = elem;
    const QBase * const qrule_template = std::get<2>(elem_qp);

    using FEBaseType = typename FEBaseHelper<OutputType>::type;
    std::unique_ptr<FEBaseType> fe(FEBaseType::build(elem->dim(), _fe_type));
    std::unique_ptr<QBase> qrule(QBase::build(
        qrule_template->type(), qrule_template->get_dim(), qrule_template->get_order()));

    const auto & phi = fe->get_phi();
    const auto & dphi = fe->get_dphi();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem);

    computeSolution(elem,
                    qrule.get(),
                    state,
                    phi,
                    _current_elem_qp_functor_sln,
                    dphi,
                    _current_elem_qp_functor_gradient,
                    _current_elem_qp_functor_dot);
  }
}

template <>
void
MooseVariableField<RealEigenVector>::evaluateOnElement(const ElemQpArg &, const StateArg &) const
{
  mooseError("evaluate not implemented for array variables");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(const ElemQpArg & elem_qp, const StateArg & state) const
{
  evaluateOnElement(elem_qp, state);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_sln.size(),
              "The requested " << qp << " is outside our solution size");
  return _current_elem_qp_functor_sln[qp];
}

template <typename OutputType>
typename MooseVariableField<OutputType>::GradientType
MooseVariableField<OutputType>::evaluateGradient(const ElemQpArg & elem_qp,
                                                 const StateArg & state) const
{
  evaluateOnElement(elem_qp, state);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_gradient.size(),
              "The requested " << qp << " is outside our gradient size");
  return _current_elem_qp_functor_gradient[qp];
}

template <typename OutputType>
typename MooseVariableField<OutputType>::DotType
MooseVariableField<OutputType>::evaluateDot(const ElemQpArg & elem_qp, const StateArg & state) const
{
  mooseAssert(_time_integrator && _time_integrator->dt(),
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");
  evaluateOnElement(elem_qp, state);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_dot.size(),
              "The requested " << qp << " is outside our dot size");
  return _current_elem_qp_functor_dot[qp];
}

template <typename OutputType>
void
MooseVariableField<OutputType>::evaluateOnElementSide(const ElemSideQpArg & elem_side_qp,
                                                      const StateArg & state) const
{
  mooseAssert(this->hasBlocks(std::get<0>(elem_side_qp)->subdomain_id()),
              "This variable doesn't exist in the requested block!");

  const Elem * const elem = std::get<0>(elem_side_qp);
  const auto side = std::get<1>(elem_side_qp);
  if (elem != _current_elem_side_qp_functor_elem_side.first ||
      side != _current_elem_side_qp_functor_elem_side.second)
  {
    _current_elem_side_qp_functor_elem_side = std::make_pair(elem, side);
    const QBase * const qrule_template = std::get<3>(elem_side_qp);

    using FEBaseType = typename FEBaseHelper<OutputType>::type;
    std::unique_ptr<FEBaseType> fe(FEBaseType::build(elem->dim(), _fe_type));
    std::unique_ptr<QBase> qrule(QBase::build(
        qrule_template->type(), qrule_template->get_dim(), qrule_template->get_order()));

    const auto & phi = fe->get_phi();
    const auto & dphi = fe->get_dphi();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem, side);

    computeSolution(elem,
                    qrule.get(),
                    state,
                    phi,
                    _current_elem_side_qp_functor_sln,
                    dphi,
                    _current_elem_side_qp_functor_gradient,
                    _current_elem_side_qp_functor_dot);
  }
}

template <>
void
MooseVariableField<RealEigenVector>::evaluateOnElementSide(const ElemSideQpArg &,
                                                           const StateArg &) const
{
  mooseError("evaluate not implemented for array variables");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(const ElemSideQpArg & elem_side_qp,
                                         const StateArg & state) const
{
  evaluateOnElementSide(elem_side_qp, state);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_sln.size(),
              "The requested " << qp << " is outside our solution size");
  return _current_elem_side_qp_functor_sln[qp];
}

template <typename OutputType>
typename MooseVariableField<OutputType>::GradientType
MooseVariableField<OutputType>::evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                                 const StateArg & state) const
{
  evaluateOnElementSide(elem_side_qp, state);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_gradient.size(),
              "The requested " << qp << " is outside our gradient size");
  return _current_elem_side_qp_functor_gradient[qp];
}

template <typename OutputType>
typename MooseVariableField<OutputType>::DotType
MooseVariableField<OutputType>::evaluateDot(const ElemSideQpArg & elem_side_qp,
                                            const StateArg & state) const
{
  mooseAssert(_time_integrator && _time_integrator->dt(),
              "A time derivative is being requested but we do not have a time integrator so we'll "
              "have no idea how to compute it");
  evaluateOnElementSide(elem_side_qp, state);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_dot.size(),
              "The requested " << qp << " is outside our dot size");
  return _current_elem_side_qp_functor_dot[qp];
}

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(const ElemPointArg & elem_point,
                                         const StateArg & state) const
{
  return (*this)(elem_point.makeElem(), state) +
         (elem_point.point - elem_point.elem->vertex_average()) *
             this->gradient(elem_point.makeElem(), state);
}

template <>
typename MooseVariableField<RealEigenVector>::ValueType
MooseVariableField<RealEigenVector>::evaluate(const ElemQpArg &, const StateArg &) const
{
  mooseError(
      "MooseVariableField::evaluate(ElemQpArg &, const StateArg &) overload not implemented for "
      "array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::ValueType
MooseVariableField<RealEigenVector>::evaluate(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("MooseVariableField::evaluate(ElemSideQpArg &, const StateArg &) overload not "
             "implemented for "
             "array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::ValueType
MooseVariableField<RealEigenVector>::evaluate(const ElemPointArg &, const StateArg &) const
{
  mooseError(
      "MooseVariableField::evaluate(ElemPointArg &, const StateArg &) overload not implemented for "
      "array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::GradientType
MooseVariableField<RealEigenVector>::evaluateGradient(const ElemQpArg &, const StateArg &) const
{
  mooseError("MooseVariableField::evaluateGradient(ElemQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::GradientType
MooseVariableField<RealEigenVector>::evaluateGradient(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("MooseVariableField::evaluateGradient(ElemSideQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::DotType
MooseVariableField<RealEigenVector>::evaluateDot(const ElemQpArg &, const StateArg &) const
{
  mooseError("MooseVariableField::evaluateDot(ElemQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::DotType
MooseVariableField<RealEigenVector>::evaluateDot(const ElemSideQpArg &, const StateArg &) const
{
  mooseError("MooseVariableField::evaluateDot(ElemSideQpArg &, const StateArg &) overload not "
             "implemented for array variables");
}

template <typename OutputType>
void
MooseVariableField<OutputType>::meshChanged()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
}

template <typename OutputType>
void
MooseVariableField<OutputType>::residualSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  MooseVariableFieldBase::residualSetup();
  FunctorBase<typename Moose::ADType<OutputType>::type>::residualSetup();
}

template <typename OutputType>
void
MooseVariableField<OutputType>::jacobianSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  MooseVariableFieldBase::jacobianSetup();
  FunctorBase<typename Moose::ADType<OutputType>::type>::jacobianSetup();
}

template <typename OutputType>
void
MooseVariableField<OutputType>::timestepSetup()
{
  MooseVariableFieldBase::timestepSetup();
  FunctorBase<typename Moose::ADType<OutputType>::type>::timestepSetup();
}

template class MooseVariableField<Real>;
template class MooseVariableField<RealVectorValue>;
template class MooseVariableField<RealEigenVector>;
