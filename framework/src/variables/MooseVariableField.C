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
  : MooseVariableFieldBase(parameters)
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
typename Moose::ADType<OutputType>::type
MooseVariableField<OutputType>::evaluate(const QpArg & qp_arg, const unsigned int state) const
{
  mooseAssert(this->hasBlocks(std::get<0>(qp_arg)->subdomain_id()),
              "This variable doesn't exist in the requested block!");

  const Elem * const elem = std::get<0>(qp_arg);
  const auto qp = std::get<1>(qp_arg);
  if (elem != _current_functor_elem)
  {
    _current_functor_elem = elem;
    const QBase * const qrule_template = std::get<2>(qp_arg);

    using FEBaseType = typename FEBaseHelper<OutputType>::type;
    std::unique_ptr<FEBaseType> fe(FEBaseType::build(elem->dim(), _fe_type));
    std::unique_ptr<QBase> qrule(QBase::build(
        qrule_template->type(), qrule_template->get_dim(), qrule_template->get_order()));

    const auto & phi = fe->get_phi();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem);

    std::vector<dof_id_type> dof_indices;
    _dof_map.dof_indices(elem, dof_indices, _var_num);
    std::vector<ADReal> dof_values;
    // It's not safe to use solutionState(0) because it returns the libMesh System solution member
    // which is wrong during things like finite difference Jacobian evaluation, e.g. when PETSc
    // perturbs the solution vector we feed these perturbations into the current_local_solution
    // while the libMesh solution is frozen in the non-perturbed state
    const auto & solution = (state == 0) ? *_sys.currentSolution() : _sys.solutionState(state);
    for (const auto dof_index : dof_indices)
    {
      dof_values.push_back(ADReal(solution(dof_index)));
      Moose::derivInsert(dof_values.back().derivatives(), dof_index, 1.);
    }

    const auto n_qp = qrule->n_points();
    _current_functor_sln.resize(n_qp);

    for (const auto qp : make_range(n_qp))
    {
      _current_functor_sln[qp] = 0;
      for (const auto i : index_range(dof_indices))
        _current_functor_sln[qp] += dof_values[i] * phi[i][qp];
    }
  }
  mooseAssert(qp < _current_functor_sln.size(),
              "The requested " << qp << " is outside our solution size");
  return _current_functor_sln[qp];
}

#else

template <typename OutputType>
typename Moose::ADType<OutputType>::type
MooseVariableField<OutputType>::evaluate(const QpArg &, unsigned int) const
{
  mooseError("MooseVariableField::evaluate(QpArg &, unsigned int) is only supported for global AD "
             "indexing");
}

#endif

template <>
typename Moose::ADType<RealEigenVector>::type
MooseVariableField<RealEigenVector>::evaluate(const QpArg &, unsigned int) const
{
  mooseError("MooseVariableField::evaluate(QpArg &, unsigned int) overload not implemented for "
             "array variables");
}

template <typename OutputType>
typename Moose::ADType<OutputType>::type
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

template class MooseVariableField<Real>;
template class MooseVariableField<RealVectorValue>;
template class MooseVariableField<RealEigenVector>;
