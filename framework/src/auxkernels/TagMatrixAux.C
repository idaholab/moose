//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagMatrixAux.h"

registerMooseObject("MooseApp", TagMatrixAux);

template <>
InputParameters
validParams<TagMatrixAux>()
{
  InputParameters params = validParams<TagVectorAux>();

  MultiMooseEnum mtags("system", "system", true);

  params.addParam<MultiMooseEnum>("matrix_tags", mtags, "matrix tag this auxkernel is working on");

  return params;
}

TagMatrixAux::TagMatrixAux(const InputParameters & parameters) : TagVectorAux(parameters)
{
  auto & mtags = getParam<MultiMooseEnum>("matrix_tags");

  if (mtags.size() > 1)
    mooseError("TagMatrixAux takes one tag only");

  for (auto & tag : mtags)
    if (_subproblem.matrixTagExists(tag.name()))
      _tag_id = _subproblem.getMatrixTagID(tag.name());
    else
      mooseError("Required tag: ", tag.name(), " does not exist");

  _tag_matrix = &_nl_sys.getMatrix(_tag_id);
}

Real
TagMatrixAux::computeValue()
{
  if (_var.isNodal())
  {
    auto node_index = _nl_var.nodalDofIndex();
    if (_tag_matrix->row_start() <= node_index && node_index < _tag_matrix->row_stop())
      return (*_tag_matrix)(node_index, node_index);
    else
      return 0.0;
  }
  else
    mooseError("Require a nodal variable");
}
