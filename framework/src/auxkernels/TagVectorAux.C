//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorAux.h"

template <>
InputParameters
validParams<TagVectorAux>()
{
  InputParameters params = validParams<AuxKernel>();

  MultiMooseEnum vtags("nontime time", "nontime", true);

  params.addParam<MultiMooseEnum>("vector_tags", vtags, "vector tag this auxkernel is working on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");
  return params;
}

TagVectorAux::TagVectorAux(const InputParameters & parameters)
  : AuxKernel(parameters), _v_var(coupled("v")), _nl_var(_nl_sys.getVariable(0, _v_var))
{
  auto & vtags = getParam<MultiMooseEnum>("vector_tags");

  if (vtags.size() > 1)
    mooseError("TagVectorAux takes one tag only");

  for (auto & tag : vtags)
    if (_subproblem.vectorTagExists(tag.name()))
      _tag_id = _subproblem.getVectorTagID(tag.name());
    else
      mooseError("Required tag: ", tag.name(), " does not exist");

  _tag_vector = &_nl_sys.getVector(_tag_id);
}

Real
TagVectorAux::computeValue()
{
  if (_var.isNodal())
  {
    auto node_index = _nl_var.nodalDofIndex();
    if (_tag_vector->first_local_index() <= node_index &&
        node_index < _tag_vector->last_local_index())
      return (*_tag_vector)(node_index);
    else
      return 0.0;
  }
  else
    mooseError("Require a nodal variable");
}
