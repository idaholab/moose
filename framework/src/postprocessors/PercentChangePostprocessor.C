//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PercentChangePostprocessor.h"

registerMooseObject("MooseApp", PercentChangePostprocessor);

InputParameters
PercentChangePostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Computes the percent change of a postprocessor value compared to the "
                             "value at the previous timestep.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The name of the postprocessor used for exit criterion");
  return params;
}

PercentChangePostprocessor::PercentChangePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _postprocessor_old(getPostprocessorValueOld("postprocessor"))
{
  mooseDeprecated("PercentChangePostprocessor is deprecated: instead, ",
                  "please use ChangeOverTimePostprocessor using the parameter ",
                  "'compute_relative_change' set to 'true'");
}

void
PercentChangePostprocessor::initialize()
{
}

void
PercentChangePostprocessor::execute()
{
}

Real
PercentChangePostprocessor::getValue() const
{
  return std::fabs((std::fabs(_postprocessor) - std::fabs(_postprocessor_old)) *
                   std::pow(std::fabs(_postprocessor), -1));
}
