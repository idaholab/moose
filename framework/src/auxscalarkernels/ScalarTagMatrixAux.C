//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarTagMatrixAux.h"
#include "SubProblem.h"

registerMooseObject("MooseApp", ScalarTagMatrixAux);

InputParameters
ScalarTagMatrixAux::validParams()
{
  InputParameters params = TagAuxBase<AuxScalarKernel>::validParams();

  params.addParam<TagName>("matrix_tag", "TagName", "Tag Name this Aux works on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");
  params.addClassDescription("Couple a tag matrix, and return its nodal value");
  return params;
}

ScalarTagMatrixAux::ScalarTagMatrixAux(const InputParameters & parameters)
  : TagAuxBase<AuxScalarKernel>(parameters),
    _v(coupledMatrixTagScalarValue("v", "matrix_tag")),
    _v_var(*getScalarVar("v", 0))
{
}

Real
ScalarTagMatrixAux::computeValue()
{
  return _scaled ? _v[0] : _v[0] / _v_var.scalingFactor();
}
