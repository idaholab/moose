//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosInfiniteCylinderRadiativeBC.h"

registerKokkosResidualObject("HeatTransferApp", KokkosInfiniteCylinderRadiativeBC);

InputParameters
KokkosInfiniteCylinderRadiativeBC::validParams()
{
  InputParameters params = KokkosRadiativeHeatFluxBCBase::validParams();
  params.addRequiredParam<Real>("boundary_emissivity", "Emissivity of the boundary.");
  params.addParam<Real>("cylinder_emissivity",
                        1,
                        "Emissivity of the cylinder in radiative heat transfer with the boundary.");
  params.addRequiredParam<Real>("boundary_radius",
                                "Radius of the boundary approximated as cylinder.");
  params.addRequiredParam<Real>("cylinder_radius",
                                "Radius of the cylinder on the outside of the boundary.");
  params.addClassDescription("Boundary condition for radiative heat exchange with a cylinder"
                             "where the boundary is approximated as a cylinder as well.");
  return params;
}

namespace
{
Real
effectiveEmissivity(const InputParameters & params)
{
  const auto eps_boundary = params.get<Real>("boundary_emissivity");
  const auto eps_cylinder = params.get<Real>("cylinder_emissivity");
  const auto boundary_radius = params.get<Real>("boundary_radius");
  const auto cylinder_radius = params.get<Real>("cylinder_radius");

  return eps_boundary * eps_cylinder * cylinder_radius /
         (eps_cylinder * cylinder_radius + eps_boundary * boundary_radius * (1 - eps_cylinder));
}
}

KokkosInfiniteCylinderRadiativeBC::KokkosInfiniteCylinderRadiativeBC(
    const InputParameters & parameters)
  : KokkosRadiativeHeatFluxBCBase(parameters), _coefficient(effectiveEmissivity(parameters))
{
}
