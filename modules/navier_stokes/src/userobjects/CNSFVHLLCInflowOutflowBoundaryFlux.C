/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CNSFVHLLCInflowOutflowBoundaryFlux.h"

template <>
InputParameters
validParams<CNSFVHLLCInflowOutflowBoundaryFlux>()
{
  InputParameters params = validParams<BoundaryFluxBase>();

  params.addClassDescription("A user object that computes inflow/outflow boundary flux using the "
                             "HLLC approximate Riemann solver.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  params.addRequiredParam<Real>("infinity_density", "Infinity density");

  params.addRequiredParam<Real>("infinity_x_velocity",
                                "Infinity velocity component in x-direction");

  params.addParam<Real>("infinity_y_velocity", 0., "Infinity velocity component in y-direction");

  params.addParam<Real>("infinity_z_velocity", 0., "Infinity velocity component in z-direction");

  params.addRequiredParam<Real>("infinity_pressure", "Infinity pressure");

  return params;
}

CNSFVHLLCInflowOutflowBoundaryFlux::CNSFVHLLCInflowOutflowBoundaryFlux(
    const InputParameters & parameters)
  : BoundaryFluxBase(parameters),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _inf_rho(getParam<Real>("infinity_density")),
    _inf_uadv(getParam<Real>("infinity_x_velocity")),
    _inf_vadv(getParam<Real>("infinity_y_velocity")),
    _inf_wadv(getParam<Real>("infinity_z_velocity")),
    _inf_pres(getParam<Real>("infinity_pressure"))
{
}

CNSFVHLLCInflowOutflowBoundaryFlux::~CNSFVHLLCInflowOutflowBoundaryFlux() {}

void
CNSFVHLLCInflowOutflowBoundaryFlux::calcFlux(unsigned int iside,
                                             dof_id_type ielem,
                                             const std::vector<Real> & uvec1,
                                             const RealVectorValue & dwave,
                                             std::vector<Real> & flux) const
{
  Real eps = 1e-6;
  Real gamma = _fp.gamma(0., 0.);

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

  if (mach1 > -1. && mach1 < 0.)
  {
    /// subsonic inflow case
    ///   -- fix pressure (but also OK to extrapolate pressure)
    ///   -- fix temperature
    ///   -- fix velocity
    ///   -- fix mass fractions

    /// state variables required at ghost cell <-- inifinity state

    Real rho2 = _inf_rho;
    Real uadv2 = _inf_uadv;
    Real vadv2 = _inf_vadv;
    Real wadv2 = _inf_wadv;
    Real pres2 = _inf_pres;
    Real rhom2 = 1. / rho2;
    Real rhou2 = rho2 * uadv2;
    Real rhov2 = rho2 * vadv2;
    Real rhow2 = rho2 * wadv2;
    Real vdov2 = uadv2 * uadv2 + vadv2 * vadv2 + wadv2 * wadv2;
    Real eint2 = _fp.e(pres2, rho2);
    Real rhoe2 = rho2 * eint2 + 0.5 * rho2 * vdov2;
    Real csou2 = _fp.c(rhom2, eint2);

    /// get the enthalpy

    Real enth1 = (rhoe1 + pres1) * rhom1;
    Real enth2 = (rhoe2 + pres2) * rhom2;

    /// get the so-called Roe averaged variables

    Real rhsca = std::sqrt(rho2 / rho1);
    Real rmden = 1. / (rhsca + 1.);

    // Real rhoav = rhsca * rho1;
    Real uaver = (rhsca * uadv2 + uadv1) * rmden;
    Real vaver = (rhsca * vadv2 + vadv1) * rmden;
    Real waver = (rhsca * wadv2 + wadv1) * rmden;
    Real entav = (rhsca * enth2 + enth1) * rmden;

    /// averaged speed of sound

    Real qave5 = 0.5 * (uaver * uaver + vaver * vaver + waver * waver);
    Real cssa2 = std::max(eps, (gamma - 1.) * (entav - qave5));
    Real cssav = std::sqrt(cssa2);

    /// compute the eigenvalues at the left, right, and Roe states

    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;
    Real vdon2 = uadv2 * nx + vadv2 * ny + wadv2 * nz;
    Real vnave = uaver * nx + vaver * ny + waver * nz;

    /// get the S_L and S_R defined by Eq. (13).

    Real s1 = std::min(vdon1 - csou1, vnave - cssav);
    Real s2 = std::max(vdon2 + csou2, vnave + cssav);

    Real dsv1 = s1 - vdon1;
    Real dsv2 = s2 - vdon2;

    /// compute the S_M defined by Eq. (12).

    Real sm =
        (rho2 * vdon2 * dsv2 - rho1 * vdon1 * dsv1 + pres1 - pres2) / (rho2 * dsv2 - rho1 * dsv1);

    /// compute the Omega_l, Omega_r, and p^*

    Real omeg1 = 1. / (s1 - sm);
    Real omeg2 = 1. / (s2 - sm);
    Real prsta = rho1 * dsv1 * (sm - vdon1) + pres1;

    Real prst1 = prsta - pres1;
    Real prst2 = prsta - pres2;

    /// compute the U_l^\star, U_r^\star

    Real rhol = omeg1 * dsv1 * rho1;
    Real rhoul = omeg1 * (dsv1 * rhou1 + prst1 * nx);
    Real rhovl = omeg1 * (dsv1 * rhov1 + prst1 * ny);
    Real rhowl = omeg1 * (dsv1 * rhow1 + prst1 * nz);
    Real rhoel = omeg1 * (dsv1 * rhoe1 - pres1 * vdon1 + prsta * sm);

    Real rhor = omeg2 * dsv2 * rho2;
    Real rhour = omeg2 * (dsv2 * rhou2 + prst2 * nx);
    Real rhovr = omeg2 * (dsv2 * rhov2 + prst2 * ny);
    Real rhowr = omeg2 * (dsv2 * rhow2 + prst2 * nz);
    Real rhoer = omeg2 * (dsv2 * rhoe2 - pres2 * vdon2 + prsta * sm);

    /// compute the fluxes according to the wave speed

    if (s1 > 0.)
    {
      flux[0] = vdon1 * rho1;
      flux[1] = vdon1 * rhou1 + pres1 * nx;
      flux[2] = vdon1 * rhov1 + pres1 * ny;
      flux[3] = vdon1 * rhow1 + pres1 * nz;
      flux[4] = vdon1 * (rhoe1 + pres1);
    }
    else if (s1 <= 0. && sm > 0.)
    {
      flux[0] = sm * rhol;
      flux[1] = sm * rhoul + prsta * nx;
      flux[2] = sm * rhovl + prsta * ny;
      flux[3] = sm * rhowl + prsta * nz;
      flux[4] = sm * (rhoel + prsta);
    }
    else if (sm <= 0. && s2 >= 0.)
    {
      flux[0] = sm * rhor;
      flux[1] = sm * rhour + prsta * nx;
      flux[2] = sm * rhovr + prsta * ny;
      flux[3] = sm * rhowr + prsta * nz;
      flux[4] = sm * (rhoer + prsta);
    }
    else if (s2 < 0.)
    {
      flux[0] = vdon2 * rho2;
      flux[1] = vdon2 * rhou2 + pres2 * nx;
      flux[2] = vdon2 * rhov2 + pres2 * ny;
      flux[3] = vdon2 * rhow2 + pres2 * nz;
      flux[4] = vdon2 * (rhoe2 + pres2);
    }
    else
    {
      mooseError("Weird wave speed occured in",
                 "\n",
                 name(),
                 ": ",
                 __FUNCTION__,
                 "\n",
                 "iside = ",
                 iside,
                 "\n",
                 "ielem = ",
                 ielem,
                 "\n",
                 "rho1  = ",
                 rho1,
                 "\n",
                 "rhou1 = ",
                 rhou1,
                 "\n",
                 "rhov1 = ",
                 rhov1,
                 "\n",
                 "rhow1 = ",
                 rhow1,
                 "\n",
                 "rhoe1 = ",
                 rhoe1,
                 "\n",
                 "pres1 = ",
                 pres1,
                 "\n",
                 "enth1 = ",
                 enth1,
                 "\n",
                 "csou1 = ",
                 csou1,
                 "\n",
                 "rho2  = ",
                 rho2,
                 "\n",
                 "rhou2 = ",
                 rhou2,
                 "\n",
                 "rhov2 = ",
                 rhov2,
                 "\n",
                 "rhoe2 = ",
                 rhoe2,
                 "\n",
                 "pres2 = ",
                 pres2,
                 "\n",
                 "enth2 = ",
                 enth2,
                 "\n",
                 "csou2 = ",
                 csou2,
                 "\n",
                 "vdon1 = ",
                 vdon1,
                 "\n",
                 "vdon2 = ",
                 vdon2,
                 "\n",
                 "vnave = ",
                 vnave,
                 "\n",
                 "cssav = ",
                 cssav,
                 "\n",
                 "s1    = ",
                 s1,
                 "\n",
                 "s2    = ",
                 s2,
                 "\n",
                 "sm    = ",
                 sm,
                 "\n",
                 "omeg1 = ",
                 omeg1,
                 "\n",
                 "omeg2 = ",
                 omeg2,
                 "\n",
                 "prsta = ",
                 prsta,
                 "\n",
                 "Please check before continuing!\n");
    }
  }
  else if (mach1 >= 0. && mach1 < 1.)
  {
    /// subsonic outflow case
    ///   -- fix pressure
    ///   -- extrapolate temperature
    ///   -- extrapolate velocities
    ///   -- extrapolate mass fractions

    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;

    flux[0] = vdon1 * rho1;
    flux[1] = vdon1 * rhou1 + _inf_pres * nx;
    flux[2] = vdon1 * rhov1 + _inf_pres * ny;
    flux[3] = vdon1 * rhow1 + _inf_pres * nz;
    flux[4] = vdon1 * (rhoe1 + _inf_pres);
  }
  else if (mach1 <= -1.)
  {
    /// supersonic inflow case
    ///   -- fix pressure
    ///   -- fix temperature
    ///   -- fix velocity
    ///   -- fix mass fractions

    Real rho2 = _inf_rho;
    Real uadv2 = _inf_uadv;
    Real vadv2 = _inf_vadv;
    Real wadv2 = _inf_wadv;
    Real pres2 = _inf_pres;
    Real rhou2 = rho2 * uadv2;
    Real rhov2 = rho2 * vadv2;
    Real rhow2 = rho2 * wadv2;
    Real vdov2 = uadv2 * uadv2 + vadv2 * vadv2 + wadv2 * wadv2;
    Real eint2 = _fp.e(pres2, rho2);
    Real rhoe2 = rho2 * eint2 + 0.5 * rho2 * vdov2;
    Real vdon2 = uadv2 * nx + vadv2 * ny + wadv2 * nz;

    flux[0] = vdon2 * rho2;
    flux[1] = vdon2 * rhou2 + pres2 * nx;
    flux[2] = vdon2 * rhov2 + pres2 * ny;
    flux[3] = vdon2 * rhow2 + pres2 * nz;
    flux[4] = vdon2 * (rhoe2 + pres2);
  }
  else if (mach1 >= 1.)
  {
    /// supersonic outflow case
    ///   -- extrapolate pressure
    ///   -- extrapolate temperature
    ///   -- extrapolate velocity
    ///   -- extrapolate mass fractions

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
CNSFVHLLCInflowOutflowBoundaryFlux::calcJacobian(unsigned int iside,
                                                 dof_id_type ielem,
                                                 const std::vector<Real> & uvec1,
                                                 const RealVectorValue & dwave,
                                                 DenseMatrix<Real> & jac1) const
{
  Real eps = 1e-6;
  Real gamma = _fp.gamma(0., 0.);
  Real gamm1 = gamma - 1.;
  Real gamm2 = 2. - gamma;

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
  Real rq051 = 0.5 * gamm1 * vdov1;
  Real mach1 = std::sqrt(vdov1) / csou1;

  if (mach1 > -1. && mach1 < 0.)
  {
    /// subsonic inflow case
    ///   -- fix pressure (but also OK to extrapolate pressure)
    ///   -- fix temperature
    ///   -- fix velocity
    ///   -- fix mass fractions

    /// state variables required at ghost cell <-- inifinity state

    Real rho2 = _inf_rho;
    Real uadv2 = _inf_uadv;
    Real vadv2 = _inf_vadv;
    Real wadv2 = _inf_wadv;
    Real pres2 = _inf_pres;
    Real rhom2 = 1. / rho2;
    Real rhou2 = rho2 * uadv2;
    Real rhov2 = rho2 * vadv2;
    Real rhow2 = rho2 * wadv2;
    Real vdov2 = uadv2 * uadv2 + vadv2 * vadv2 + wadv2 * wadv2;
    Real eint2 = _fp.e(pres2, rho2);
    Real rhoe2 = rho2 * eint2 + 0.5 * rho2 * vdov2;
    Real csou2 = _fp.c(rhom2, eint2);

    /// get the enthalpy

    Real enth1 = (rhoe1 + pres1) * rhom1;
    Real enth2 = (rhoe2 + pres2) * rhom2;

    /// get the so-called Roe averaged variables

    Real rhsca = std::sqrt(rho2 / rho1);
    Real rmden = 1. / (rhsca + 1.);

    // Real rhoav = rhsca * rho1;
    Real uaver = (rhsca * uadv2 + uadv1) * rmden;
    Real vaver = (rhsca * vadv2 + vadv1) * rmden;
    Real waver = (rhsca * wadv2 + wadv1) * rmden;
    Real entav = (rhsca * enth2 + enth1) * rmden;

    /// averaged speed of sound

    Real qave5 = 0.5 * (uaver * uaver + vaver * vaver + waver * waver);
    Real cssa2 = std::max(eps, (gamma - 1.) * (entav - qave5));
    Real cssav = std::sqrt(cssa2);

    /// compute the eigenvalues at the left, right, and Roe states

    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;
    Real vdon2 = uadv2 * nx + vadv2 * ny + wadv2 * nz;
    Real vnave = uaver * nx + vaver * ny + waver * nz;

    /// get the S_L and S_R defined by Eq. (13).

    Real s1 = std::min(vdon1 - csou1, vnave - cssav);
    Real s2 = std::max(vdon2 + csou2, vnave + cssav);

    Real dsv1 = s1 - vdon1;
    Real dsv2 = s2 - vdon2;

    /// compute the S_M defined by Eq. (12).

    Real sm =
        (rho2 * vdon2 * dsv2 - rho1 * vdon1 * dsv1 + pres1 - pres2) / (rho2 * dsv2 - rho1 * dsv1);

    /// compute the flux Jacobians according to the wave speed

    if (s1 > 0.)
    {
      /// get the Jacobian matrix on the left only

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
    else if (s1 <= 0. && sm > 0.)
    {
      /// compute the Omega_l and p^* by Eq. (10). and (11).

      Real omeg1 = 1. / (s1 - sm);
      Real prsta = rho1 * dsv1 * (sm - vdon1) + pres1;

      Real prst1 = prsta - pres1;

      /// compute the U_l^\star by Eq. (7).

      Real rhol = omeg1 * dsv1 * rho1;
      Real rhoul = omeg1 * (dsv1 * rhou1 + prst1 * nx);
      Real rhovl = omeg1 * (dsv1 * rhov1 + prst1 * ny);
      Real rhowl = omeg1 * (dsv1 * rhow1 + prst1 * nz);
      Real rhoel = omeg1 * (dsv1 * rhoe1 - pres1 * vdon1 + prsta * sm);

      Real rhoepl = rhoel + prsta;

      /// compute rho^\tilde

      Real rhotd = std::max(eps, rho2 * (s2 - vdon2) - rho1 * (s1 - vdon1));
      Real rhotm = 1. / rhotd;

      /// compute a(S_M)/a(U_l) by Eq. (42).

      Real sm_rho1 = rhotm * (-vdon1 * vdon1 + rq051 + sm * s1);
      Real sm_rhou1 = rhotm * (nx * (2. * vdon1 - s1 - sm) - gamm1 * uadv1);
      Real sm_rhov1 = rhotm * (ny * (2. * vdon1 - s1 - sm) - gamm1 * vadv1);
      Real sm_rhow1 = rhotm * (nz * (2. * vdon1 - s1 - sm) - gamm1 * wadv1);
      Real sm_rhoe1 = rhotm * gamm1;

      /// compute a(p^*)/a(U_l) by Eq. (44).

      Real ps_rho1 = rho2 * dsv2 * sm_rho1;
      Real ps_rhou1 = rho2 * dsv2 * sm_rhou1;
      Real ps_rhov1 = rho2 * dsv2 * sm_rhov1;
      Real ps_rhow1 = rho2 * dsv2 * sm_rhow1;
      Real ps_rhoe1 = rho2 * dsv2 * sm_rhoe1;

      /// compute a(rho_l^\star)/a(U_l) by Eq. (46).

      Real rhol_rho1 = omeg1 * (s1 + rhol * sm_rho1);
      Real rhol_rhou1 = omeg1 * (-nx + rhol * sm_rhou1);
      Real rhol_rhov1 = omeg1 * (-ny + rhol * sm_rhov1);
      Real rhol_rhow1 = omeg1 * (-nz + rhol * sm_rhow1);
      Real rhol_rhoe1 = omeg1 * (rhol * sm_rhoe1);

      /// compute a(rhou_l^\star)/a(U_l) by Eq. (47).

      Real rhoul_rho1 = omeg1 * (uadv1 * vdon1 - nx * rq051 + nx * ps_rho1 + rhoul * sm_rho1);
      Real rhoul_rhou1 = omeg1 * (dsv1 - gamm2 * nx * uadv1 + nx * ps_rhou1 + rhoul * sm_rhou1);
      Real rhoul_rhov1 =
          omeg1 * (-uadv1 * ny + gamm1 * nx * vadv1 + nx * ps_rhov1 + rhoul * sm_rhov1);
      Real rhoul_rhow1 =
          omeg1 * (-uadv1 * nz + gamm1 * nx * wadv1 + nx * ps_rhow1 + rhoul * sm_rhow1);
      Real rhoul_rhoe1 = omeg1 * (-gamm1 * nx + nx * ps_rhoe1 + rhoul * sm_rhoe1);

      /// compute a(rhov_l^\star)/a(U_l) by Eq. (49).

      Real rhovl_rho1 = omeg1 * (vadv1 * vdon1 - ny * rq051 + ny * ps_rho1 + rhovl * sm_rho1);
      Real rhovl_rhou1 =
          omeg1 * (-vadv1 * nx + gamm1 * ny * uadv1 + ny * ps_rhou1 + rhovl * sm_rhou1);
      Real rhovl_rhov1 = omeg1 * (dsv1 - gamm2 * ny * vadv1 + ny * ps_rhov1 + rhovl * sm_rhov1);
      Real rhovl_rhow1 =
          omeg1 * (-vadv1 * nz + gamm1 * ny * wadv1 + ny * ps_rhow1 + rhovl * sm_rhow1);
      Real rhovl_rhoe1 = omeg1 * (-gamm1 * ny + ny * ps_rhoe1 + rhovl * sm_rhoe1);

      /// compute a(rhow_l^\star)/a(U_l) by Eq. (51).

      Real rhowl_rho1 = omeg1 * (wadv1 * vdon1 - nz * rq051 + nz * ps_rho1 + rhowl * sm_rho1);
      Real rhowl_rhou1 =
          omeg1 * (-wadv1 * nx + gamm1 * nz * uadv1 + nz * ps_rhou1 + rhowl * sm_rhou1);
      Real rhowl_rhov1 =
          omeg1 * (-wadv1 * ny + gamm1 * nz * vadv1 + nz * ps_rhov1 + rhowl * sm_rhov1);
      Real rhowl_rhow1 = omeg1 * (dsv1 - gamm2 * nz * wadv1 + nz * ps_rhow1 + rhowl * sm_rhow1);
      Real rhowl_rhoe1 = omeg1 * (-gamm1 * nz + nz * ps_rhoe1 + rhowl * sm_rhoe1);

      /// compute a(rhoe_l^\star)/a(U_l) by Eq. (53).
      /// note that enth1  = (rhoe1 + pres1)*rhom1
      /// and rhoepl = rhoel + prsta

      Real rhoel_rho1 = omeg1 * (vdon1 * enth1 - vdon1 * rq051 + sm * ps_rho1 + rhoepl * sm_rho1);
      Real rhoel_rhou1 =
          omeg1 * (-nx * enth1 + gamm1 * vdon1 * uadv1 + sm * ps_rhou1 + rhoepl * sm_rhou1);
      Real rhoel_rhov1 =
          omeg1 * (-ny * enth1 + gamm1 * vdon1 * vadv1 + sm * ps_rhov1 + rhoepl * sm_rhov1);
      Real rhoel_rhow1 =
          omeg1 * (-nz * enth1 + gamm1 * vdon1 * wadv1 + sm * ps_rhow1 + rhoepl * sm_rhow1);
      Real rhoel_rhoe1 = omeg1 * (s1 - vdon1 * gamma + sm * ps_rhoe1 + rhoepl * sm_rhoe1);

      /// compute the HLLC Jacobians a(F_l^\star)/a(U_l) by Eq. (40).

      jac1(0, 0) = sm * rhol_rho1 + rhol * sm_rho1;
      jac1(0, 1) = sm * rhol_rhou1 + rhol * sm_rhou1;
      jac1(0, 2) = sm * rhol_rhov1 + rhol * sm_rhov1;
      jac1(0, 3) = sm * rhol_rhow1 + rhol * sm_rhow1;
      jac1(0, 4) = sm * rhol_rhoe1 + rhol * sm_rhoe1;

      jac1(1, 0) = sm * rhoul_rho1 + rhoul * sm_rho1 + nx * ps_rho1;
      jac1(1, 1) = sm * rhoul_rhou1 + rhoul * sm_rhou1 + nx * ps_rhou1;
      jac1(1, 2) = sm * rhoul_rhov1 + rhoul * sm_rhov1 + nx * ps_rhov1;
      jac1(1, 3) = sm * rhoul_rhow1 + rhoul * sm_rhow1 + nx * ps_rhow1;
      jac1(1, 4) = sm * rhoul_rhoe1 + rhoul * sm_rhoe1 + nx * ps_rhoe1;

      jac1(2, 0) = sm * rhovl_rho1 + rhovl * sm_rho1 + ny * ps_rho1;
      jac1(2, 1) = sm * rhovl_rhou1 + rhovl * sm_rhou1 + ny * ps_rhou1;
      jac1(2, 2) = sm * rhovl_rhov1 + rhovl * sm_rhov1 + ny * ps_rhov1;
      jac1(2, 3) = sm * rhovl_rhow1 + rhovl * sm_rhow1 + ny * ps_rhow1;
      jac1(2, 4) = sm * rhovl_rhoe1 + rhovl * sm_rhoe1 + ny * ps_rhoe1;

      jac1(3, 0) = sm * rhowl_rho1 + rhowl * sm_rho1 + nz * ps_rho1;
      jac1(3, 1) = sm * rhowl_rhou1 + rhowl * sm_rhou1 + nz * ps_rhou1;
      jac1(3, 2) = sm * rhowl_rhov1 + rhowl * sm_rhov1 + nz * ps_rhov1;
      jac1(3, 3) = sm * rhowl_rhow1 + rhowl * sm_rhow1 + nz * ps_rhow1;
      jac1(3, 4) = sm * rhowl_rhoe1 + rhowl * sm_rhoe1 + nz * ps_rhoe1;

      jac1(4, 0) = sm * (rhoel_rho1 + ps_rho1) + rhoepl * sm_rho1;
      jac1(4, 1) = sm * (rhoel_rhou1 + ps_rhou1) + rhoepl * sm_rhou1;
      jac1(4, 2) = sm * (rhoel_rhov1 + ps_rhov1) + rhoepl * sm_rhov1;
      jac1(4, 3) = sm * (rhoel_rhow1 + ps_rhow1) + rhoepl * sm_rhow1;
      jac1(4, 4) = sm * (rhoel_rhoe1 + ps_rhoe1) + rhoepl * sm_rhoe1;
    }
    else if (sm <= 0. && s2 >= 0.)
    {
      /// compute the Omega_r, and p^* by Eq. (10). and (11).

      Real omeg2 = 1. / (s2 - sm);
      Real prsta = rho2 * dsv2 * (sm - vdon2) + pres2;

      Real prst2 = prsta - pres2;

      /// compute the U_r^\star by Eq. (8).

      Real rhor = omeg2 * dsv2 * rho2;
      Real rhour = omeg2 * (dsv2 * rhou2 + prst2 * nx);
      Real rhovr = omeg2 * (dsv2 * rhov2 + prst2 * ny);
      Real rhowr = omeg2 * (dsv2 * rhow2 + prst2 * nz);
      Real rhoer = omeg2 * (dsv2 * rhoe2 - pres2 * vdon2 + prsta * sm);

      Real rhoepr = rhoer + prsta;

      /// compute rho^\tilde

      Real rhotd = std::max(eps, rho2 * (s2 - vdon2) - rho1 * (s1 - vdon1));
      Real rhotm = 1. / rhotd;

      /// compute a(S_M)/a(U_l) by Eq. (42).

      Real sm_rho1 = rhotm * (-vdon1 * vdon1 + rq051 + sm * s1);
      Real sm_rhou1 = rhotm * (nx * (2. * vdon1 - s1 - sm) - gamm1 * uadv1);
      Real sm_rhov1 = rhotm * (ny * (2. * vdon1 - s1 - sm) - gamm1 * vadv1);
      Real sm_rhow1 = rhotm * (nz * (2. * vdon1 - s1 - sm) - gamm1 * wadv1);
      Real sm_rhoe1 = rhotm * gamm1;

      /// compute a(p^*)/a(U_l) by Eq. (44).

      Real ps_rho1 = rho2 * dsv2 * sm_rho1;
      Real ps_rhou1 = rho2 * dsv2 * sm_rhou1;
      Real ps_rhov1 = rho2 * dsv2 * sm_rhov1;
      Real ps_rhow1 = rho2 * dsv2 * sm_rhow1;
      Real ps_rhoe1 = rho2 * dsv2 * sm_rhoe1;

      /// compute a(rho_r^\star)/a(U_l) by Eq. (46a*).

      Real rhor_rho1 = omeg2 * rhor * sm_rho1;
      Real rhor_rhou1 = omeg2 * rhor * sm_rhou1;
      Real rhor_rhov1 = omeg2 * rhor * sm_rhov1;
      Real rhor_rhow1 = omeg2 * rhor * sm_rhow1;
      Real rhor_rhoe1 = omeg2 * rhor * sm_rhoe1;

      /// compute a(rhou_r^\star)/a(U_l) by Eq. (48*).

      Real rhour_rho1 = omeg2 * (nx * ps_rho1 + rhour * sm_rho1);
      Real rhour_rhou1 = omeg2 * (nx * ps_rhou1 + rhour * sm_rhou1);
      Real rhour_rhov1 = omeg2 * (nx * ps_rhov1 + rhour * sm_rhov1);
      Real rhour_rhow1 = omeg2 * (nx * ps_rhow1 + rhour * sm_rhow1);
      Real rhour_rhoe1 = omeg2 * (nx * ps_rhoe1 + rhour * sm_rhoe1);

      /// compute a(rhov_r^\star)/a(U_l) by Eq. (50*).

      Real rhovr_rho1 = omeg2 * (ny * ps_rho1 + rhovr * sm_rho1);
      Real rhovr_rhou1 = omeg2 * (ny * ps_rhou1 + rhovr * sm_rhou1);
      Real rhovr_rhov1 = omeg2 * (ny * ps_rhov1 + rhovr * sm_rhov1);
      Real rhovr_rhow1 = omeg2 * (ny * ps_rhow1 + rhovr * sm_rhow1);
      Real rhovr_rhoe1 = omeg2 * (ny * ps_rhoe1 + rhovr * sm_rhoe1);

      /// compute a(rhow_r^\star)/a(U_l) by Eq. (52*).

      Real rhowr_rho1 = omeg2 * (nz * ps_rho1 + rhowr * sm_rho1);
      Real rhowr_rhou1 = omeg2 * (nz * ps_rhou1 + rhowr * sm_rhou1);
      Real rhowr_rhov1 = omeg2 * (nz * ps_rhov1 + rhowr * sm_rhov1);
      Real rhowr_rhow1 = omeg2 * (nz * ps_rhow1 + rhowr * sm_rhow1);
      Real rhowr_rhoe1 = omeg2 * (nz * ps_rhoe1 + rhowr * sm_rhoe1);

      /// compute a(rhoe_r^\star)/a(U_l) by Eq. (54*).

      Real rhoer_rho1 = omeg2 * (sm * ps_rho1 + rhoepr * sm_rho1);
      Real rhoer_rhou1 = omeg2 * (sm * ps_rhou1 + rhoepr * sm_rhou1);
      Real rhoer_rhov1 = omeg2 * (sm * ps_rhov1 + rhoepr * sm_rhov1);
      Real rhoer_rhow1 = omeg2 * (sm * ps_rhow1 + rhoepr * sm_rhow1);
      Real rhoer_rhoe1 = omeg2 * (sm * ps_rhoe1 + rhoepr * sm_rhoe1);

      /// compute the HLLC Jacobians a(F_r^\star)/a(U_l) by Eq. (41*).

      jac1(0, 0) = sm * rhor_rho1 + rhor * sm_rho1;
      jac1(0, 1) = sm * rhor_rhou1 + rhor * sm_rhou1;
      jac1(0, 2) = sm * rhor_rhov1 + rhor * sm_rhov1;
      jac1(0, 3) = sm * rhor_rhow1 + rhor * sm_rhow1;
      jac1(0, 4) = sm * rhor_rhoe1 + rhor * sm_rhoe1;

      jac1(1, 0) = sm * rhour_rho1 + rhour * sm_rho1 + nx * ps_rho1;
      jac1(1, 1) = sm * rhour_rhou1 + rhour * sm_rhou1 + nx * ps_rhou1;
      jac1(1, 2) = sm * rhour_rhov1 + rhour * sm_rhov1 + nx * ps_rhov1;
      jac1(1, 3) = sm * rhour_rhow1 + rhour * sm_rhow1 + nx * ps_rhow1;
      jac1(1, 4) = sm * rhour_rhoe1 + rhour * sm_rhoe1 + nx * ps_rhoe1;

      jac1(2, 0) = sm * rhovr_rho1 + rhovr * sm_rho1 + ny * ps_rho1;
      jac1(2, 1) = sm * rhovr_rhou1 + rhovr * sm_rhou1 + ny * ps_rhou1;
      jac1(2, 2) = sm * rhovr_rhov1 + rhovr * sm_rhov1 + ny * ps_rhov1;
      jac1(2, 3) = sm * rhovr_rhow1 + rhovr * sm_rhow1 + ny * ps_rhow1;
      jac1(2, 4) = sm * rhovr_rhoe1 + rhovr * sm_rhoe1 + ny * ps_rhoe1;

      jac1(3, 0) = sm * rhowr_rho1 + rhowr * sm_rho1 + nz * ps_rho1;
      jac1(3, 1) = sm * rhowr_rhou1 + rhowr * sm_rhou1 + nz * ps_rhou1;
      jac1(3, 2) = sm * rhowr_rhov1 + rhowr * sm_rhov1 + nz * ps_rhov1;
      jac1(3, 3) = sm * rhowr_rhow1 + rhowr * sm_rhow1 + nz * ps_rhow1;
      jac1(3, 4) = sm * rhowr_rhoe1 + rhowr * sm_rhoe1 + nz * ps_rhoe1;

      jac1(4, 0) = sm * (rhoer_rho1 + ps_rho1) + rhoepr * sm_rho1;
      jac1(4, 1) = sm * (rhoer_rhou1 + ps_rhou1) + rhoepr * sm_rhou1;
      jac1(4, 2) = sm * (rhoer_rhov1 + ps_rhov1) + rhoepr * sm_rhov1;
      jac1(4, 3) = sm * (rhoer_rhow1 + ps_rhow1) + rhoepr * sm_rhow1;
      jac1(4, 4) = sm * (rhoer_rhoe1 + ps_rhoe1) + rhoepr * sm_rhoe1;
    }
    else if (s2 < 0.)
    {
      /// do nothing
    }
    else
    {
      /// compute the Omega_l and p^* by Eq. (10). and (11).

      Real omeg1 = 1. / (s1 - sm);
      Real omeg2 = 1. / (s2 - sm);
      Real prsta = rho1 * dsv1 * (sm - vdon1) + pres1;

      mooseError("Weird wave speed occured in ",
                 name(),
                 ": ",
                 __FUNCTION__,
                 "\n",
                 "iside = ",
                 iside,
                 "\n",
                 "ielem = ",
                 ielem,
                 "\n",
                 "rho1  = ",
                 rho1,
                 "\n",
                 "rhou1 = ",
                 rhou1,
                 "\n",
                 "rhov1 = ",
                 rhov1,
                 "\n",
                 "rhow1 = ",
                 rhow1,
                 "\n",
                 "rhoe1 = ",
                 rhoe1,
                 "\n",
                 "pres1 = ",
                 pres1,
                 "\n",
                 "enth1 = ",
                 enth1,
                 "\n",
                 "csou1 = ",
                 csou1,
                 "\n",
                 "rho2  = ",
                 rho2,
                 "\n",
                 "rhou2 = ",
                 rhou2,
                 "\n",
                 "rhov2 = ",
                 rhov2,
                 "\n",
                 "rhoe2 = ",
                 rhoe2,
                 "\n",
                 "pres2 = ",
                 pres2,
                 "\n",
                 "enth2 = ",
                 enth2,
                 "\n",
                 "csou2 = ",
                 csou2,
                 "\n",
                 "vdon1 = ",
                 vdon1,
                 "\n",
                 "vdon2 = ",
                 vdon2,
                 "\n",
                 "vnave = ",
                 vnave,
                 "\n",
                 "cssav = ",
                 cssav,
                 "\n",
                 "s1    = ",
                 s1,
                 "\n",
                 "s2    = ",
                 s2,
                 "\n",
                 "sm    = ",
                 sm,
                 "\n",
                 "omeg1 = ",
                 omeg1,
                 "\n",
                 "omeg2 = ",
                 omeg2,
                 "\n",
                 "prsta = ",
                 prsta,
                 "\n",
                 "Please check before continuing!\n");
    }
  }
  else if (mach1 >= 0. && mach1 < 1.)
  {
    /// subsonic outflow case
    ///   -- fix pressure
    ///   -- extrapolate temperature
    ///   -- extrapolate velocities
    ///   -- extrapolate mass fractions

    Real vdon1 = uadv1 * nx + vadv1 * ny + wadv1 * nz;
    Real enth1 = (rhoe1 + _inf_pres) * rhom1;

    jac1(0, 0) = 0.;
    jac1(0, 1) = nx;
    jac1(0, 2) = ny;
    jac1(0, 3) = nz;
    jac1(0, 4) = 0.;

    jac1(1, 0) = -vdon1 * uadv1;
    jac1(1, 1) = nx * uadv1 + vdon1;
    jac1(1, 2) = ny * uadv1;
    jac1(1, 3) = nz * uadv1;
    jac1(1, 4) = 0.;

    jac1(2, 0) = -vdon1 * vadv1;
    jac1(2, 1) = nx * vadv1;
    jac1(2, 2) = ny * vadv1 + vdon1;
    jac1(2, 3) = nz * vadv1;
    jac1(2, 4) = 0.;

    jac1(3, 0) = -vdon1 * wadv1;
    jac1(3, 1) = nx * wadv1;
    jac1(3, 2) = ny * wadv1;
    jac1(3, 3) = nz * wadv1 + vdon1;
    jac1(3, 4) = 0.;

    jac1(4, 0) = -vdon1 * enth1;
    jac1(4, 1) = nx * enth1;
    jac1(4, 2) = ny * enth1;
    jac1(4, 3) = nz * enth1;
    jac1(4, 4) = vdon1;
  }
  else if (mach1 <= -1.)
  {
    /// supersonic inflow case
    ///   -- fix pressure
    ///   -- fix temperature
    ///   -- fix velocity
    ///   -- fix mass fractions

    /// do nothing
  }
  else if (mach1 >= 1.)
  {
    /// supersonic outflow case
    ///   -- extrapolate pressure
    ///   -- extrapolate temperature
    ///   -- extrapolate velocity
    ///   -- extrapolate mass fractions

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
