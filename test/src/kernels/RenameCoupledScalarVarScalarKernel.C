//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenameCoupledScalarVarScalarKernel.h"

registerMooseObject("MooseTestApp", RenameCoupledScalarVarScalarKernel);

InputParameters
RenameCoupledScalarVarScalarKernel::validParams()
{
  InputParameters params = TestADScalarKernel::validParams();
  params.renameCoupledVar("v", "coupled_scalar_variable", "Coupled scalar variable");
  return params;
}

RenameCoupledScalarVarScalarKernel::RenameCoupledScalarVarScalarKernel(
    const InputParameters & parameters)
  : TestADScalarKernel(parameters)
{
}
