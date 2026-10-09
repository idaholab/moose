//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGrayLambertNeumannBC.h"

registerKokkosResidualObject("HeatTransferApp", KokkosGrayLambertNeumannBC);

InputParameters
KokkosGrayLambertNeumannBC::validParams()
{
  InputParameters params = IntegratedBCValue::validParams();
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the KokkosGrayLambertSurfaceRadiationBase UO");
  params.addParam<bool>(
      "reconstruct_emission",
      true,
      "Flag to apply constant heat flux on sideset or reconstruct emission by T^4 law.");
  // The Stefan-Boltzmann constant used by GrayLambertNeumannBC in units of [W / m^2 K^4]
  params.addParam<Real>("stefan_boltzmann_constant", 5.670367e-8, "The Stefan-Boltzmann constant.");
  params.addClassDescription("This BC imposes a heat flux density that is computed from the "
                             "KokkosGrayLambertSurfaceRadiationBase userobject.");
  return params;
}

KokkosGrayLambertNeumannBC::KokkosGrayLambertNeumannBC(const InputParameters & parameters)
  : IntegratedBCValue(parameters),
    _sigma_stefan_boltzmann(getParam<Real>("stefan_boltzmann_constant")),
    _glsr_uo(getVirtualKokkosUserObject<KokkosGrayLambertSurfaceRadiationBase>(
        "surface_radiation_object_name")),
    _reconstruct_emission(getParam<bool>("reconstruct_emission"))
{
  const auto & uo =
      getUserObject<KokkosGrayLambertSurfaceRadiationBase>("surface_radiation_object_name");

  std::vector<BoundaryID> ids(boundaryIDs().begin(), boundaryIDs().end());
  for (const auto id : ids)
    if (!uo.hasSurface(id))
      paramError("boundary",
                 "Boundary '",
                 _mesh.getBoundaryName(id),
                 "' is not a boundary of the surface radiation user object.");

  _boundary_ids = ids;
}
