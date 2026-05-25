//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVGradientMethod.h"

namespace
{
Moose::FV::GradientLimiterType
selectGradientLimiter(const std::string & limiter_name)
{
  if (limiter_name == "none")
    return Moose::FV::GradientLimiterType::None;
  if (limiter_name == "venkatakrishnan")
    return Moose::FV::GradientLimiterType::Venkatakrishnan;

  mooseError("Linear FV gradient limiter '", limiter_name, "' is not currently supported.");
}
}

InputParameters
FVGradientMethod::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.registerBase("FVGradientMethod");
  params.registerSystemAttributeName("FVGradientMethod");
  params.addClassDescription("Base class for defining cell-centered gradient methods used by "
                             "linear finite volume objects.");
  params.addParam<MooseEnum>("limiter",
                             MooseEnum("none venkatakrishnan", "none"),
                             "Limiter to apply to gradients produced by this method.");
  return params;
}

FVGradientMethod::FVGradientMethod(const InputParameters & params)
  : MooseObject(params), _limiter_type(selectGradientLimiter(getParam<MooseEnum>("limiter")))
{
}
