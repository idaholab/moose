//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorDirichletBC.h"

registerMooseObject("MooseApp", PostprocessorDirichletBC);

// Used by MOOSEDocs: syntax/Postprocessors/index.md
InputParameters
PostprocessorDirichletBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addClassDescription(
      "Dirichlet boundary condition with value prescribed by a Postprocessor value.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The postprocessor to set the value to on the boundary.");
  return params;
}

PostprocessorDirichletBC::PostprocessorDirichletBC(const InputParameters & parameters)
  : NodalBC(parameters), _postprocessor_value(getPostprocessorValue("postprocessor"))
{
}

Real
PostprocessorDirichletBC::computeQpResidual()
{
  return _u[_qp] - _postprocessor_value;
}
