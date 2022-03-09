//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "EmptyReporter.h"
#include "MooseUtils.h"

registerMooseObject("PorousFlowTestApp", EmptyReporter);

InputParameters
EmptyReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "This is a dummy reporter containing data needed for the"
      " PorousFlowLineGeometry but is empty to mimic a transfer"
      " not taking place or the reporter not being properly filled"
      " by another reporter");
  return params;
}

EmptyReporter::EmptyReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _coord_x(declareValueByName<std::vector<Real>>("x", REPORTER_MODE_REPLICATED)),
    _coord_y(declareValueByName<std::vector<Real>>("y", REPORTER_MODE_REPLICATED)),
    _coord_z(declareValueByName<std::vector<Real>>("z", REPORTER_MODE_REPLICATED)),
    _weight(declareValueByName<std::vector<Real>>("w", REPORTER_MODE_REPLICATED))
{
}
