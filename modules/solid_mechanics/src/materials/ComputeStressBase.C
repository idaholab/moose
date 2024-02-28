//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeStressBase.h"
#include "ComputeElasticityTensorBase.h"
#include "Function.h"

InputParameters
ComputeStressBase::validParams()
{
  InputParameters params = ComputeGeneralStressBase::validParams();
  params.suppressParameter<bool>("use_displaced_mesh");
  return params;
}

ComputeStressBase::ComputeStressBase(const InputParameters & parameters)
  : ComputeGeneralStressBase(parameters)
{
  if (getParam<bool>("use_displaced_mesh"))
    mooseError("The stress calculator needs to run on the undisplaced mesh.");
}
