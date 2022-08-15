//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorDofValueAux.h"

registerMooseObject("MooseApp", TagVectorDofValueAux);

InputParameters
TagVectorDofValueAux::validParams()
{
  InputParameters params = TagAuxBase<AuxKernel>::validParams();
  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  return params;
}

TagVectorDofValueAux::TagVectorDofValueAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters), _v(coupledVectorTagDofValue("v", "vector_tag"))

{
  auto & v_var = *getFieldVar("v", 0);

  if (v_var.feType() != _var.feType())
    paramError("variable",
               "The AuxVariable this AuxKernel is acting on has to have the same order and family "
               "as the variable 'v'");
}

void
TagVectorDofValueAux::compute()
{
  const auto n_local_dofs = _var.numberOfDofs();
  _local_sol.resize(n_local_dofs);
  for (const auto i : make_range(n_local_dofs))
    _local_sol(i) = _v[i];
  _var.setDofValues(_local_sol);
}
