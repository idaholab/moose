//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeBlockOrientationBase.h"
#include "MooseMesh.h"

#include "libmesh/mesh_tools.h"

InputParameters
ComputeBlockOrientationBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  return params;
}

ComputeBlockOrientationBase::ComputeBlockOrientationBase(const InputParameters & parameters)
  : ElementUserObject(parameters)
{
}

EulerAngles
ComputeBlockOrientationBase::getBlockOrientation(SubdomainID block) const
{
  return _block_ea_values.at(block);
}

void
ComputeBlockOrientationBase::initialize()
{
  _block_ea_values.clear();
}
