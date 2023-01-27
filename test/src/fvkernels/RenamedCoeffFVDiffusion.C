//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RenamedCoeffFVDiffusion.h"

registerMooseObject("MooseTestApp", RenamedCoeffFVDiffusion);

InputParameters
RenamedCoeffFVDiffusion::validParams()
{
  InputParameters params = FVDiffusion::validParams();
  params.addClassDescription("Computes residual for diffusion operator for finite volume method.");
  params.renameParam("coeff", "diffusion_coeff", "The diffusion coefficient.");
  return params;
}

RenamedCoeffFVDiffusion::RenamedCoeffFVDiffusion(const InputParameters & params)
  : FVDiffusion(params)
{
}
