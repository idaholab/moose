//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenamedPostprocessorDiffusion.h"

registerMooseObject("MooseTestApp", RenamedPostprocessorDiffusion);

InputParameters
RenamedPostprocessorDiffusion::validParams()
{
  InputParameters params = DefaultPostprocessorDiffusion::validParams();
  params.renameParam("pps_name",
                     "diffusion_postprocessor",
                     "The name of the postprocessor we are going to use, if the name is not "
                     "found a default value of 0.1 is utilized for the postprocessor value");
  return params;
}

RenamedPostprocessorDiffusion::RenamedPostprocessorDiffusion(const InputParameters & parameters)
  : DefaultPostprocessorDiffusion(parameters)
{
}
