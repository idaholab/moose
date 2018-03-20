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
  InputParameters params = validParams<AuxKernel>();
  params.addParam<std::string>("matrix_tag", "TagName", "Tag Name this Aux works on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");

  return params;
}

TagMatrixAux::TagMatrixAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _tag_id(_subproblem.getMatrixTagID(getParam<std::string>("matrix_tag"))),
    _v(coupledMatrixTagValue("v", _tag_id))
{
}

Real
TagMatrixAux::computeValue()
{
  if (_var.isNodal())
  {
    return _v[_qp];
  }
  else
    mooseError("Require a nodal variable");
}
