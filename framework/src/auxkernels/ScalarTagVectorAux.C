//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarTagVectorAux.h"

registerMooseObject("MooseApp", ScalarTagVectorAux);

defineLegacyParams(ScalarTagVectorAux);

InputParameters
ScalarTagVectorAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();

  params.addParam<std::string>("vector_tag", "TagName", "Tag Name this Aux works on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");

  params.addClassDescription("Couple a tag vector, and return its value");
  return params;
}

ScalarTagVectorAux::ScalarTagVectorAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _tag_id(_subproblem.getVectorTagID(getParam<std::string>("vector_tag"))),
    _v(coupledVectorTagScalarValue("v", _tag_id))
{
}

Real
ScalarTagVectorAux::computeValue()
{
  return _v[0];
}
