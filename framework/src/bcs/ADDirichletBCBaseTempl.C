//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDirichletBCBaseTempl.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "libmesh/node.h"

template <typename T>
InputParameters
ADDirichletBCBaseTempl<T>::validParams()
{
  InputParameters params = ADNodalBCTempl<T, ADDirichletBCBase>::validParams();
  params.addParam<bool>(
      "preset", true, "Whether or not to preset the BC (apply the value before the solve begins).");
  return params;
}

template <typename T>
ADDirichletBCBaseTempl<T>::ADDirichletBCBaseTempl(const InputParameters & parameters)
  : ADNodalBCTempl<T, ADDirichletBCBase>(parameters)
{
}

template <typename T>
void
ADDirichletBCBaseTempl<T>::computeValue(NumericVector<Number> & current_solution)
{
  mooseAssert(this->_preset, "BC is not preset");

  if (_var.isNodalDefined())
  {
    const auto n_comp = _current_node->n_comp(_sys.number(), _var.number());
    const auto value = MetaPhysicL::raw_value(computeQpValue());
    for (const auto i : make_range(n_comp))
    {
      const auto dof_idx = _current_node->dof_number(_sys.number(), _var.number(), i);
      if constexpr (std::is_same<T, Real>::value)
      {
        mooseAssert(n_comp == 1, "This should only be unity");
        current_solution.set(dof_idx, value);
      }
      else
      {
        if (shouldSetComp(i))
          current_solution.set(dof_idx, value(i));
      }
    }
  }
}

template <typename T>
typename Moose::ADType<T>::type
ADDirichletBCBaseTempl<T>::computeQpResidual()
{
  return _u - computeQpValue();
}

template class ADDirichletBCBaseTempl<Real>;
template class ADDirichletBCBaseTempl<RealVectorValue>;
