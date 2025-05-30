//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryFluxGasMixGhostBase.h"
#include "NumericalFluxGasMixBase.h"

InputParameters
BoundaryFluxGasMixGhostBase::validParams()
{
  InputParameters params = ADBoundaryFluxBase::validParams();

  params.addRequiredParam<UserObjectName>("numerical_flux", "Name of numerical flux user object");
  params.addRequiredParam<Real>("normal", "Outward normal");

  return params;
}

BoundaryFluxGasMixGhostBase::BoundaryFluxGasMixGhostBase(const InputParameters & parameters)
  : ADBoundaryFluxBase(parameters),
    _numerical_flux(getUserObject<NumericalFluxGasMixBase>("numerical_flux")),
    _normal(getParam<Real>("normal"))
{
}

void
BoundaryFluxGasMixGhostBase::calcFlux(unsigned int iside,
                                      dof_id_type ielem,
                                      const std::vector<ADReal> & U1,
                                      const RealVectorValue & /*normal*/,
                                      std::vector<ADReal> & flux) const
{
  const std::vector<ADReal> U2 = getGhostCellSolution(U1);
  flux = _numerical_flux.getFlux(iside, ielem, true, U1, U2, _normal);
}
