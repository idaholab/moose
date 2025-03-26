//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimestepSize.h"
#include "FEProblem.h"

registerMooseObject("MooseApp", TimestepSize);

InputParameters
TimestepSize::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Reports the timestep size");

  // Not supported
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

TimestepSize::TimestepSize(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _feproblem(dynamic_cast<FEProblemBase &>(_subproblem))
{
}

Real
TimestepSize::getValue() const
{
  return _feproblem.dt();
}
