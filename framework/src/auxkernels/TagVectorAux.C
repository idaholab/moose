//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TagVectorAux.h"

registerMooseObject("MooseApp", TagVectorAux);

defineLegacyParams(TagVectorAux);

InputParameters
TagVectorAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<TagName>("vector_tag", "Tag Name this Aux works on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_TIMESTEP_END};

  params.addClassDescription("Couple a tag vector, and return its nodal value");
  return params;
}

TagVectorAux::TagVectorAux(const InputParameters & parameters)
  : AuxKernel(parameters), _v(coupledVectorTagValue("v", "vector_tag"))
{
  auto & execute_on = getParam<ExecFlagEnum>("execute_on");
  if (execute_on.size() != 1 || !execute_on.contains(EXEC_TIMESTEP_END))
    mooseError("execute_on for TagVectorAux must be set to EXEC_TIMESTEP_END");
}

Real
TagVectorAux::computeValue()
{
  return _v[_qp];
}
