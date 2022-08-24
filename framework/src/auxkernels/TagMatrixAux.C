//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagMatrixAux.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", TagMatrixAux);

InputParameters
TagMatrixAux::validParams()
{
  InputParameters params = TagAuxBase<AuxKernel>::validParams();
  params.addParam<TagName>("matrix_tag", "TagName", "Tag Name this Aux works on");
  params.addClassDescription("Couple the diagonal of a tag matrix, and return its nodal value");
  return params;
}

TagMatrixAux::TagMatrixAux(const InputParameters & parameters)
  : TagAuxBase<AuxKernel>(parameters),
    _v(coupledMatrixTagValue("v", "matrix_tag")),
    _v_var(*getVar("v", 0))
{
  checkCoupledVariable(&_v_var, &_var);
}

Real
TagMatrixAux::computeValue()
{
  return _scaled ? _v[_qp] : _v[_qp] / _v_var.scalingFactor();
}
