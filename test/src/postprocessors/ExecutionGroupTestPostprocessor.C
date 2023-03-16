//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecutionGroupTestPostprocessor.h"
#include "libmesh/utility.h"

registerMooseObject("MooseTestApp", ExecutionGroupTestPostprocessor);

InputParameters
ExecutionGroupTestPostprocessor::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addClassDescription("Element postprocessor for checking execution order grouping");
  params.addParam<PostprocessorName>("depends_on", "Explicitly sort within an execution group");
  return params;
}

ExecutionGroupTestPostprocessor::ExecutionGroupTestPostprocessor(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters)
{
  if (isParamValid("depends_on"))
  {
    auto dummy = getPostprocessorValue("depends_on");
    libmesh_ignore(dummy);
  }
}

void
ExecutionGroupTestPostprocessor::initialize()
{
  if (_tid == 0)
    Moose::out << "I:" << name() << ' ';
  ElementIntegralVariablePostprocessor::initialize();
}

void
ExecutionGroupTestPostprocessor::finalize()
{
  if (_tid == 0)
    Moose::out << "F:" << name() << ' ';
  ElementIntegralVariablePostprocessor::finalize();
}
