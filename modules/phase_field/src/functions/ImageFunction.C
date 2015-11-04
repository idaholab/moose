/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ImageFunction.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ImageFunction>()
{
  // Define the general parameters
  InputParameters params = validParams<Function>();
  params += validParams<ImageSampler>();
  params.addClassDescription("Function with values sampled from a given image stack");
  return params;
}

ImageFunction::ImageFunction(const InputParameters & parameters) :
    ImageSampler(parameters),
    Function(parameters)
{
}

ImageFunction::~ImageFunction()
{
}

void
ImageFunction::initialSetup()
{
  FEProblem * fe_problem = this->getParam<FEProblem *>("_fe_problem");
  MooseMesh & mesh = fe_problem->mesh();
  setupImageSampler(mesh);
}

Real
ImageFunction::value(Real /*t*/, const Point & p)
{
  return sample(p);
}
