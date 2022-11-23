//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorArrayVariableAux.h"

registerMooseObject("MooseApp", TagVectorArrayVariableAux);

InputParameters
TagVectorArrayVariableAux::validParams()
{
  InputParameters params = TagAuxBase<ArrayAuxKernel>::validParams();

  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  params.addClassDescription(
      "Couple a tagged vector, and return its evaluations at degree of freedom "
      "indices corresponding to the coupled array variable.");
  return params;
}

TagVectorArrayVariableAux::TagVectorArrayVariableAux(const InputParameters & parameters)
  : TagAuxBase<ArrayAuxKernel>(parameters), _v(coupledVectorTagArrayDofValue("v", "vector_tag"))
{
  checkCoupledVariable(getArrayVar("v", 0), &_var);
}

void
TagVectorArrayVariableAux::compute()
{
  const auto n_local_dofs = _var.numberOfDofs();
  _local_sol.resize(n_local_dofs);
  for (const auto i : make_range(n_local_dofs))
    _local_sol(i) = _v[i];
  _var.setDofValues(_local_sol);
}
