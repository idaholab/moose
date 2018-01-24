//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImageFunction.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ImageFunction>()
{
  // Define the general parameters
  InputParameters params = validParams<Function>();
  params += validParams<ImageSampler>();
  params.addClassDescription("Function with values sampled from a given image stack");
  return params;
}

ImageFunction::ImageFunction(const InputParameters & parameters)
  : ImageSampler(parameters), Function(parameters)
{
}

ImageFunction::~ImageFunction() {}

void
ImageFunction::initialSetup()
{
  FEProblemBase * fe_problem = this->getParam<FEProblemBase *>("_fe_problem_base");
  MooseMesh & mesh = fe_problem->mesh();
  setupImageSampler(mesh);
}

Real
ImageFunction::value(Real /*t*/, const Point & p)
{
  return sample(p);
}
