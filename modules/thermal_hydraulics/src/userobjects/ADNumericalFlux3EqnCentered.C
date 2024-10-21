//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADNumericalFlux3EqnCentered.h"
#include "THMIndicesVACE.h"
#include "Numerics.h"

registerMooseObject("ThermalHydraulicsApp", ADNumericalFlux3EqnCentered);

InputParameters
ADNumericalFlux3EqnCentered::validParams()
{
  InputParameters params = ADNumericalFlux3EqnBase::validParams();

  params.addClassDescription(
      "Computes internal side flux for the 1-D, 1-phase, variable-area Euler equations using a "
      "centered average of the left and right side fluxes");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

ADNumericalFlux3EqnCentered::ADNumericalFlux3EqnCentered(const InputParameters & parameters)
  : ADNumericalFlux3EqnBase(parameters),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

void
ADNumericalFlux3EqnCentered::calcFlux(const std::vector<ADReal> & U1,
                                      const std::vector<ADReal> & U2,
                                      const RealVectorValue & nLR,
                                      const RealVectorValue & t1,
                                      const RealVectorValue & t2,
                                      std::vector<ADReal> & FL,
                                      std::vector<ADReal> & FR) const
{
  const std::vector<ADReal> flux1 = computeFlux(U1, nLR, t1, t2);
  const std::vector<ADReal> flux2 = computeFlux(U2, nLR, t1, t2);

  FL.resize(THMVACE3D::N_FLUX_OUTPUTS);
  for (unsigned int i = 0; i < THMVACE3D::N_FLUX_OUTPUTS; i++)
    FL[i] = 0.5 * (flux1[i] + flux2[i]);

  FR = FL;
}

std::vector<ADReal>
ADNumericalFlux3EqnCentered::computeFlux(const std::vector<ADReal> & U,
                                         const RealVectorValue & n,
                                         const RealVectorValue & t1,
                                         const RealVectorValue & t2) const
{
  const ADReal rhoA = U[THMVACE3D::RHOA];
  const ADReal rhouA = U[THMVACE3D::RHOUA];
  const ADReal rhovA = U[THMVACE3D::RHOVA];
  const ADReal rhowA = U[THMVACE3D::RHOWA];
  const ADReal rhoEA = U[THMVACE3D::RHOEA];
  const ADReal A = U[THMVACE3D::AREA];

  const ADReal rho = rhoA / A;
  const ADRealVectorValue uvec(rhouA / rhoA, rhovA / rhoA, rhowA / rhoA);
  const ADReal un = uvec * n;
  const ADReal ut1 = uvec * t1;
  const ADReal ut2 = uvec * t2;
  const ADReal v = 1.0 / rho;
  const ADReal E = rhoEA / rhoA;
  const ADReal e = E - 0.5 * uvec * uvec;
  const ADReal p = _fp.p_from_v_e(v, e);
  const ADReal H = E + p / rho;

  std::vector<ADReal> flux(THMVACE3D::N_FLUX_OUTPUTS, 0.0);
  flux[THMVACE3D::MASS] = rho * un * A;
  flux[THMVACE3D::MOM_NORM] = (rho * un * un + p) * A;
  flux[THMVACE3D::MOM_TAN1] = rho * un * ut1 * A;
  flux[THMVACE3D::MOM_TAN2] = rho * un * ut2 * A;
  flux[THMVACE3D::ENERGY] = rho * un * H * A;

  return flux;
}
