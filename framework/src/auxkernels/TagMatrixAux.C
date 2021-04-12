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

defineLegacyParams(TagMatrixAux);

InputParameters
TagMatrixAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addParam<TagName>("matrix_tag", "TagName", "Tag Name this Aux works on");
  params.addRequiredCoupledVar("v",
                               "The coupled variable whose components are coupled to AuxVariable");
  params.set<ExecFlagEnum>("execute_on", true) = {EXEC_TIMESTEP_END};

  params.addClassDescription("Couple the diag of a tag matrix, and return its nodal value");
  return params;
}

TagMatrixAux::TagMatrixAux(const InputParameters & parameters)
  : AuxKernel(parameters), _v(coupledMatrixTagValue("v", "matrix_tag"))
{
  auto & execute_on = getParam<ExecFlagEnum>("execute_on");
  if (execute_on.size() != 1 || !execute_on.contains(EXEC_TIMESTEP_END))
    mooseError("execute_on for TagMatrixAux must be set to EXEC_TIMESTEP_END");
}

Real
TagMatrixAux::computeValue()
{
  return _v[_qp];
}
