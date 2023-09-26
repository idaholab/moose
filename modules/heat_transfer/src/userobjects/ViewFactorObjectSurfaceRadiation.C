//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewFactorObjectSurfaceRadiation.h"
#include "ViewFactorBase.h"

registerMooseObject("HeatConductionApp", ViewFactorObjectSurfaceRadiation);

InputParameters
ViewFactorObjectSurfaceRadiation::validParams()
{
  InputParameters params = GrayLambertSurfaceRadiationBase::validParams();
  params.addRequiredParam<UserObjectName>("view_factor_object_name",
                                          "Name of the ViewFactor userobjects.");
  params.addClassDescription(
      "ViewFactorObjectSurfaceRadiation computes radiative heat transfer between side sets and the "
      "view factors are computed by a ViewFactor object");
  return params;
}

ViewFactorObjectSurfaceRadiation::ViewFactorObjectSurfaceRadiation(
    const InputParameters & parameters)
  : GrayLambertSurfaceRadiationBase(parameters)
{
}

std::vector<std::vector<Real>>
ViewFactorObjectSurfaceRadiation::setViewFactors()
{
  const ViewFactorBase & view_factor_uo = getUserObject<ViewFactorBase>("view_factor_object_name");
  std::vector<BoundaryName> boundary_names = getParam<std::vector<BoundaryName>>("boundary");
  std::vector<std::vector<Real>> vf(_n_sides);

  for (unsigned int i = 0; i < _n_sides; ++i)
  {
    vf[i].resize(_n_sides);

    for (unsigned int j = 0; j < _n_sides; ++j)
      vf[i][j] = view_factor_uo.getViewFactor(boundary_names[i], boundary_names[j]);
  }
  return vf;
}
