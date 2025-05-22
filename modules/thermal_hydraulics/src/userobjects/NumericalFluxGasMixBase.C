//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumericalFluxGasMixBase.h"
#include "THMIndicesGasMix.h"

InputParameters
NumericalFluxGasMixBase::validParams()
{
  InputParameters params = NumericalFlux1D::validParams();
  return params;
}

NumericalFluxGasMixBase::NumericalFluxGasMixBase(const InputParameters & parameters)
  : NumericalFlux1D(parameters)
{
}

std::vector<ADReal>
NumericalFluxGasMixBase::convert1DInputTo3D(const std::vector<ADReal> & U_1d) const
{
  std::vector<ADReal> U_3d(THMGasMix3D::N_FLUX_INPUTS);
  U_3d[THMGasMix3D::XIRHOA] = U_1d[THMGasMix1D::XIRHOA];
  U_3d[THMGasMix3D::RHOA] = U_1d[THMGasMix1D::RHOA];
  U_3d[THMGasMix3D::RHOUA] = U_1d[THMGasMix1D::RHOUA];
  U_3d[THMGasMix3D::RHOVA] = 0;
  U_3d[THMGasMix3D::RHOWA] = 0;
  U_3d[THMGasMix3D::RHOEA] = U_1d[THMGasMix1D::RHOEA];
  U_3d[THMGasMix3D::AREA] = U_1d[THMGasMix1D::AREA];

  return U_3d;
}

std::vector<ADReal>
NumericalFluxGasMixBase::convert3DFluxTo1D(const std::vector<ADReal> & F_3d) const
{
  std::vector<ADReal> F_1d(THMGasMix1D::N_FLUX_OUTPUTS);
  F_1d[THMGasMix1D::SPECIES] = F_3d[THMGasMix3D::SPECIES];
  F_1d[THMGasMix1D::MASS] = F_3d[THMGasMix3D::MASS];
  F_1d[THMGasMix1D::MOMENTUM] = F_3d[THMGasMix3D::MOM_NORM];
  F_1d[THMGasMix1D::ENERGY] = F_3d[THMGasMix3D::ENERGY];

  return F_1d;
}

void
NumericalFluxGasMixBase::transform3DFluxDirection(std::vector<ADReal> & F_3d, Real nLR_dot_d) const
{
  F_3d[THMGasMix3D::SPECIES] *= nLR_dot_d;
  F_3d[THMGasMix3D::MASS] *= nLR_dot_d;
  F_3d[THMGasMix3D::ENERGY] *= nLR_dot_d;
}
