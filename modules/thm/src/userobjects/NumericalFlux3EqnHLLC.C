#include "NumericalFlux3EqnHLLC.h"
#include "Numerics.h"

#define SHOW(var) ss << #var << " = " << var << "\n"

registerMooseObject("RELAP7App", NumericalFlux3EqnHLLC);

template <>
InputParameters
validParams<NumericalFlux3EqnHLLC>()
{
  InputParameters params = validParams<NumericalFlux3EqnBase>();

  params.addClassDescription("Computes internal side flux for the 1-D, 1-phase, variable-area "
                             "Euler equations using the HLLC approximate Riemann solver.");

  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "Name for fluid properties user object");

  params.addParam<bool>(
      "use_approximate_jacobian", true, "Use frozen acoustic speeds Jacobian approximation");

  return params;
}

NumericalFlux3EqnHLLC::NumericalFlux3EqnHLLC(const InputParameters & parameters)
  : NumericalFlux3EqnBase(parameters),

    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties")),
    _use_approximate_jacobian(getParam<bool>("use_approximate_jacobian"))
{
}

void
NumericalFlux3EqnHLLC::calcFlux(const std::vector<Real> & U1,
                                const std::vector<Real> & U2,
                                const RealVectorValue & normal,
                                std::vector<Real> & flux) const
{
  // extract the conserved variables and area

  const Real rhoA1 = U1[VAR_RHOA];
  const Real rhouA1 = U1[VAR_RHOUA];
  const Real rhoEA1 = U1[VAR_RHOEA];
  const Real A1 = U1[VAR_A];

  const Real rhoA2 = U2[VAR_RHOA];
  const Real rhouA2 = U2[VAR_RHOUA];
  const Real rhoEA2 = U2[VAR_RHOEA];
  const Real A2 = U2[VAR_A];

  // areas are assumed be equal, so name the area to be either side
  mooseAssert(std::abs(A2 - A1) < 1e-15, "Left and right areas must be equal.");
  const Real A = A1;

  // x-component of normal vector
  const Real nx = normal(0);

  // compute the primitive variables

  const Real rho1 = rhoA1 / A1;
  const Real rhou1 = rhouA1 / A1;
  const Real rhoE1 = rhoEA1 / A1;
  const Real u1 = rhouA1 / rhoA1;
  const Real q1 = u1 * nx;
  const Real v1 = 1.0 / rho1;
  const Real E1 = rhoEA1 / rhoA1;
  const Real e1 = E1 - 0.5 * u1 * u1;
  const Real p1 = _fp.p_from_v_e(v1, e1);
  const Real H1 = E1 + p1 / rho1;
  const Real c1 = _fp.c_from_v_e(v1, e1);

  const Real rho2 = rhoA2 / A2;
  const Real rhou2 = rhouA2 / A2;
  const Real rhoE2 = rhoEA2 / A2;
  const Real u2 = rhouA2 / rhoA2;
  const Real q2 = u2 * nx;
  const Real v2 = 1.0 / rho2;
  const Real E2 = rhoEA2 / rhoA2;
  const Real e2 = E2 - 0.5 * u2 * u2;
  const Real p2 = _fp.p_from_v_e(v2, e2);
  const Real H2 = E2 + p2 / rho2;
  const Real c2 = _fp.c_from_v_e(v2, e2);

  // compute Roe-averaged variables
  const Real sqrt_rho1 = std::sqrt(rho1);
  const Real sqrt_rho2 = std::sqrt(rho2);
  const Real u_roe = (sqrt_rho1 * u1 + sqrt_rho2 * u2) / (sqrt_rho1 + sqrt_rho2);
  const Real q_roe = u_roe * nx;
  const Real H_roe = (sqrt_rho1 * H1 + sqrt_rho2 * H2) / (sqrt_rho1 + sqrt_rho2);
  const Real h_roe = H_roe - 0.5 * u_roe * u_roe;
  const Real rho_roe = std::sqrt(rho1 * rho2);
  const Real v_roe = 1.0 / rho_roe;
  const Real e_roe = _fp.e_from_v_h(v_roe, h_roe);
  const Real c_roe = _fp.c_from_v_e(v_roe, e_roe);

  // compute wave speeds
  const Real s1 = std::min(q1 - c1, q_roe - c_roe);
  const Real s2 = std::max(q2 + c2, q_roe + c_roe);
  const Real sm = (rho2 * q2 * (s2 - q2) - rho1 * q1 * (s1 - q1) + p1 - p2) /
                  (rho2 * (s2 - q2) - rho1 * (s1 - q1));

  // compute Omega_L, Omega_R
  const Real omeg1 = 1.0 / (s1 - sm);
  const Real omeg2 = 1.0 / (s2 - sm);

  // compute p^*
  const Real ps = rho1 * (s1 - q1) * (sm - q1) + p1;

  // compute U_L^*, U_R^*

  const Real rhoLs = omeg1 * (s1 - q1) * rho1;
  const Real rhouLs = omeg1 * ((s1 - q1) * rhou1 + (ps - p1) * nx);
  const Real rhoELs = omeg1 * ((s1 - q1) * rhoE1 - p1 * q1 + ps * sm);

  const Real rhoRs = omeg2 * (s2 - q2) * rho2;
  const Real rhouRs = omeg2 * ((s2 - q2) * rhou2 + (ps - p2) * nx);
  const Real rhoERs = omeg2 * ((s2 - q2) * rhoE2 - p2 * q2 + ps * sm);

  // compute the fluxes
  flux.resize(_n_eq);
  if (s1 > 0.0)
  {
    flux[EQ_MASS] = u1 * rho1 * A1;
    flux[EQ_MOMENTUM] = (u1 * rhou1 + p1) * A1;
    flux[EQ_ENERGY] = u1 * (rhoE1 + p1) * A1;

    _last_region_index = 0;
  }
  else if (s1 <= 0.0 && sm > 0.0)
  {
    flux[EQ_MASS] = sm * nx * rhoLs * A;
    flux[EQ_MOMENTUM] = (sm * nx * rhouLs + ps) * A;
    flux[EQ_ENERGY] = sm * nx * (rhoELs + ps) * A;

    _last_region_index = 1;
  }
  else if (sm <= 0.0 && s2 >= 0.0)
  {
    flux[EQ_MASS] = sm * nx * rhoRs * A;
    flux[EQ_MOMENTUM] = (sm * nx * rhouRs + ps) * A;
    flux[EQ_ENERGY] = sm * nx * (rhoERs + ps) * A;

    _last_region_index = 2;
  }
  else if (s2 < 0.0)
  {
    flux[EQ_MASS] = u2 * rho2 * A2;
    flux[EQ_MOMENTUM] = (u2 * rhou2 + p2) * A2;
    flux[EQ_ENERGY] = u2 * (rhoE2 + p2) * A2;

    _last_region_index = 3;
  }
  else
  {
    std::stringstream ss;
    ss << "Sampling error occurred in " << name() << ": " << __FUNCTION__ << ":\n";
    SHOW(rho1);
    SHOW(u1);
    SHOW(E1);
    SHOW(p1);
    SHOW(c1);
    SHOW(rho2);
    SHOW(u2);
    SHOW(E2);
    SHOW(p2);
    SHOW(c2);
    SHOW(q1);
    SHOW(q2);
    SHOW(q_roe);
    SHOW(c_roe);
    SHOW(s1);
    SHOW(s2);
    SHOW(sm);
    SHOW(ps);
    mooseException(ss.str());
  }
}

void
NumericalFlux3EqnHLLC::calcJacobian(const std::vector<Real> & U1,
                                    const std::vector<Real> & U2,
                                    const RealVectorValue & normal,
                                    DenseMatrix<Real> & jac1,
                                    DenseMatrix<Real> & jac2) const
{
  // extract the conserved variables and area

  const Real rhoA1 = U1[VAR_RHOA];
  const Real rhouA1 = U1[VAR_RHOUA];
  const Real rhoEA1 = U1[VAR_RHOEA];
  const Real A1 = U1[VAR_A];

  const Real rhoA2 = U2[VAR_RHOA];
  const Real rhouA2 = U2[VAR_RHOUA];
  const Real rhoEA2 = U2[VAR_RHOEA];
  const Real A2 = U2[VAR_A];

  const Real rho1 = rhoA1 / A1;
  const Real rhou1 = rhouA1 / A1;
  const Real rhoE1 = rhoEA1 / A1;

  const Real rho2 = rhoA2 / A2;
  const Real rhou2 = rhouA2 / A2;
  const Real rhoE2 = rhoEA2 / A2;

  // x-component of normal vector
  const Real nx = normal(0);

  // velocity
  Real u1, du1_drho1, du1_drhou1;
  RELAP7::vel_from_arhoA_arhouA(rho1, rhou1, u1, du1_drho1, du1_drhou1);
  const Real q1 = u1 * nx;
  const Real dq1_drho1 = du1_drho1 * nx;
  const Real dq1_drhou1 = du1_drhou1 * nx;

  Real u2, du2_drho2, du2_drhou2;
  RELAP7::vel_from_arhoA_arhouA(rho2, rhou2, u2, du2_drho2, du2_drhou2);
  const Real q2 = u2 * nx;
  const Real dq2_drho2 = du2_drho2 * nx;
  const Real dq2_drhou2 = du2_drhou2 * nx;

  // specific volume
  const Real v1 = 1.0 / rho1;
  const Real dv1_drho1 = RELAP7::dv_darhoA(1.0, rho1);

  const Real v2 = 1.0 / rho2;
  const Real dv2_drho2 = RELAP7::dv_darhoA(1.0, rho2);

  // specific internal energy
  const Real e1 = rhoE1 / rho1 - 0.5 * rhou1 * rhou1 / (rho1 * rho1);
  const Real de1_drho1 = RELAP7::de_darhoA(rho1, rhou1, rhoE1);
  const Real de1_drhou1 = RELAP7::de_darhouA(rho1, rhou1);
  const Real de1_drhoE1 = RELAP7::de_darhoEA(rho1);

  const Real e2 = rhoE2 / rho2 - 0.5 * rhou2 * rhou2 / (rho2 * rho2);
  const Real de2_drho2 = RELAP7::de_darhoA(rho2, rhou2, rhoE2);
  const Real de2_drhou2 = RELAP7::de_darhouA(rho2, rhou2);
  const Real de2_drhoE2 = RELAP7::de_darhoEA(rho2);

  // pressure
  Real p1, dp1_dv1, dp1_de1;
  _fp.p_from_v_e(v1, e1, p1, dp1_dv1, dp1_de1);
  const Real dp1_drho1 = dp1_dv1 * dv1_drho1 + dp1_de1 * de1_drho1;
  const Real dp1_drhou1 = dp1_de1 * de1_drhou1;
  const Real dp1_drhoE1 = dp1_de1 * de1_drhoE1;

  Real p2, dp2_dv2, dp2_de2;
  _fp.p_from_v_e(v2, e2, p2, dp2_dv2, dp2_de2);
  const Real dp2_drho2 = dp2_dv2 * dv2_drho2 + dp2_de2 * de2_drho2;
  const Real dp2_drhou2 = dp2_de2 * de2_drhou2;
  const Real dp2_drhoE2 = dp2_de2 * de2_drhoE2;

  // compute Roe-averaged variables
  const Real sqrt_rho1 = std::sqrt(rho1);
  const Real sqrt_rho2 = std::sqrt(rho2);
  const Real sqrt_rho_sum = sqrt_rho1 + sqrt_rho2;
  const Real u_roe = (sqrt_rho1 * u1 + sqrt_rho2 * u2) / sqrt_rho_sum;
  const Real q_roe = u_roe * nx;
  const Real H1 = rhoEA1 / rhoA1 + p1 / rho1;
  const Real H2 = rhoEA2 / rhoA2 + p2 / rho2;
  const Real H_roe = (sqrt_rho1 * H1 + sqrt_rho2 * H2) / sqrt_rho_sum;
  const Real h_roe = H_roe - 0.5 * u_roe * u_roe;
  const Real rho_roe = std::sqrt(rho1 * rho2);
  const Real v_roe = 1.0 / rho_roe;

  Real e_roe, de_roe_dv_roe, de_roe_dh_roe;
  _fp.e_from_v_h(v_roe, h_roe, e_roe, de_roe_dv_roe, de_roe_dh_roe);

  Real c_roe, dc_roe_dv_roe_partial, dc_roe_de_roe;
  _fp.c_from_v_e(v_roe, e_roe, c_roe, dc_roe_dv_roe_partial, dc_roe_de_roe);

  // sound speed
  Real c1, dc1_dv1, dc1_de1;
  _fp.c_from_v_e(v1, e1, c1, dc1_dv1, dc1_de1);

  Real c2, dc2_dv2, dc2_de2;
  _fp.c_from_v_e(v2, e2, c2, dc2_dv2, dc2_de2);

  // compute wave speeds
  const Real lambda1 = q1 - c1;
  const Real lambda1_roe = q_roe - c_roe;
  const Real s1 = std::min(lambda1, lambda1_roe);

  const Real lambda2 = q2 + c2;
  const Real lambda2_roe = q_roe + c_roe;
  const Real s2 = std::max(lambda2, lambda2_roe);

  const Real sm = (rho2 * q2 * (s2 - q2) - rho1 * q1 * (s1 - q1) + p1 - p2) /
                  (rho2 * (s2 - q2) - rho1 * (s1 - q1));

  // compute the flux Jacobians

  jac1.resize(_n_eq, _n_eq);
  jac2.resize(_n_eq, _n_eq);
  if (s1 > 0.0) // left region
  {
    jac1(EQ_MASS, EQ_MASS) = 0.0;
    jac1(EQ_MASS, EQ_MOMENTUM) = 1.0;
    jac1(EQ_MASS, EQ_ENERGY) = 0.0;

    jac1(EQ_MOMENTUM, EQ_MASS) = (dp1_drho1 - u1 * u1);
    jac1(EQ_MOMENTUM, EQ_MOMENTUM) = (2.0 * u1 + dp1_drhou1);
    jac1(EQ_MOMENTUM, EQ_ENERGY) = dp1_drhoE1;

    jac1(EQ_ENERGY, EQ_MASS) = (u1 * dp1_drho1 - u1 / rho1 * (rhoE1 + p1));
    jac1(EQ_ENERGY, EQ_MOMENTUM) = ((rhoE1 + p1) / rho1 + u1 * dp1_drhou1);
    jac1(EQ_ENERGY, EQ_ENERGY) = u1 * (1.0 + dp1_drhoE1);

    _last_region_index = 0;
  }
  else if (s2 < 0.0) // right region
  {
    jac2(EQ_MASS, EQ_MASS) = 0.0;
    jac2(EQ_MASS, EQ_MOMENTUM) = 1.0;
    jac2(EQ_MASS, EQ_ENERGY) = 0.0;

    jac2(EQ_MOMENTUM, EQ_MASS) = (dp2_drho2 - u2 * u2);
    jac2(EQ_MOMENTUM, EQ_MOMENTUM) = (2.0 * u2 + dp2_drhou2);
    jac2(EQ_MOMENTUM, EQ_ENERGY) = dp2_drhoE2;

    jac2(EQ_ENERGY, EQ_MASS) = (u2 * dp2_drho2 - u2 / rho2 * (rhoE2 + p2));
    jac2(EQ_ENERGY, EQ_MOMENTUM) = ((rhoE2 + p2) / rho2 + u2 * dp2_drhou2);
    jac2(EQ_ENERGY, EQ_ENERGY) = u2 * (1.0 + dp2_drhoE2);

    _last_region_index = 3;
  }
  else // star (left or right) region
  {
    Real ds1_drho1 = 0, ds1_drhou1 = 0, ds1_drhoE1 = 0, ds1_drho2 = 0, ds1_drhou2 = 0,
         ds1_drhoE2 = 0;
    Real ds2_drho1 = 0, ds2_drhou1 = 0, ds2_drhoE1 = 0, ds2_drho2 = 0, ds2_drhou2 = 0,
         ds2_drhoE2 = 0;
    if (!_use_approximate_jacobian)
    {
      const Real dc1_drho1 = dc1_dv1 * dv1_drho1 + dc1_de1 * de1_drho1;
      const Real dc2_drho2 = dc2_dv2 * dv2_drho2 + dc2_de2 * de2_drho2;
      const Real dc1_drhou1 = dc1_de1 * de1_drhou1;
      const Real dc2_drhou2 = dc2_de2 * de2_drhou2;
      const Real dc1_drhoE1 = dc1_de1 * de1_drhoE1;
      const Real dc2_drhoE2 = dc2_de2 * de2_drhoE2;

      const Real dH1_drho1 = -rhoE1 / (rho1 * rho1) + dp1_drho1 / rho1 - p1 / (rho1 * rho1);
      const Real dH2_drho2 = -rhoE2 / (rho2 * rho2) + dp2_drho2 / rho2 - p2 / (rho2 * rho2);
      const Real dH1_drhou1 = dp1_drhou1 / rho1;
      const Real dH2_drhou2 = dp2_drhou2 / rho2;
      const Real dH1_drhoE1 = 1.0 / rho1 + dp1_drhoE1 / rho1;
      const Real dH2_drhoE2 = 1.0 / rho2 + dp2_drhoE2 / rho2;

      const Real du_roe_drho1 = (0.5 / sqrt_rho1 * u1 + sqrt_rho1 * du1_drho1) / sqrt_rho_sum -
                                0.5 * u_roe / (sqrt_rho_sum * sqrt_rho1);
      const Real du_roe_drho2 = (0.5 / sqrt_rho2 * u2 + sqrt_rho2 * du2_drho2) / sqrt_rho_sum -
                                0.5 * u_roe / (sqrt_rho_sum * sqrt_rho2);
      const Real du_roe_drhou1 = sqrt_rho1 * du1_drhou1 / sqrt_rho_sum;
      const Real du_roe_drhou2 = sqrt_rho2 * du2_drhou2 / sqrt_rho_sum;

      const Real dq_roe_drho1 = du_roe_drho1 * nx;
      const Real dq_roe_drho2 = du_roe_drho2 * nx;
      const Real dq_roe_drhou1 = du_roe_drhou1 * nx;
      const Real dq_roe_drhou2 = du_roe_drhou2 * nx;

      const Real drho_roe_drho1 = 0.5 * rho2 / std::sqrt(rho1 * rho2);
      const Real drho_roe_drho2 = 0.5 * rho1 / std::sqrt(rho1 * rho2);

      const Real dv_roe_drho1 = -drho_roe_drho1 / (rho_roe * rho_roe);
      const Real dv_roe_drho2 = -drho_roe_drho2 / (rho_roe * rho_roe);

      const Real dH_roe_drho1 = (0.5 / sqrt_rho1 * H1 + sqrt_rho1 * dH1_drho1) / sqrt_rho_sum -
                                0.5 * H_roe / (sqrt_rho_sum * sqrt_rho1);
      const Real dH_roe_drho2 = (0.5 / sqrt_rho2 * H2 + sqrt_rho2 * dH2_drho2) / sqrt_rho_sum -
                                0.5 * H_roe / (sqrt_rho_sum * sqrt_rho2);
      const Real dH_roe_drhou1 = sqrt_rho1 * dH1_drhou1 / sqrt_rho_sum;
      const Real dH_roe_drhou2 = sqrt_rho2 * dH2_drhou2 / sqrt_rho_sum;
      const Real dH_roe_drhoE1 = sqrt_rho1 * dH1_drhoE1 / sqrt_rho_sum;
      const Real dH_roe_drhoE2 = sqrt_rho2 * dH2_drhoE2 / sqrt_rho_sum;

      const Real dh_roe_drho1 = dH_roe_drho1 - u_roe * du_roe_drho1;
      const Real dh_roe_drho2 = dH_roe_drho2 - u_roe * du_roe_drho2;
      const Real dh_roe_drhou1 = dH_roe_drhou1 - u_roe * du_roe_drhou1;
      const Real dh_roe_drhou2 = dH_roe_drhou2 - u_roe * du_roe_drhou2;
      const Real dh_roe_drhoE1 = dH_roe_drhoE1;
      const Real dh_roe_drhoE2 = dH_roe_drhoE2;

      const Real dc_roe_dv_roe = dc_roe_dv_roe_partial + dc_roe_de_roe * de_roe_dv_roe;
      const Real dc_roe_dh_roe = dc_roe_de_roe * de_roe_dh_roe;
      const Real dc_roe_drho1 = dc_roe_dv_roe * dv_roe_drho1 + dc_roe_dh_roe * dh_roe_drho1;
      const Real dc_roe_drho2 = dc_roe_dv_roe * dv_roe_drho2 + dc_roe_dh_roe * dh_roe_drho2;
      const Real dc_roe_drhou1 = dc_roe_dh_roe * dh_roe_drhou1;
      const Real dc_roe_drhou2 = dc_roe_dh_roe * dh_roe_drhou2;
      const Real dc_roe_drhoE1 = dc_roe_dh_roe * dh_roe_drhoE1;
      const Real dc_roe_drhoE2 = dc_roe_dh_roe * dh_roe_drhoE2;

      if (lambda1 <= lambda1_roe)
      {
        ds1_drho1 = dq1_drho1 - dc1_drho1;
        ds1_drhou1 = dq1_drhou1 - dc1_drhou1;
        ds1_drhoE1 = -dc1_drhoE1;
      }
      else
      {
        ds1_drho1 = dq_roe_drho1 - dc_roe_drho1;
        ds1_drhou1 = dq_roe_drhou1 - dc_roe_drhou1;
        ds1_drhoE1 = -dc_roe_drhoE1;
        ds1_drho2 = dq_roe_drho2 - dc_roe_drho2;
        ds1_drhou2 = dq_roe_drhou2 - dc_roe_drhou2;
        ds1_drhoE2 = -dc_roe_drhoE2;
      }

      if (lambda2 >= lambda2_roe)
      {
        ds2_drho2 = dq2_drho2 + dc2_drho2;
        ds2_drhou2 = dq2_drhou2 + dc2_drhou2;
        ds2_drhoE2 = dc2_drhoE2;
      }
      else
      {
        ds2_drho1 = dq_roe_drho1 + dc_roe_drho1;
        ds2_drhou1 = dq_roe_drhou1 + dc_roe_drhou1;
        ds2_drhoE1 = dc_roe_drhoE1;
        ds2_drho2 = dq_roe_drho2 + dc_roe_drho2;
        ds2_drhou2 = dq_roe_drhou2 + dc_roe_drhou2;
        ds2_drhoE2 = dc_roe_drhoE2;
      }
    }

    // numerator of S_M
    const Real smA = rho2 * q2 * (s2 - q2) - rho1 * q1 * (s1 - q1) + p1 - p2;
    const Real dsmA_drho1 = rho2 * q2 * ds2_drho1 - rho1 * q1 * (ds1_drho1 - dq1_drho1) + dp1_drho1;
    const Real dsmA_drho2 = rho2 * q2 * (ds2_drho2 - dq2_drho2) - rho1 * q1 * ds1_drho2 - dp2_drho2;
    const Real dsmA_drhou1 = -nx * (s1 - q1) + rho2 * q2 * ds2_drhou1 -
                             rho1 * q1 * (ds1_drhou1 - dq1_drhou1) + dp1_drhou1;
    const Real dsmA_drhou2 = nx * (s2 - q2) + rho2 * q2 * (ds2_drhou2 - dq2_drhou2) -
                             rho1 * q1 * ds1_drhou2 - dp2_drhou2;
    const Real dsmA_drhoE1 = rho2 * q2 * ds2_drhoE1 - rho1 * q1 * ds1_drhoE1 + dp1_drhoE1;
    const Real dsmA_drhoE2 = rho2 * q2 * ds2_drhoE2 - rho1 * q1 * ds1_drhoE2 - dp2_drhoE2;

    // denominator of S_M
    const Real smB = rho2 * (s2 - q2) - rho1 * (s1 - q1);
    const Real dsmB_drho1 = -(s1 - q1) + rho2 * ds2_drho1 - rho1 * (ds1_drho1 - dq1_drho1);
    const Real dsmB_drho2 = (s2 - q2) + rho2 * (ds2_drho2 - dq2_drho2) - rho1 * ds1_drho2;
    const Real dsmB_drhou1 = rho2 * ds2_drhou1 - rho1 * (ds1_drhou1 - dq1_drhou1);
    const Real dsmB_drhou2 = rho2 * (ds2_drhou2 - dq2_drhou2) - rho1 * ds1_drhou2;
    const Real dsmB_drhoE1 = rho2 * ds2_drhoE1 - rho1 * ds1_drhoE1;
    const Real dsmB_drhoE2 = rho2 * ds2_drhoE2 - rho1 * ds1_drhoE2;

    // S_M
    const Real dsm_drho1 = dsmA_drho1 / smB - smA / smB / smB * dsmB_drho1;
    const Real dsm_drho2 = dsmA_drho2 / smB - smA / smB / smB * dsmB_drho2;
    const Real dsm_drhou1 = dsmA_drhou1 / smB - smA / smB / smB * dsmB_drhou1;
    const Real dsm_drhou2 = dsmA_drhou2 / smB - smA / smB / smB * dsmB_drhou2;
    const Real dsm_drhoE1 = dsmA_drhoE1 / smB - smA / smB / smB * dsmB_drhoE1;
    const Real dsm_drhoE2 = dsmA_drhoE2 / smB - smA / smB / smB * dsmB_drhoE2;

    // p^*
    const Real ps = rho1 * (s1 - q1) * (sm - q1) + p1;
    const Real dps_drho1 = rho2 * (s2 - q2) * dsm_drho1 + rho2 * (sm - q2) * ds2_drho1;
    const Real dps_drho2 = rho1 * (s1 - q1) * dsm_drho2 + rho1 * (sm - q1) * ds1_drho2;
    const Real dps_drhou1 = rho2 * (s2 - q2) * dsm_drhou1 + rho2 * (sm - q2) * ds2_drhou1;
    const Real dps_drhou2 = rho1 * (s1 - q1) * dsm_drhou2 + rho1 * (sm - q1) * ds1_drhou2;
    const Real dps_drhoE1 = rho2 * (s2 - q2) * dsm_drhoE1 + rho2 * (sm - q2) * ds2_drhoE1;
    const Real dps_drhoE2 = rho1 * (s1 - q1) * dsm_drhoE2 + rho1 * (sm - q1) * ds1_drhoE2;

    if (s1 <= 0.0 && sm > 0.0) // left star region
    {
      // Omega
      const Real omeg1 = 1.0 / (s1 - sm);
      const Real domeg1_drho1 = -omeg1 * omeg1 * (ds1_drho1 - dsm_drho1);
      const Real domeg1_drhou1 = -omeg1 * omeg1 * (ds1_drhou1 - dsm_drhou1);
      const Real domeg1_drhoE1 = -omeg1 * omeg1 * (ds1_drhoE1 - dsm_drhoE1);
      const Real domeg1_drho2 = -omeg1 * omeg1 * (ds1_drho2 - dsm_drho2);
      const Real domeg1_drhou2 = -omeg1 * omeg1 * (ds1_drhou2 - dsm_drhou2);
      const Real domeg1_drhoE2 = -omeg1 * omeg1 * (ds1_drhoE2 - dsm_drhoE2);

      // rho
      const Real rhoLs = omeg1 * (s1 - q1) * rho1;
      const Real rhoLs_mult = (s1 - q1) * rho1;
      const Real omegrho1 = omeg1 * rho1;
      const Real drhoLs_drho1 =
          domeg1_drho1 * rhoLs_mult + omegrho1 * (ds1_drho1 - dq1_drho1) + omeg1 * (s1 - q1);
      const Real drhoLs_drho2 = domeg1_drho2 * rhoLs_mult + omegrho1 * ds1_drho2;
      const Real drhoLs_drhou1 = domeg1_drhou1 * rhoLs_mult + omegrho1 * (ds1_drhou1 - dq1_drhou1);
      const Real drhoLs_drhou2 = domeg1_drhou2 * rhoLs_mult + omegrho1 * ds1_drhou2;
      const Real drhoLs_drhoE1 = domeg1_drhoE1 * rhoLs_mult + omegrho1 * ds1_drhoE1;
      const Real drhoLs_drhoE2 = domeg1_drhoE2 * rhoLs_mult + omegrho1 * ds1_drhoE2;

      // rho * u
      const Real rhouLs = omeg1 * ((s1 - q1) * rhou1 + (ps - p1) * nx);
      const Real rhouLs_mult = (s1 - q1) * rhou1 + (ps - p1) * nx;
      const Real drhouLs_drho1 =
          domeg1_drho1 * rhouLs_mult +
          omeg1 * ((ds1_drho1 - dq1_drho1) * rhou1 + (dps_drho1 - dp1_drho1) * nx);
      const Real drhouLs_drho2 =
          domeg1_drho2 * rhouLs_mult + omeg1 * (ds1_drho2 * rhou1 + dps_drho2 * nx);
      const Real drhouLs_drhou1 =
          domeg1_drhou1 * rhouLs_mult +
          omeg1 * ((ds1_drhou1 - dq1_drhou1) * rhou1 + (s1 - q1) + (dps_drhou1 - dp1_drhou1) * nx);
      const Real drhouLs_drhou2 =
          domeg1_drhou2 * rhouLs_mult + omeg1 * (ds1_drhou2 * rhou1 + dps_drhou2 * nx);
      const Real drhouLs_drhoE1 = domeg1_drhoE1 * rhouLs_mult +
                                  omeg1 * (ds1_drhoE1 * rhou1 + (dps_drhoE1 - dp1_drhoE1) * nx);
      const Real drhouLs_drhoE2 =
          domeg1_drhoE2 * rhouLs_mult + omeg1 * (ds1_drhoE2 * rhou1 + dps_drhoE2 * nx);

      // rho * E
      const Real rhoELs = omeg1 * ((s1 - q1) * rhoE1 - p1 * q1 + ps * sm);
      const Real rhoELs_mult = (s1 - q1) * rhoE1 - p1 * q1 + ps * sm;
      const Real drhoELs_drho1 =
          domeg1_drho1 * rhoELs_mult + omeg1 * ((ds1_drho1 - dq1_drho1) * rhoE1 - dp1_drho1 * q1 -
                                                p1 * dq1_drho1 + dps_drho1 * sm + ps * dsm_drho1);
      const Real drhoELs_drho2 = domeg1_drho2 * rhoELs_mult +
                                 omeg1 * (ds1_drho2 * rhoE1 + dps_drho2 * sm + ps * dsm_drho2);
      const Real drhoELs_drhou1 = domeg1_drhou1 * rhoELs_mult +
                                  omeg1 * ((ds1_drhou1 - dq1_drhou1) * rhoE1 - dp1_drhou1 * q1 -
                                           p1 * dq1_drhou1 + dps_drhou1 * sm + ps * dsm_drhou1);
      const Real drhoELs_drhou2 = domeg1_drhou2 * rhoELs_mult +
                                  omeg1 * (ds1_drhou2 * rhoE1 + dps_drhou2 * sm + ps * dsm_drhou2);
      const Real drhoELs_drhoE1 =
          domeg1_drhoE1 * rhoELs_mult + omeg1 * (ds1_drhoE1 * rhoE1 + s1 - q1 - dp1_drhoE1 * q1 +
                                                 dps_drhoE1 * sm + ps * dsm_drhoE1);
      const Real drhoELs_drhoE2 = domeg1_drhoE2 * rhoELs_mult +
                                  omeg1 * (ds1_drhoE2 * rhoE1 + dps_drhoE2 * sm + ps * dsm_drhoE2);

      // left Jacobian

      jac1(EQ_MASS, EQ_MASS) = sm * drhoLs_drho1 + rhoLs * dsm_drho1;
      jac1(EQ_MASS, EQ_MOMENTUM) = sm * drhoLs_drhou1 + rhoLs * dsm_drhou1;
      jac1(EQ_MASS, EQ_ENERGY) = sm * drhoLs_drhoE1 + rhoLs * dsm_drhoE1;

      jac1(EQ_MOMENTUM, EQ_MASS) = sm * drhouLs_drho1 + rhouLs * dsm_drho1 + nx * dps_drho1;
      jac1(EQ_MOMENTUM, EQ_MOMENTUM) = sm * drhouLs_drhou1 + rhouLs * dsm_drhou1 + nx * dps_drhou1;
      jac1(EQ_MOMENTUM, EQ_ENERGY) = sm * drhouLs_drhoE1 + rhouLs * dsm_drhoE1 + nx * dps_drhoE1;

      jac1(EQ_ENERGY, EQ_MASS) = sm * (drhoELs_drho1 + dps_drho1) + (rhoELs + ps) * dsm_drho1;
      jac1(EQ_ENERGY, EQ_MOMENTUM) =
          sm * (drhoELs_drhou1 + dps_drhou1) + (rhoELs + ps) * dsm_drhou1;
      jac1(EQ_ENERGY, EQ_ENERGY) = sm * (drhoELs_drhoE1 + dps_drhoE1) + (rhoELs + ps) * dsm_drhoE1;

      jac1 *= nx;

      // right Jacobian

      jac2(EQ_MASS, EQ_MASS) = sm * drhoLs_drho2 + rhoLs * dsm_drho2;
      jac2(EQ_MASS, EQ_MOMENTUM) = sm * drhoLs_drhou2 + rhoLs * dsm_drhou2;
      jac2(EQ_MASS, EQ_ENERGY) = sm * drhoLs_drhoE2 + rhoLs * dsm_drhoE2;

      jac2(EQ_MOMENTUM, EQ_MASS) = sm * drhouLs_drho2 + rhouLs * dsm_drho2 + nx * dps_drho2;
      jac2(EQ_MOMENTUM, EQ_MOMENTUM) = sm * drhouLs_drhou2 + rhouLs * dsm_drhou2 + nx * dps_drhou2;
      jac2(EQ_MOMENTUM, EQ_ENERGY) = sm * drhouLs_drhoE2 + rhouLs * dsm_drhoE2 + nx * dps_drhoE2;

      jac2(EQ_ENERGY, EQ_MASS) = sm * (drhoELs_drho2 + dps_drho2) + (rhoELs + ps) * dsm_drho2;
      jac2(EQ_ENERGY, EQ_MOMENTUM) =
          sm * (drhoELs_drhou2 + dps_drhou2) + (rhoELs + ps) * dsm_drhou2;
      jac2(EQ_ENERGY, EQ_ENERGY) = sm * (drhoELs_drhoE2 + dps_drhoE2) + (rhoELs + ps) * dsm_drhoE2;

      jac2 *= nx;

      _last_region_index = 1;
    }
    else if (sm <= 0.0 && s2 >= 0.0) // right star region
    {
      // Omega
      const Real omeg2 = 1.0 / (s2 - sm);
      const Real domeg2_drho1 = -omeg2 * omeg2 * (ds2_drho1 - dsm_drho1);
      const Real domeg2_drhou1 = -omeg2 * omeg2 * (ds2_drhou1 - dsm_drhou1);
      const Real domeg2_drhoE1 = -omeg2 * omeg2 * (ds2_drhoE1 - dsm_drhoE1);
      const Real domeg2_drho2 = -omeg2 * omeg2 * (ds2_drho2 - dsm_drho2);
      const Real domeg2_drhou2 = -omeg2 * omeg2 * (ds2_drhou2 - dsm_drhou2);
      const Real domeg2_drhoE2 = -omeg2 * omeg2 * (ds2_drhoE2 - dsm_drhoE2);

      // rho
      const Real rhoRs = omeg2 * (s2 - q2) * rho2;
      const Real rhoRs_mult = (s2 - q2) * rho2;
      const Real omegrho2 = omeg2 * rho2;
      const Real drhoRs_drho1 = domeg2_drho1 * rhoRs_mult + omegrho2 * ds2_drho1;
      const Real drhoRs_drho2 =
          domeg2_drho2 * rhoRs_mult + omegrho2 * (ds2_drho2 - dq2_drho2) + omeg2 * (s2 - q2);
      const Real drhoRs_drhou1 = domeg2_drhou1 * rhoRs_mult + omegrho2 * ds2_drhou1;
      const Real drhoRs_drhou2 = domeg2_drhou2 * rhoRs_mult + omegrho2 * (ds2_drhou2 - dq2_drhou2);
      const Real drhoRs_drhoE1 = domeg2_drhoE1 * rhoRs_mult + omegrho2 * ds2_drhoE1;
      const Real drhoRs_drhoE2 = domeg2_drhoE2 * rhoRs_mult + omegrho2 * ds2_drhoE2;

      // rho * u
      const Real rhouRs = omeg2 * ((s2 - q2) * rhou2 + (ps - p2) * nx);
      const Real rhouRs_mult = (s2 - q2) * rhou2 + (ps - p2) * nx;
      const Real drhouRs_drho1 =
          domeg2_drho1 * rhouRs_mult + omeg2 * (ds2_drho1 * rhou2 + dps_drho1 * nx);
      const Real drhouRs_drho2 =
          domeg2_drho2 * rhouRs_mult +
          omeg2 * ((ds2_drho2 - dq2_drho2) * rhou2 + (dps_drho2 - dp2_drho2) * nx);
      const Real drhouRs_drhou1 =
          domeg2_drhou1 * rhouRs_mult + omeg2 * (ds2_drhou1 * rhou2 + dps_drhou1 * nx);
      const Real drhouRs_drhou2 =
          domeg2_drhou2 * rhouRs_mult +
          omeg2 * ((ds2_drhou2 - dq2_drhou2) * rhou2 + (s2 - q2) + (dps_drhou2 - dp2_drhou2) * nx);
      const Real drhouRs_drhoE1 =
          domeg2_drhoE1 * rhouRs_mult + omeg2 * (ds2_drhoE1 * rhou2 + dps_drhoE1 * nx);
      const Real drhouRs_drhoE2 = domeg2_drhoE2 * rhouRs_mult +
                                  omeg2 * (ds2_drhoE2 * rhou2 + (dps_drhoE2 - dp2_drhoE2) * nx);

      // rho * E
      const Real rhoERs = omeg2 * ((s2 - q2) * rhoE2 - p2 * q2 + ps * sm);
      const Real rhoERs_mult = (s2 - q2) * rhoE2 - p2 * q2 + ps * sm;
      const Real drhoERs_drho1 = domeg2_drho1 * rhoERs_mult +
                                 omeg2 * (ds2_drho1 * rhoE2 + dps_drho1 * sm + ps * dsm_drho1);
      const Real drhoERs_drho2 =
          domeg2_drho2 * rhoERs_mult + omeg2 * ((ds2_drho2 - dq2_drho2) * rhoE2 - dp2_drho2 * q2 -
                                                p2 * dq2_drho2 + dps_drho2 * sm + ps * dsm_drho2);
      const Real drhoERs_drhou1 = domeg2_drhou1 * rhoERs_mult +
                                  omeg2 * (ds2_drhou1 * rhoE2 + dps_drhou1 * sm + ps * dsm_drhou1);
      const Real drhoERs_drhou2 = domeg2_drhou2 * rhoERs_mult +
                                  omeg2 * ((ds2_drhou2 - dq2_drhou2) * rhoE2 - dp2_drhou2 * q2 -
                                           p2 * dq2_drhou2 + dps_drhou2 * sm + ps * dsm_drhou2);
      const Real drhoERs_drhoE1 = domeg2_drhoE1 * rhoERs_mult +
                                  omeg2 * (ds2_drhoE1 * rhoE2 + dps_drhoE1 * sm + ps * dsm_drhoE1);
      const Real drhoERs_drhoE2 =
          domeg2_drhoE2 * rhoERs_mult + omeg2 * (ds2_drhoE2 * rhoE2 + s2 - q2 - dp2_drhoE2 * q2 +
                                                 dps_drhoE2 * sm + ps * dsm_drhoE2);

      // left Jacobian

      jac1(EQ_MASS, EQ_MASS) = sm * drhoRs_drho1 + rhoRs * dsm_drho1;
      jac1(EQ_MASS, EQ_MOMENTUM) = sm * drhoRs_drhou1 + rhoRs * dsm_drhou1;
      jac1(EQ_MASS, EQ_ENERGY) = sm * drhoRs_drhoE1 + rhoRs * dsm_drhoE1;

      jac1(EQ_MOMENTUM, EQ_MASS) = sm * drhouRs_drho1 + rhouRs * dsm_drho1 + nx * dps_drho1;
      jac1(EQ_MOMENTUM, EQ_MOMENTUM) = sm * drhouRs_drhou1 + rhouRs * dsm_drhou1 + nx * dps_drhou1;
      jac1(EQ_MOMENTUM, EQ_ENERGY) = sm * drhouRs_drhoE1 + rhouRs * dsm_drhoE1 + nx * dps_drhoE1;

      jac1(EQ_ENERGY, EQ_MASS) = sm * (drhoERs_drho1 + dps_drho1) + (rhoERs + ps) * dsm_drho1;
      jac1(EQ_ENERGY, EQ_MOMENTUM) =
          sm * (drhoERs_drhou1 + dps_drhou1) + (rhoERs + ps) * dsm_drhou1;
      jac1(EQ_ENERGY, EQ_ENERGY) = sm * (drhoERs_drhoE1 + dps_drhoE1) + (rhoERs + ps) * dsm_drhoE1;

      jac1 *= nx;

      // right Jacobian

      jac2(EQ_MASS, EQ_MASS) = sm * drhoRs_drho2 + rhoRs * dsm_drho2;
      jac2(EQ_MASS, EQ_MOMENTUM) = sm * drhoRs_drhou2 + rhoRs * dsm_drhou2;
      jac2(EQ_MASS, EQ_ENERGY) = sm * drhoRs_drhoE2 + rhoRs * dsm_drhoE2;

      jac2(EQ_MOMENTUM, EQ_MASS) = sm * drhouRs_drho2 + rhouRs * dsm_drho2 + nx * dps_drho2;
      jac2(EQ_MOMENTUM, EQ_MOMENTUM) = sm * drhouRs_drhou2 + rhouRs * dsm_drhou2 + nx * dps_drhou2;
      jac2(EQ_MOMENTUM, EQ_ENERGY) = sm * drhouRs_drhoE2 + rhouRs * dsm_drhoE2 + nx * dps_drhoE2;

      jac2(EQ_ENERGY, EQ_MASS) = sm * (drhoERs_drho2 + dps_drho2) + (rhoERs + ps) * dsm_drho2;
      jac2(EQ_ENERGY, EQ_MOMENTUM) =
          sm * (drhoERs_drhou2 + dps_drhou2) + (rhoERs + ps) * dsm_drhou2;
      jac2(EQ_ENERGY, EQ_ENERGY) = sm * (drhoERs_drhoE2 + dps_drhoE2) + (rhoERs + ps) * dsm_drhoE2;

      jac2 *= nx;

      _last_region_index = 2;
    }
  }
}
