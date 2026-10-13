//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGrayLambertSurfaceRadiationPP.h"

registerMooseObject("HeatTransferApp", KokkosGrayLambertSurfaceRadiationPP);

InputParameters
KokkosGrayLambertSurfaceRadiationPP::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum return_type("RADIOSITY HEAT_FLUX_DENSITY TEMPERATURE", "HEAT_FLUX_DENSITY");
  params.addParam<MooseEnum>("return_type",
                             return_type,
                             "Requested return type: RADIOSITY | HEAT_FLUX_DENSITY | TEMPERATURE");
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the KokkosGrayLambertSurfaceRadiationBase UO");
  params.addRequiredParam<BoundaryName>("boundary", "The boundary of interest.");
  params.addClassDescription(
      "This postprocessor allows to extract radiosity, heat flux density, and temperature from the "
      "KokkosGrayLambertSurfaceRadiationBase object.");
  return params;
}

KokkosGrayLambertSurfaceRadiationPP::KokkosGrayLambertSurfaceRadiationPP(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _glsr_uo(getVirtualKokkosUserObject<KokkosGrayLambertSurfaceRadiationBase>(
        "surface_radiation_object_name")),
    _return_type(getParam<MooseEnum>("return_type")),
    _bnd_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("boundary"))),
    _value(0)
{
  if (!getUserObject<KokkosGrayLambertSurfaceRadiationBase>("surface_radiation_object_name")
           .hasSurface(_bnd_id))
    paramError("boundary", "The boundary is not a boundary of the surface radiation user object.");

  _buffer.create(1);
}

void
KokkosGrayLambertSurfaceRadiationPP::finalize()
{
  extractValue();
}

void
KokkosGrayLambertSurfaceRadiationPP::extractValue()
{
  // The user object data are only accessible in the Kokkos execution space
  const auto glsr_uo = _glsr_uo;
  const auto buffer = _buffer;
  const auto id = _bnd_id;
  const unsigned int type = _return_type;

  ::Kokkos::parallel_for(
      ::Kokkos::RangePolicy<ExecSpace>(0, 1), KOKKOS_LAMBDA(const unsigned int) {
        if (type == 0)
          buffer[0] = glsr_uo.getSurfaceRadiosity(id);
        else if (type == 2)
          buffer[0] = glsr_uo.getSurfaceTemperature(id);
        else
          buffer[0] = glsr_uo.getSurfaceHeatFluxDensity(id);
      });
  ::Kokkos::fence();

  _buffer.copyToHost();
  _value = _buffer[0];
}
