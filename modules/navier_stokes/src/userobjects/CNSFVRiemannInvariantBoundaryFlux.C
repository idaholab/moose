/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVRiemannInvariantBoundaryFlux.h"

template <>
InputParameters
validParams<CNSFVRiemannInvariantBoundaryFlux>()
{
  InputParameters params = validParams<BoundaryFluxBase>();

  params.addClassDescription("A user objec that computes the Riemann-invariant boundary flux.");

  params.addRequiredParam<UserObjectName>("bc_uo", "Name for boundary condition user object");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  return params;
}

CNSFVRiemannInvariantBoundaryFlux::CNSFVRiemannInvariantBoundaryFlux(
    const InputParameters & parameters)
  : BoundaryFluxBase(parameters),
    _bc_uo(getUserObject<BCUserObject>("bc_uo")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

CNSFVRiemannInvariantBoundaryFlux::~CNSFVRiemannInvariantBoundaryFlux() {}

void
CNSFVRiemannInvariantBoundaryFlux::calcFlux(unsigned int iside,
                                            dof_id_type ielem,
                                            const std::vector<Real> & uvec1,
                                            const RealVectorValue & dwave,
                                            std::vector<Real> & flux) const
{
  /// pass the inputs to local

  Real rho1 = uvec1[0];
  Real rhou1 = uvec1[1];
  Real rhov1 = uvec1[2];
  Real rhow1 = uvec1[3];
  Real rhoe1 = uvec1[4];

  Real nx = dwave(0);
  Real ny = dwave(1);
  Real nz = dwave(2);

  /// assign the proper size for the flux vector

  flux.resize(5);

  /// compute the local Mach number with state variables on the left

  Real rhom1 = 1. / rho1;
  Real uadv1 = rhou1 * rhom1;
  Real vadv1 = rhov1 * rhom1;
  Real wadv1 = rhow1 * rhom1;
  Real vdov1 = uadv1 * uadv1 + vadv1 * vadv1 + wadv1 * wadv1;
  Real eint1 = rhoe1 * rhom1 - 0.5 * vdov1;
  Real pres1 = _fp.pressure(rhom1, eint1);
  Real csou1 = _fp.c(rhom1, eint1);
  Real mach1 = std::sqrt(vdov1) / csou1;

  /// calc the flux vector according to local Mach number

  if (std::abs(mach1) < 1.)
  {
    /// subsonic

    Real gamma = _fp.gamma(0., 0.);
    Real gamm1 = gamma - 1.;

    std::vector<Real> U2(5, 0.);

    U2 = _bc_uo.getGhostCellValue(iside, ielem, uvec1, dwave);

    Real rho2 = U2[0];
    Real rhou2 = U2[1];
    Real rhov2 = U2[2];
    Real rhow2 = U2[3];
    Real rhoe2 = U2[4];

    Real uadv2 = rhou2 / rho2;
    Real vadv2 = rhov2 / rho2;
    Real wadv2 = rhow2 / rho2;
    Real vdov2 = uadv2 * uadv2 + vadv2 * vadv2 + wadv2 * wadv2;
    Real v2 = 1. / rho2;
    Real e2 = rhoe2 / rho2 - 0.5 * vdov2;
    Real pres2 = _fp.pressure(v2, e2);

    Real eint2 = _fp.e(pres2, rho2);
    Real csou2 = _fp.c(1. / rho2, eint2);

    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;
    Real vdon2 = uadv2 * nx + vadv2 * ny + wadv2 * nz;

    Real rplus = vdon1 + 2. * csou1 / gamm1;
    Real rmins = vdon2 - 2. * csou2 / gamm1;

    Real velob = (rplus + rmins) * 0.5;
    Real csoub = (rplus - rmins) * 0.25 * gamm1;

    Real vdiff, uadvb, vadvb, wadvb, entrb;

    if (mach1 <= 0.)
    {
      /// subsonic inflow

      vdiff = velob - vdon2;
      uadvb = uadv2 + vdiff * nx;
      vadvb = vadv2 + vdiff * ny;
      wadvb = wadv2 + vdiff * nz;
      entrb = csou2 * csou2 / gamma / std::pow(rho2, gamm1);
    }
    else
    {
      /// subsonic outflow

      vdiff = velob - vdon1;
      uadvb = uadv1 + vdiff * nx;
      vadvb = vadv1 + vdiff * ny;
      wadvb = wadv1 + vdiff * nz;
      entrb = csou1 * csou1 / gamma / std::pow(rho1, gamm1);
    }

    Real rhob = std::pow(csoub * csoub / gamma / entrb, 1. / gamm1);

    Real presb = rhob * csoub * csoub / gamma;

    Real vdonb = uadvb * nx + vadvb * ny + wadvb * nz;

    Real vdovb = uadvb * uadvb + vadvb * vadvb + wadvb * wadvb;

    Real rhoeb = rhob * _fp.e(presb, rhob) + 0.5 * rhob * vdovb;

    flux[0] = vdonb * rhob;
    flux[1] = vdonb * rhob * uadvb + presb * nx;
    flux[2] = vdonb * rhob * vadvb + presb * ny;
    flux[3] = vdonb * rhob * wadvb + presb * nz;
    flux[4] = vdonb * (rhoeb + presb);
  }
  else if (mach1 <= -1.)
  {
    /// supersonic inflow

    std::vector<Real> U2(5, 0.);

    U2 = _bc_uo.getGhostCellValue(iside, ielem, uvec1, dwave);

    Real rho2 = U2[0];
    Real rhou2 = U2[1];
    Real rhov2 = U2[2];
    Real rhow2 = U2[3];
    Real rhoe2 = U2[4];

    Real uadv2 = rhou2 / rho2;
    Real vadv2 = rhov2 / rho2;
    Real wadv2 = rhow2 / rho2;
    Real vdov2 = uadv2 * uadv2 + vadv2 * vadv2 + wadv2 * wadv2;
    Real v2 = 1. / rho2;
    Real e2 = rhoe2 / rho2 - 0.5 * vdov2;
    Real pres2 = _fp.pressure(v2, e2);

    Real vdon2 = uadv2 * nx + vadv2 * ny + wadv2 * nz;

    flux[0] = vdon2 * rho2;
    flux[1] = vdon2 * rho2 * uadv2 + pres2 * nx;
    flux[2] = vdon2 * rho2 * vadv2 + pres2 * ny;
    flux[3] = vdon2 * rho2 * wadv2 + pres2 * nz;
    flux[4] = vdon2 * (rhoe2 + pres2);
  }
  else if (mach1 >= 1.)
  {
    /// supersonic outflow

    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;

    flux[0] = vdon1 * rho1;
    flux[1] = vdon1 * rhou1 + pres1 * nx;
    flux[2] = vdon1 * rhov1 + pres1 * ny;
    flux[3] = vdon1 * rhow1 + pres1 * nz;
    flux[4] = vdon1 * (rhoe1 + pres1);
  }
  else
    mooseError("Something is wrong in ",
               name(),
               ": ",
               __FUNCTION__,
               "\n",
               "ielem = ",
               ielem,
               "\n",
               "iside = ",
               iside,
               "\n",
               "mach1 = ",
               mach1,
               "\n");
}

void
CNSFVRiemannInvariantBoundaryFlux::calcJacobian(unsigned int iside,
                                                dof_id_type ielem,
                                                const std::vector<Real> & uvec1,
                                                const RealVectorValue & dwave,
                                                DenseMatrix<Real> & jac1) const
{
  /// pass the inputs to local

  Real rho1 = uvec1[0];
  Real rhou1 = uvec1[1];
  Real rhov1 = uvec1[2];
  Real rhow1 = uvec1[3];
  Real rhoe1 = uvec1[4];

  Real nx = dwave(0);
  Real ny = dwave(1);
  Real nz = dwave(2);

  /// assign the proper size for the Jacobian matrix

  jac1.resize(5, 5);

  /// compute the local Mach number with state variables on the left

  Real rhom1 = 1. / rho1;
  Real uadv1 = rhou1 * rhom1;
  Real vadv1 = rhov1 * rhom1;
  Real wadv1 = rhow1 * rhom1;
  Real vdov1 = uadv1 * uadv1 + vadv1 * vadv1 + wadv1 * wadv1;
  Real eint1 = rhoe1 * rhom1 - 0.5 * vdov1;
  Real pres1 = _fp.pressure(rhom1, eint1);
  Real csou1 = _fp.c(rhom1, eint1);
  Real mach1 = std::sqrt(vdov1) / csou1;

  /// calc the flux Jacobian matrix according to local Mach number

  if (std::abs(mach1) < 1.)
  {
    /// subsonic

    Real gamma = _fp.gamma(0., 0.);
    Real gamm1 = gamma - 1.;

    std::vector<Real> U2(5, 0.);

    U2 = _bc_uo.getGhostCellValue(iside, ielem, uvec1, dwave);

    Real rho2 = U2[0];
    Real rhou2 = U2[1];
    Real rhov2 = U2[2];
    Real rhow2 = U2[3];
    Real rhoe2 = U2[4];

    Real uadv2 = rhou2 / rho2;
    Real vadv2 = rhov2 / rho2;
    Real wadv2 = rhow2 / rho2;
    Real vdov2 = uadv2 * uadv2 + vadv2 * vadv2 + wadv2 * wadv2;
    Real v2 = 1. / rho2;
    Real e2 = rhoe2 / rho2 - 0.5 * vdov2;
    Real pres2 = _fp.pressure(v2, e2);

    Real eint2 = _fp.e(pres2, rho2);
    Real csou2 = _fp.c(1. / rho2, eint2);

    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;
    Real vdon2 = uadv2 * nx + vadv2 * ny + wadv2 * nz;

    Real rplus = vdon1 + 2. * csou1 / gamm1;
    Real rmins = vdon2 - 2. * csou2 / gamm1;

    Real velob = (rplus + rmins) * 0.5;
    Real csoub = (rplus - rmins) * 0.25 * gamm1;

    Real vdiff, uadvb, vadvb, wadvb, entrb;

    if (mach1 <= 0.)
    {
      /// subsonic inflow

      vdiff = velob - vdon2;
      uadvb = uadv2 + vdiff * nx;
      vadvb = vadv2 + vdiff * ny;
      wadvb = wadv2 + vdiff * nz;
      entrb = csou2 * csou2 / gamma / std::pow(rho2, gamm1);
    }
    else
    {
      /// subsonic outflow

      vdiff = velob - vdon1;
      uadvb = uadv1 + vdiff * nx;
      vadvb = vadv1 + vdiff * ny;
      wadvb = wadv1 + vdiff * nz;
      entrb = csou1 * csou1 / gamma / std::pow(rho1, gamm1);
    }

    Real rhob = std::pow(csoub * csoub / gamma / entrb, 1. / gamm1);

    Real presb = rhob * csoub * csoub / gamma;

    Real vdonb = uadvb * nx + vadvb * ny + wadvb * nz;

    Real vdovb = uadvb * uadvb + vadvb * vadvb + wadvb * wadvb;

    Real rhoeb = rhob * _fp.e(presb, rhob) + 0.5 * rhob * vdovb;

    Real rhoub = rhob * uadvb;
    Real rhovb = rhob * vadvb;
    Real rhowb = rhob * wadvb;

    Real tenthb = rhoeb + presb;

    /// calc a(F)/a(Q_b)

    Real fmass_rhob = vdonb;
    Real fmass_uadvb = nx * rhob;
    Real fmass_vadvb = ny * rhob;
    Real fmass_wadvb = nz * rhob;
    Real fmass_presb = 0.;

    Real fmomx_rhob = vdonb * uadvb;
    Real fmomx_uadvb = nx * rhoub * 2.;
    Real fmomx_vadvb = ny * rhoub;
    Real fmomx_wadvb = nz * rhoub;
    Real fmomx_presb = nx;

    Real fmomy_rhob = vdonb * vadvb;
    Real fmomy_uadvb = nx * rhovb;
    Real fmomy_vadvb = ny * rhovb * 2.;
    Real fmomy_wadvb = nz * rhovb;
    Real fmomy_presb = ny;

    Real fmomz_rhob = vdonb * wadvb;
    Real fmomz_uadvb = nx * rhowb;
    Real fmomz_vadvb = ny * rhowb;
    Real fmomz_wadvb = nz * rhowb * 2.;
    Real fmomz_presb = nz;

    Real fetot_rhob = 0.5 * vdonb * vdovb;
    Real fetot_uadvb = nx * tenthb + vdonb * rhoub;
    Real fetot_vadvb = ny * tenthb + vdonb * rhovb;
    Real fetot_wadvb = nz * tenthb + vdonb * rhowb;
    Real fetot_presb = gamma / gamm1 * vdonb;

    /// calc a(Q_1)/a(U_1)

    Real rho1_rho1 = 1.;
    Real rho1_rhou1 = 0.;
    Real rho1_rhov1 = 0.;
    Real rho1_rhow1 = 0.;
    Real rho1_rhoe1 = 0.;

    Real uadv1_rho1 = -rhom1 * uadv1;
    Real uadv1_rhou1 = rhom1;
    Real uadv1_rhov1 = 0.;
    Real uadv1_rhow1 = 0.;
    Real uadv1_rhoe1 = 0.;

    Real vadv1_rho1 = -rhom1 * vadv1;
    Real vadv1_rhou1 = 0.;
    Real vadv1_rhov1 = rhom1;
    Real vadv1_rhow1 = 0.;
    Real vadv1_rhoe1 = 0.;

    Real wadv1_rho1 = -rhom1 * wadv1;
    Real wadv1_rhou1 = 0.;
    Real wadv1_rhov1 = 0.;
    Real wadv1_rhow1 = rhom1;
    Real wadv1_rhoe1 = 0.;

    Real pres1_rho1 = gamm1 * vdov1 * 0.5;
    Real pres1_rhou1 = -gamm1 * uadv1;
    Real pres1_rhov1 = -gamm1 * vadv1;
    Real pres1_rhow1 = -gamm1 * wadv1;
    Real pres1_rhoe1 = gamm1;

    /// calc a(vdon1)/a(U_1)

    Real vdon1_rho1 = -rhom1 * vdon1;
    Real vdon1_rhou1 = rhom1 * nx;
    Real vdon1_rhov1 = rhom1 * ny;
    Real vdon1_rhow1 = rhom1 * nz;
    Real vdon1_rhoe1 = 0.;

    /// calc a(a_1)/a(U_1)

    Real cp051 = 0.5 * csou1 / pres1;
    Real prho1 = pres1 * rhom1;

    Real csou1_rho1 = cp051 * (pres1_rho1 - prho1 * rho1_rho1);
    Real csou1_rhou1 = cp051 * (pres1_rhou1 - prho1 * rho1_rhou1);
    Real csou1_rhov1 = cp051 * (pres1_rhov1 - prho1 * rho1_rhov1);
    Real csou1_rhow1 = cp051 * (pres1_rhow1 - prho1 * rho1_rhow1);
    Real csou1_rhoe1 = cp051 * (pres1_rhoe1 - prho1 * rho1_rhoe1);

    /// calc a(R^+)/a(U_1)

    Real Rplus_rho1 = vdon1_rho1 + 2. / gamm1 * csou1_rho1;
    Real Rplus_rhou1 = vdon1_rhou1 + 2. / gamm1 * csou1_rhou1;
    Real Rplus_rhov1 = vdon1_rhov1 + 2. / gamm1 * csou1_rhov1;
    Real Rplus_rhow1 = vdon1_rhow1 + 2. / gamm1 * csou1_rhow1;
    Real Rplus_rhoe1 = vdon1_rhoe1 + 2. / gamm1 * csou1_rhoe1;

    /// calc a(a_b)/a(U_1)

    Real csoub_rho1 = 0.25 * gamm1 * Rplus_rho1;
    Real csoub_rhou1 = 0.25 * gamm1 * Rplus_rhou1;
    Real csoub_rhov1 = 0.25 * gamm1 * Rplus_rhov1;
    Real csoub_rhow1 = 0.25 * gamm1 * Rplus_rhow1;
    Real csoub_rhoe1 = 0.25 * gamm1 * Rplus_rhoe1;

    /// init a(Q_b)/a(U_1)

    Real rhob_rho1, rhob_rhou1, rhob_rhov1, rhob_rhow1, rhob_rhoe1;
    Real uadvb_rho1, uadvb_rhou1, uadvb_rhov1, uadvb_rhow1, uadvb_rhoe1;
    Real vadvb_rho1, vadvb_rhou1, vadvb_rhov1, vadvb_rhow1, vadvb_rhoe1;
    Real wadvb_rho1, wadvb_rhou1, wadvb_rhov1, wadvb_rhow1, wadvb_rhoe1;
    Real presb_rho1, presb_rhou1, presb_rhov1, presb_rhow1, presb_rhoe1;

    if (mach1 <= 0.)
    {
      /// subsonic inflow

      /// calc a(Q_b)/a(U_1)

      Real coeff = rhob / gamm1 * 2. / csoub;

      rhob_rho1 = coeff * csoub_rho1;
      rhob_rhou1 = coeff * csoub_rhou1;
      rhob_rhov1 = coeff * csoub_rhov1;
      rhob_rhow1 = coeff * csoub_rhow1;
      rhob_rhoe1 = coeff * csoub_rhoe1;

      uadvb_rho1 = nx * 0.5 * Rplus_rho1;
      uadvb_rhou1 = nx * 0.5 * Rplus_rhou1;
      uadvb_rhov1 = nx * 0.5 * Rplus_rhov1;
      uadvb_rhow1 = nx * 0.5 * Rplus_rhow1;
      uadvb_rhoe1 = nx * 0.5 * Rplus_rhoe1;

      vadvb_rho1 = ny * 0.5 * Rplus_rho1;
      vadvb_rhou1 = ny * 0.5 * Rplus_rhou1;
      vadvb_rhov1 = ny * 0.5 * Rplus_rhov1;
      vadvb_rhow1 = ny * 0.5 * Rplus_rhow1;
      vadvb_rhoe1 = ny * 0.5 * Rplus_rhoe1;

      wadvb_rho1 = nz * 0.5 * Rplus_rho1;
      wadvb_rhou1 = nz * 0.5 * Rplus_rhou1;
      wadvb_rhov1 = nz * 0.5 * Rplus_rhov1;
      wadvb_rhow1 = nz * 0.5 * Rplus_rhow1;
      wadvb_rhoe1 = nz * 0.5 * Rplus_rhoe1;

      presb_rho1 = presb * (1. / rhob * rhob_rho1 + 2. / csoub * csoub_rho1);
      presb_rhou1 = presb * (1. / rhob * rhob_rhou1 + 2. / csoub * csoub_rhou1);
      presb_rhov1 = presb * (1. / rhob * rhob_rhov1 + 2. / csoub * csoub_rhov1);
      presb_rhow1 = presb * (1. / rhob * rhob_rhow1 + 2. / csoub * csoub_rhow1);
      presb_rhoe1 = presb * (1. / rhob * rhob_rhoe1 + 2. / csoub * csoub_rhoe1);
    }
    else
    {
      /// subsonic outflow

      /// calc a(s_b)/a(U_1)

      Real coeff = csou1 / gamma / std::pow(rho1, gamma);

      Real entrb_rho1 = coeff * (2. * rho1 * csou1_rho1 - gamm1 * csou1 * rho1_rho1);
      Real entrb_rhou1 = coeff * (2. * rho1 * csou1_rhou1 - gamm1 * csou1 * rho1_rhou1);
      Real entrb_rhov1 = coeff * (2. * rho1 * csou1_rhov1 - gamm1 * csou1 * rho1_rhov1);
      Real entrb_rhow1 = coeff * (2. * rho1 * csou1_rhow1 - gamm1 * csou1 * rho1_rhow1);
      Real entrb_rhoe1 = coeff * (2. * rho1 * csou1_rhoe1 - gamm1 * csou1 * rho1_rhoe1);

      /// calc a(Q_b)/a(U_1)

      rhob_rho1 = rhob / gamm1 * (2. / csoub * csoub_rho1 - 1. / entrb * entrb_rho1);
      rhob_rhou1 = rhob / gamm1 * (2. / csoub * csoub_rhou1 - 1. / entrb * entrb_rhou1);
      rhob_rhov1 = rhob / gamm1 * (2. / csoub * csoub_rhov1 - 1. / entrb * entrb_rhov1);
      rhob_rhow1 = rhob / gamm1 * (2. / csoub * csoub_rhow1 - 1. / entrb * entrb_rhow1);
      rhob_rhoe1 = rhob / gamm1 * (2. / csoub * csoub_rhoe1 - 1. / entrb * entrb_rhoe1);

      uadvb_rho1 = nx * (0.5 * Rplus_rho1 - vdon1_rho1) + uadv1_rho1;
      uadvb_rhou1 = nx * (0.5 * Rplus_rhou1 - vdon1_rhou1) + uadv1_rhou1;
      uadvb_rhov1 = nx * (0.5 * Rplus_rhov1 - vdon1_rhov1) + uadv1_rhov1;
      uadvb_rhow1 = nx * (0.5 * Rplus_rhow1 - vdon1_rhow1) + uadv1_rhow1;
      uadvb_rhoe1 = nx * (0.5 * Rplus_rhoe1 - vdon1_rhoe1) + uadv1_rhoe1;

      vadvb_rho1 = ny * (0.5 * Rplus_rho1 - vdon1_rho1) + vadv1_rho1;
      vadvb_rhou1 = ny * (0.5 * Rplus_rhou1 - vdon1_rhou1) + vadv1_rhou1;
      vadvb_rhov1 = ny * (0.5 * Rplus_rhov1 - vdon1_rhov1) + vadv1_rhov1;
      vadvb_rhow1 = ny * (0.5 * Rplus_rhow1 - vdon1_rhow1) + vadv1_rhow1;
      vadvb_rhoe1 = ny * (0.5 * Rplus_rhoe1 - vdon1_rhoe1) + vadv1_rhoe1;

      wadvb_rho1 = nz * (0.5 * Rplus_rho1 - vdon1_rho1) + wadv1_rho1;
      wadvb_rhou1 = nz * (0.5 * Rplus_rhou1 - vdon1_rhou1) + wadv1_rhou1;
      wadvb_rhov1 = nz * (0.5 * Rplus_rhov1 - vdon1_rhov1) + wadv1_rhov1;
      wadvb_rhow1 = nz * (0.5 * Rplus_rhow1 - vdon1_rhow1) + wadv1_rhow1;
      wadvb_rhoe1 = nz * (0.5 * Rplus_rhoe1 - vdon1_rhoe1) + wadv1_rhoe1;

      presb_rho1 = presb * (1. / rhob * rhob_rho1 + 2. / csoub * csoub_rho1);
      presb_rhou1 = presb * (1. / rhob * rhob_rhou1 + 2. / csoub * csoub_rhou1);
      presb_rhov1 = presb * (1. / rhob * rhob_rhov1 + 2. / csoub * csoub_rhov1);
      presb_rhow1 = presb * (1. / rhob * rhob_rhow1 + 2. / csoub * csoub_rhow1);
      presb_rhoe1 = presb * (1. / rhob * rhob_rhoe1 + 2. / csoub * csoub_rhoe1);
    }

    /// calc a(F_mass)/a(U_1)

    jac1(0, 0) = fmass_rhob * rhob_rho1 + fmass_uadvb * uadvb_rho1 + fmass_vadvb * vadvb_rho1 +
                 fmass_wadvb * wadvb_rho1 + fmass_presb * presb_rho1;

    jac1(0, 1) = fmass_rhob * rhob_rhou1 + fmass_uadvb * uadvb_rhou1 + fmass_vadvb * vadvb_rhou1 +
                 fmass_wadvb * wadvb_rhou1 + fmass_presb * presb_rhou1;

    jac1(0, 2) = fmass_rhob * rhob_rhov1 + fmass_uadvb * uadvb_rhov1 + fmass_vadvb * vadvb_rhov1 +
                 fmass_wadvb * wadvb_rhov1 + fmass_presb * presb_rhov1;

    jac1(0, 3) = fmass_rhob * rhob_rhow1 + fmass_uadvb * uadvb_rhow1 + fmass_vadvb * vadvb_rhow1 +
                 fmass_wadvb * wadvb_rhow1 + fmass_presb * presb_rhow1;

    jac1(0, 4) = fmass_rhob * rhob_rhoe1 + fmass_uadvb * uadvb_rhoe1 + fmass_vadvb * vadvb_rhoe1 +
                 fmass_wadvb * wadvb_rhoe1 + fmass_presb * presb_rhoe1;

    /// calc a(F_momx)/a(U_1)

    jac1(1, 0) = fmomx_rhob * rhob_rho1 + fmomx_uadvb * uadvb_rho1 + fmomx_vadvb * vadvb_rho1 +
                 fmomx_wadvb * wadvb_rho1 + fmomx_presb * presb_rho1;

    jac1(1, 1) = fmomx_rhob * rhob_rhou1 + fmomx_uadvb * uadvb_rhou1 + fmomx_vadvb * vadvb_rhou1 +
                 fmomx_wadvb * wadvb_rhou1 + fmomx_presb * presb_rhou1;

    jac1(1, 2) = fmomx_rhob * rhob_rhov1 + fmomx_uadvb * uadvb_rhov1 + fmomx_vadvb * vadvb_rhov1 +
                 fmomx_wadvb * wadvb_rhov1 + fmomx_presb * presb_rhov1;

    jac1(1, 3) = fmomx_rhob * rhob_rhow1 + fmomx_uadvb * uadvb_rhow1 + fmomx_vadvb * vadvb_rhow1 +
                 fmomx_wadvb * wadvb_rhow1 + fmomx_presb * presb_rhow1;

    jac1(1, 4) = fmomx_rhob * rhob_rhoe1 + fmomx_uadvb * uadvb_rhoe1 + fmomx_vadvb * vadvb_rhoe1 +
                 fmomx_wadvb * wadvb_rhoe1 + fmomx_presb * presb_rhoe1;

    /// calc a(F_momy)/a(U_1)

    jac1(2, 0) = fmomy_rhob * rhob_rho1 + fmomy_uadvb * uadvb_rho1 + fmomy_vadvb * vadvb_rho1 +
                 fmomy_wadvb * wadvb_rho1 + fmomy_presb * presb_rho1;

    jac1(2, 1) = fmomy_rhob * rhob_rhou1 + fmomy_uadvb * uadvb_rhou1 + fmomy_vadvb * vadvb_rhou1 +
                 fmomy_wadvb * wadvb_rhou1 + fmomy_presb * presb_rhou1;

    jac1(2, 2) = fmomy_rhob * rhob_rhov1 + fmomy_uadvb * uadvb_rhov1 + fmomy_vadvb * vadvb_rhov1 +
                 fmomy_wadvb * wadvb_rhov1 + fmomy_presb * presb_rhov1;

    jac1(2, 3) = fmomy_rhob * rhob_rhow1 + fmomy_uadvb * uadvb_rhow1 + fmomy_vadvb * vadvb_rhow1 +
                 fmomy_wadvb * wadvb_rhow1 + fmomy_presb * presb_rhow1;

    jac1(2, 4) = fmomy_rhob * rhob_rhoe1 + fmomy_uadvb * uadvb_rhoe1 + fmomy_vadvb * vadvb_rhoe1 +
                 fmomy_wadvb * wadvb_rhoe1 + fmomy_presb * presb_rhoe1;

    /// calc a(F_momz)/a(U_1)

    jac1(3, 0) = fmomz_rhob * rhob_rho1 + fmomz_uadvb * uadvb_rho1 + fmomz_vadvb * vadvb_rho1 +
                 fmomz_wadvb * wadvb_rho1 + fmomz_presb * presb_rho1;

    jac1(3, 1) = fmomz_rhob * rhob_rhou1 + fmomz_uadvb * uadvb_rhou1 + fmomz_vadvb * vadvb_rhou1 +
                 fmomz_wadvb * wadvb_rhou1 + fmomz_presb * presb_rhou1;

    jac1(3, 2) = fmomz_rhob * rhob_rhov1 + fmomz_uadvb * uadvb_rhov1 + fmomz_vadvb * vadvb_rhov1 +
                 fmomz_wadvb * wadvb_rhov1 + fmomz_presb * presb_rhov1;

    jac1(3, 3) = fmomz_rhob * rhob_rhow1 + fmomz_uadvb * uadvb_rhow1 + fmomz_vadvb * vadvb_rhow1 +
                 fmomz_wadvb * wadvb_rhow1 + fmomz_presb * presb_rhow1;

    jac1(3, 4) = fmomz_rhob * rhob_rhoe1 + fmomz_uadvb * uadvb_rhoe1 + fmomz_vadvb * vadvb_rhoe1 +
                 fmomz_wadvb * wadvb_rhoe1 + fmomz_presb * presb_rhoe1;

    /// calc a(F_etot)/a(U_1)

    jac1(4, 0) = fetot_rhob * rhob_rho1 + fetot_uadvb * uadvb_rho1 + fetot_vadvb * vadvb_rho1 +
                 fetot_wadvb * wadvb_rho1 + fetot_presb * presb_rho1;

    jac1(4, 1) = fetot_rhob * rhob_rhou1 + fetot_uadvb * uadvb_rhou1 + fetot_vadvb * vadvb_rhou1 +
                 fetot_wadvb * wadvb_rhou1 + fetot_presb * presb_rhou1;

    jac1(4, 2) = fetot_rhob * rhob_rhov1 + fetot_uadvb * uadvb_rhov1 + fetot_vadvb * vadvb_rhov1 +
                 fetot_wadvb * wadvb_rhov1 + fetot_presb * presb_rhov1;

    jac1(4, 3) = fetot_rhob * rhob_rhow1 + fetot_uadvb * uadvb_rhow1 + fetot_vadvb * vadvb_rhow1 +
                 fetot_wadvb * wadvb_rhow1 + fetot_presb * presb_rhow1;

    jac1(4, 4) = fetot_rhob * rhob_rhoe1 + fetot_uadvb * uadvb_rhoe1 + fetot_vadvb * vadvb_rhoe1 +
                 fetot_wadvb * wadvb_rhoe1 + fetot_presb * presb_rhoe1;
  }
  else if (mach1 <= -1.)
  {
    /// supersonic inflow

    /// do nothing
  }
  else if (mach1 >= 1.)
  {
    /// supersonic outflow

    Real gamma = _fp.gamma(0., 0.);
    Real gamm1 = gamma - 1.;
    Real gamm2 = 2. - gamma;

    Real rq051 = 0.5 * gamm1 * vdov1;
    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;
    Real enth1 = (rhoe1 + pres1) * rhom1;

    jac1(0, 0) = 0.;
    jac1(0, 1) = nx;
    jac1(0, 2) = ny;
    jac1(0, 3) = nz;
    jac1(0, 4) = 0.;

    jac1(1, 0) = rq051 * nx - uadv1 * vdon1;
    jac1(1, 1) = gamm2 * nx * uadv1 + vdon1;
    jac1(1, 2) = ny * uadv1 - vadv1 * gamm1 * nx;
    jac1(1, 3) = nz * uadv1 - wadv1 * gamm1 * nx;
    jac1(1, 4) = gamm1 * nx;

    jac1(2, 0) = rq051 * ny - vadv1 * vdon1;
    jac1(2, 1) = nx * vadv1 - uadv1 * gamm1 * ny;
    jac1(2, 2) = gamm2 * ny * vadv1 + vdon1;
    jac1(2, 3) = nz * vadv1 - wadv1 * gamm1 * ny;
    jac1(2, 4) = gamm1 * ny;

    jac1(3, 0) = rq051 * nz - wadv1 * vdon1;
    jac1(3, 1) = nx * wadv1 - uadv1 * gamm1 * nz;
    jac1(3, 2) = ny * wadv1 - vadv1 * gamm1 * nz;
    jac1(3, 3) = gamm2 * nz * wadv1 + vdon1;
    jac1(3, 4) = gamm1 * nz;

    jac1(4, 0) = (rq051 - enth1) * vdon1;
    jac1(4, 1) = nx * enth1 - gamm1 * uadv1 * vdon1;
    jac1(4, 2) = ny * enth1 - gamm1 * vadv1 * vdon1;
    jac1(4, 3) = nz * enth1 - gamm1 * wadv1 * vdon1;
    jac1(4, 4) = gamma * vdon1;
  }
  else
    mooseError("Something is wrong in ",
               name(),
               ": ",
               __FUNCTION__,
               "\n",
               "ielem = ",
               ielem,
               "\n",
               "iside = ",
               iside,
               "\n",
               "mach1 = ",
               mach1,
               "\n");
}
