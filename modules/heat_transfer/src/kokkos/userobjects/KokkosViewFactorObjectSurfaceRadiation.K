//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosViewFactorObjectSurfaceRadiation.h"
#include "ViewFactorBase.h"

registerKokkosUserObject("HeatTransferApp", KokkosViewFactorObjectSurfaceRadiation);
registerVirtualKokkosUserObject(KokkosViewFactorObjectSurfaceRadiation,
                                KokkosGrayLambertSurfaceRadiationBase);

InputParameters
KokkosViewFactorObjectSurfaceRadiation::validParams()
{
  InputParameters params = KokkosGrayLambertSurfaceRadiationBase::validParams();
  params.addRequiredParam<UserObjectName>("view_factor_object_name",
                                          "Name of the ViewFactor userobjects.");
  params.addClassDescription(
      "ViewFactorObjectSurfaceRadiation computes radiative heat transfer between side sets and the "
      "view factors are computed by a ViewFactor object");
  return params;
}

KokkosViewFactorObjectSurfaceRadiation::KokkosViewFactorObjectSurfaceRadiation(
    const InputParameters & parameters)
  : KokkosGrayLambertSurfaceRadiationBase(parameters)
{
}

std::vector<std::vector<Real>>
KokkosViewFactorObjectSurfaceRadiation::setViewFactors()
{
  const ViewFactorBase & view_factor_uo = getUserObject<ViewFactorBase>("view_factor_object_name");
  const auto & boundary_names = getParam<std::vector<BoundaryName>>("boundary");
  std::vector<std::vector<Real>> vf(_n_sides, std::vector<Real>(_n_sides));

  for (const auto i : make_range(_n_sides))
    for (const auto j : make_range(_n_sides))
      vf[i][j] = view_factor_uo.getViewFactor(boundary_names[i], boundary_names[j]);

  return vf;
}
