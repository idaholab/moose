//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnBase.h"
#include "THMIndicesVACE.h"

InputParameters
ADNumericalFlux3EqnBase::validParams()
{
  InputParameters params = NumericalFlux1D::validParams();
  return params;
}

ADNumericalFlux3EqnBase::ADNumericalFlux3EqnBase(const InputParameters & parameters)
  : NumericalFlux1D(parameters)
{
}

std::vector<ADReal>
ADNumericalFlux3EqnBase::convert1DInputTo3D(const std::vector<ADReal> & U_1d) const
{
  std::vector<ADReal> U_3d(THMVACE3D::N_FLUX_INPUTS);
  U_3d[THMVACE3D::RHOA] = U_1d[THMVACE1D::RHOA];
  U_3d[THMVACE3D::RHOUA] = U_1d[THMVACE1D::RHOUA];
  U_3d[THMVACE3D::RHOVA] = 0;
  U_3d[THMVACE3D::RHOWA] = 0;
  U_3d[THMVACE3D::RHOEA] = U_1d[THMVACE1D::RHOEA];
  U_3d[THMVACE3D::AREA] = U_1d[THMVACE1D::AREA];

  return U_3d;
}

std::vector<ADReal>
ADNumericalFlux3EqnBase::convert3DFluxTo1D(const std::vector<ADReal> & F_3d) const
{
  std::vector<ADReal> F_1d(THMVACE1D::N_FLUX_OUTPUTS);
  F_1d[THMVACE1D::MASS] = F_3d[THMVACE3D::MASS];
  F_1d[THMVACE1D::MOMENTUM] = F_3d[THMVACE3D::MOM_NORM];
  F_1d[THMVACE1D::ENERGY] = F_3d[THMVACE3D::ENERGY];

  return F_1d;
}

void
ADNumericalFlux3EqnBase::transform3DFluxDirection(std::vector<ADReal> & F_3d,
                                                  const ADReal & nLR_dot_d) const
{
  F_3d[THMVACE3D::MASS] *= nLR_dot_d;
  F_3d[THMVACE3D::ENERGY] *= nLR_dot_d;
}
