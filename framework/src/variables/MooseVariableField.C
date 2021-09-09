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

#include "libmesh/fe_base.h"

template <typename OutputType>
InputParameters
MooseVariableField<OutputType>::validParams()
{
  return MooseVariableFieldBase::validParams();
}

template <typename OutputType>
MooseVariableField<OutputType>::MooseVariableField(const InputParameters & parameters)
  : MooseVariableFieldBase(parameters), MeshChangedInterface(parameters)
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

#ifdef MOOSE_GLOBAL_AD_INDEXING

template <typename OutputType>
template <typename Shapes, typename Solution, typename GradShapes, typename GradSolution>
void
MooseVariableField<OutputType>::computeSolution(const Elem * const elem,
                                                const QBase * const qrule,
                                                const unsigned int state,
                                                const Shapes & phi,
                                                Solution & local_soln,
                                                const GradShapes & grad_phi,
                                                GradSolution & grad_local_soln) const
{
  std::vector<dof_id_type> dof_indices;
  _dof_map.dof_indices(elem, dof_indices, _var_num);
  std::vector<ADReal> dof_values;
  // It's not safe to use solutionState(0) because it returns the libMesh System solution member
  // which is wrong during things like finite difference Jacobian evaluation, e.g. when PETSc
  // perturbs the solution vector we feed these perturbations into the current_local_solution
  // while the libMesh solution is frozen in the non-perturbed state
  const auto & global_soln = (state == 0) ? *_sys.currentSolution() : _sys.solutionState(state);
  for (const auto dof_index : dof_indices)
  {
    dof_values.push_back(ADReal(global_soln(dof_index)));
    Moose::derivInsert(dof_values.back().derivatives(), dof_index, 1.);
  }

  const auto n_qp = qrule->n_points();
  local_soln.resize(n_qp);
  grad_local_soln.resize(n_qp);

  for (const auto qp : make_range(n_qp))
  {
    local_soln[qp] = 0;
    grad_local_soln[qp] = 0;
    for (const auto i : index_range(dof_indices))
    {
      local_soln[qp] += dof_values[i] * phi[i][qp];
      grad_local_soln[qp] += dof_values[i] * grad_phi[i][qp];
    }
  }
}

template <typename OutputType>
void
MooseVariableField<OutputType>::evaluateOnElement(const ElemQpArg & elem_qp,
                                                  const unsigned int state) const
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
                    _current_elem_qp_functor_gradient);
  }
}

template <>
void
MooseVariableField<RealEigenVector>::evaluateOnElement(const ElemQpArg &, const unsigned int) const
{
  mooseError("evaluate not implemented for array variables");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(const ElemQpArg & elem_qp, const unsigned int state) const
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
                                                 const unsigned int state) const
{
  evaluateOnElement(elem_qp, state);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_sln.size(),
              "The requested " << qp << " is outside our gradient size");
  return _current_elem_qp_functor_gradient[qp];
}

template <typename OutputType>
void
MooseVariableField<OutputType>::evaluateOnElementSide(const ElemSideQpArg & elem_side_qp,
                                                      const unsigned int state) const
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
                    _current_elem_side_qp_functor_gradient);
  }
}

template <>
void
MooseVariableField<RealEigenVector>::evaluateOnElementSide(const ElemSideQpArg &,
                                                           const unsigned int) const
{
  mooseError("evaluate not implemented for array variables");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(const ElemSideQpArg & elem_side_qp,
                                         const unsigned int state) const
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
                                                 const unsigned int state) const
{
  evaluateOnElementSide(elem_side_qp, state);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_gradient.size(),
              "The requested " << qp << " is outside our gradient size");
  return _current_elem_side_qp_functor_gradient[qp];
}

#else

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(const ElemQpArg &, unsigned int) const
{
  mooseError(
      "MooseVariableField::evaluate(ElemQpArg &, unsigned int) is only supported for global AD "
      "indexing");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(const ElemSideQpArg &, unsigned int) const
{
  mooseError(
      "MooseVariableField::evaluate(ElemSideQpArg &, unsigned int) is only supported for global AD "
      "indexing");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::GradientType
MooseVariableField<OutputType>::evaluateGradient(const ElemQpArg &, unsigned int) const
{
  mooseError("MooseVariableField::evaluateGradient(ElemQpArg &, unsigned int) is only supported "
             "for global AD indexing");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::GradientType
MooseVariableField<OutputType>::evaluateGradient(const ElemSideQpArg &, unsigned int) const
{
  mooseError("MooseVariableField::evaluateGradient(ElemSideQpArg &, unsigned int) is only "
             "supported for global AD indexing");
}

#endif

template <>
typename MooseVariableField<RealEigenVector>::ValueType
MooseVariableField<RealEigenVector>::evaluate(const ElemQpArg &, unsigned int) const
{
  mooseError("MooseVariableField::evaluate(ElemQpArg &, unsigned int) overload not implemented for "
             "array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::ValueType
MooseVariableField<RealEigenVector>::evaluate(const ElemSideQpArg &, unsigned int) const
{
  mooseError(
      "MooseVariableField::evaluate(ElemSideQpArg &, unsigned int) overload not implemented for "
      "array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::GradientType
MooseVariableField<RealEigenVector>::evaluateGradient(const ElemQpArg &, unsigned int) const
{
  mooseError("MooseVariableField::evaluateGradient(ElemQpArg &, unsigned int) overload not "
             "implemented for array variables");
}

template <>
typename MooseVariableField<RealEigenVector>::GradientType
MooseVariableField<RealEigenVector>::evaluateGradient(const ElemSideQpArg &, unsigned int) const
{
  mooseError("MooseVariableField::evaluateGradient(ElemSideQpArg &, unsigned int) overload not "
             "implemented for array variables");
}

template <typename OutputType>
typename MooseVariableField<OutputType>::ValueType
MooseVariableField<OutputType>::evaluate(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp, unsigned int state) const
{
  mooseAssert(this->hasBlocks(std::get<2>(tqp)),
              "This variable doesn't exist in the requested block!");
  const auto elem_type = std::get<0>(tqp);
  const auto qp = std::get<1>(tqp);
  switch (elem_type)
  {
    case Moose::ElementType::Element:
    {
      switch (state)
      {
        case 0:
          return adSln()[qp];

        case 1:
          return slnOld()[qp];

        case 2:
          return slnOlder()[qp];

        default:
          mooseError("Unsupported state ", state, " in MooseVariableField::evaluate");
      }
    }

    case Moose::ElementType::Neighbor:
    {
      switch (state)
      {
        case 0:
          return adSlnNeighbor()[qp];

        case 1:
          return slnOldNeighbor()[qp];

        default:
          mooseError("Unsupported state ", state, " in MooseVariableField::evaluate");
      }
    }

    default:
      mooseError("Unrecognized element type");
  }
}

template <typename OutputType>
typename MooseVariableField<OutputType>::GradientType
MooseVariableField<OutputType>::evaluateGradient(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp, unsigned int state) const
{
  mooseAssert(this->hasBlocks(std::get<2>(tqp)),
              "This variable doesn't exist in the requested block!");
  const auto elem_type = std::get<0>(tqp);
  const auto qp = std::get<1>(tqp);
  switch (elem_type)
  {
    case Moose::ElementType::Element:
    {
      switch (state)
      {
        case 0:
          return adGradSln()[qp];

        case 1:
          return gradSlnOld()[qp];

        default:
          mooseError("Unsupported state ", state, " in MooseVariableField::evaluateGradient");
      }
    }

    case Moose::ElementType::Neighbor:
    {
      switch (state)
      {
        case 0:
          return adGradSlnNeighbor()[qp];

        case 1:
          return gradSlnOldNeighbor()[qp];

        default:
          mooseError("Unsupported state ", state, " in MooseVariableField::evaluateGradient");
      }
    }

    default:
      mooseError("Unrecognized element type");
  }
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
  Moose::Functor<typename Moose::ADType<OutputType>::type>::residualSetup();
}

template <typename OutputType>
void
MooseVariableField<OutputType>::jacobianSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  MooseVariableFieldBase::jacobianSetup();
  Moose::Functor<typename Moose::ADType<OutputType>::type>::jacobianSetup();
}

template class MooseVariableField<Real>;
template class MooseVariableField<RealVectorValue>;
template class MooseVariableField<RealEigenVector>;
