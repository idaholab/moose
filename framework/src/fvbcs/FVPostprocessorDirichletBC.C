//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPostprocessorDirichletBC.h"

registerMooseObject("MooseApp", FVPostprocessorDirichletBC);

InputParameters
FVPostprocessorDirichletBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params.addClassDescription("Defines a Dirichlet boundary condition for finite volume method.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The postprocessor to set the value to on the boundary.");
  return params;
}

FVPostprocessorDirichletBC::FVPostprocessorDirichletBC(const InputParameters & parameters)
  : FVDirichletBCBase(parameters), _postprocessor_value(getPostprocessorValue("postprocessor"))
{
}

ADReal
FVPostprocessorDirichletBC::boundaryValue(const FaceInfo & /*fi*/,
                                          const Moose::StateArg & /*state*/) const
{
  return _postprocessor_value;
}
