//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVInitialConditionTempl.h"
#include "FEProblem.h"
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"

#include "libmesh/fe_interface.h"
#include "libmesh/quadrature.h"

template <typename T>
FVInitialConditionTempl<T>::FVInitialConditionTempl(const InputParameters & parameters)
  : FVInitialConditionBase(parameters),
    _fe_problem(*getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _tid(getParam<THREAD_ID>("_tid")),
    _t(_fe_problem.time()),
    _base_var(_sys.getActualFieldVariable<T>(parameters.get<THREAD_ID>("_tid"),
                                             parameters.get<VariableName>("variable")))
{
  if (!_base_var.isFV())
    mooseError("A finite volume initial condition has been defined for variable '",
               _base_var.name(),
               "' whereas the variable itself is not a finite volume variable!");
}

template <typename T>
FVInitialConditionTempl<T>::~FVInitialConditionTempl()
{
}

template <typename T>
void
FVInitialConditionTempl<T>::computeElement(const ElemInfo & elem_info)
{
  _base_var.prepareIC();

  if constexpr (libMesh::TensorTools::TensorTraits<T>::rank == 0)
  {
    const auto system_number = _base_var.sys().number();
    const auto var_number = _base_var.number();
    const auto dof_id = elem_info.dofIndices()[system_number][var_number];
    const auto dof_value = this->value(elem_info.centroid());

    _base_var.sys().solution().set(dof_id, dof_value);
  }
  // If we want this to work with real and array variables we can add the different cases here late
  else
  {
    // This line is supposed to throw an error when the user tries to compile this function with
    // types that are not supported. This is the reason we needed the always_false function. Hope as
    // C++ gets nicer, we can do this in a nicer way.
    static_assert(Moose::always_false<T>,
                  "Initial condition is not implemented for the used type!");
  }
}

template class FVInitialConditionTempl<Real>;
