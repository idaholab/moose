#include "ShaftConnectedCompressor1PhaseUserObject.h"
#include "ShaftConnectedCompressor1Phase.h"
#include "SinglePhaseFluidProperties.h"
#include "VolumeJunction1Phase.h"
#include "MooseVariableScalar.h"
#include "THMIndices3Eqn.h"
#include "Function.h"
#include "Numerics.h"

registerMooseObject("THMApp", ShaftConnectedCompressor1PhaseUserObject);

InputParameters
ShaftConnectedCompressor1PhaseUserObject::validParams()
{
  InputParameters params = VolumeJunction1PhaseUserObject::validParams();
  params += ShaftConnectableUserObjectInterface::validParams();

  params.addParam<BoundaryName>("inlet", "Compressor inlet");
  params.addParam<BoundaryName>("outlet", "Compressor outlet");
  params.addRequiredParam<Point>("di_out", "Direction of connected outlet");
  params.addRequiredParam<Real>("omega_rated", "Rated compressor speed [rad/s]");
  params.addRequiredParam<Real>("mdot_rated", "Rated compressor mass flow rate [kg/s]");
  params.addRequiredParam<Real>("rho0_rated",
                                "Rated compressor inlet stagnation fluid density [kg/m^3]");
  params.addRequiredParam<Real>("c0_rated", "Rated compressor inlet stagnation sound speed [m/s]");
  params.addRequiredParam<Real>("speed_cr_fr", "Compressor speed threshold for friction [-]");
  params.addRequiredParam<Real>("tau_fr_const", "Compressor friction constant [N-m]");
  params.addRequiredParam<std::vector<Real>>("tau_fr_coeff", "Friction coefficients [N-m]");
  params.addRequiredParam<Real>("speed_cr_I", "Compressor speed threshold for inertia [-]");
  params.addRequiredParam<Real>("inertia_const", "Compressor inertia constant [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>("inertia_coeff",
                                             "Compressor inertia coefficients [kg-m^2]");
  params.addRequiredParam<std::vector<Real>>(
      "speeds",
      "Relative corrected speeds. Order of speeds needs correspond to the "
      "orders of `Rp_functions` and `eff_functions` [-]");
  params.addRequiredParam<std::vector<FunctionName>>(
      "Rp_functions",
      "Functions of pressure ratio versus relative corrected flow. Each function is for a "
      "different, constant relative corrected speed. The order of function names should correspond "
      "to the order of speeds in the `speeds` parameter [-]");
  params.addRequiredParam<std::vector<FunctionName>>(
      "eff_functions",
      "Functions of adiabatic efficiency versus relative corrected flow. Each function is for a "
      "different, constant relative corrected speed. The order of function names should correspond "
      "to the order of speeds in the `speeds` parameter [-]");
  params.addRequiredParam<std::string>("compressor_name",
                                       "Name of the instance of this compressor component");
  params.addRequiredCoupledVar("omega", "Shaft rotational speed [rad/s]");

  params.addClassDescription("Computes and caches flux and residual vectors for a 1-phase "
                             "compressor. Also computes compressor torque "
                             "and delta_p which is passed to the connected shaft");

  return params;
}

ShaftConnectedCompressor1PhaseUserObject::ShaftConnectedCompressor1PhaseUserObject(
    const InputParameters & params)
  : VolumeJunction1PhaseUserObject(params),
    ShaftConnectableUserObjectInterface(this),

    _di_out(getParam<Point>("di_out")),
    _omega_rated(getParam<Real>("omega_rated")),
    _mdot_rated(getParam<Real>("mdot_rated")),
    _rho0_rated(getParam<Real>("rho0_rated")),
    _c0_rated(getParam<Real>("c0_rated")),
    _speed_cr_fr(getParam<Real>("speed_cr_fr")),
    _tau_fr_const(getParam<Real>("tau_fr_const")),
    _tau_fr_coeff(getParam<std::vector<Real>>("tau_fr_coeff")),
    _speed_cr_I(getParam<Real>("speed_cr_I")),
    _inertia_const(getParam<Real>("inertia_const")),
    _inertia_coeff(getParam<std::vector<Real>>("inertia_coeff")),
    _speeds(getParam<std::vector<Real>>("speeds")),
    _Rp_function_names(getParam<std::vector<FunctionName>>("Rp_functions")),
    _eff_function_names(getParam<std::vector<FunctionName>>("eff_functions")),
    _n_speeds(_speeds.size()),
    _Rp_functions(_n_speeds),
    _eff_functions(_n_speeds),
    _compressor_name(getParam<std::string>("compressor_name")),
    _omega(coupledScalarValue("omega"))
{
  if (_n_speeds != _Rp_function_names.size() || _n_speeds != _eff_function_names.size())
    mooseError("The number of entries of speeds needs to equal the number of entries of "
               "Rp_functions and eff_functions");

  // Store functions and check to make sure there is no self-reference.
  for (unsigned int i = 0; i < _n_speeds; i++)
  {
    if (_Rp_function_names[i] == name() || _eff_function_names[i] == name())
      mooseError(name(), ": This function cannot use its own name in the 'functions' parameter.");

    _Rp_functions[i] = &getFunctionByName(_Rp_function_names[i]);
    _eff_functions[i] = &getFunctionByName(_eff_function_names[i]);
  }
  unsigned int n_jct_vars = _scalar_variable_names.size();
  _torque_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
  _torque_jacobian_scalar_vars.zero();
  _moi_jacobian_scalar_vars.resize(_n_shaft_eq, n_jct_vars);
  _moi_jacobian_scalar_vars.zero();
}

void
ShaftConnectedCompressor1PhaseUserObject::initialSetup()
{
  VolumeJunctionBaseUserObject::initialSetup();

  ShaftConnectableUserObjectInterface::setupConnections(
      VolumeJunctionBaseUserObject::_n_connections, VolumeJunctionBaseUserObject::_n_flux_eq);

  _residual_jacobian_omega_var.resize(_n_scalar_eq);
  for (auto && v : _residual_jacobian_omega_var)
    v.resize(1, ShaftConnectableUserObjectInterface::_n_shaft_eq);
}

void
ShaftConnectedCompressor1PhaseUserObject::initialize()
{
  VolumeJunctionBaseUserObject::initialize();
  ShaftConnectableUserObjectInterface::initialize();
  for (unsigned int i = 0; i < _n_scalar_eq; i++)
    _residual_jacobian_omega_var[i].zero();

  _isentropic_torque = 0;
  _dissipation_torque = 0;
  _friction_torque = 0;
  _delta_p = 0;
}

void
ShaftConnectedCompressor1PhaseUserObject::execute()
{
  VolumeJunctionBaseUserObject::storeConnectionData();
  ShaftConnectableUserObjectInterface::setConnectionData(
      VolumeJunctionBaseUserObject::_phi_face_values,
      VolumeJunctionBaseUserObject::_flow_channel_dofs);

  const unsigned int c = getBoundaryIDIndex();

  computeFluxesAndResiduals(c);
}

void
ShaftConnectedCompressor1PhaseUserObject::computeFluxesAndResiduals(const unsigned int & c)
{
  VolumeJunction1PhaseUserObject::computeFluxesAndResiduals(c);

  // inlet c=0 established in component
  if (c == 0)
  {
    const Real rhoA_in = _rhoA[0];
    const Real rhouA_in = _rhouA[0];
    const Real rhoEA_in = _rhoEA[0];

    Real e_in, dein_drhoA, dein_drhouA, dein_drhoEA;
    THM::e_from_arhoA_arhouA_arhoEA(
        rhoA_in, rhouA_in, rhoEA_in, e_in, dein_drhoA, dein_drhouA, dein_drhoEA);

    const Real v_in = _A[0] / rhoA_in;
    const Real dvin_drhoA = -_A[0] / rhoA_in / rhoA_in;

    Real p_in, dpin_dv, dpin_de;
    _fp.p_from_v_e(v_in, e_in, p_in, dpin_dv, dpin_de);
    const Real dpin_drhoA = dpin_dv * dvin_drhoA + dpin_de * dein_drhoA;
    const Real dpin_drhouA = dpin_de * dein_drhouA;
    const Real dpin_drhoEA = dpin_de * dein_drhoEA;

    const Real vel_in = rhouA_in / rhoA_in;
    const Real dvel_in_drhoA = -(rhouA_in / rhoA_in / rhoA_in);
    const Real dvel_in_drhouA = 1 / rhoA_in;

    // static entropy is equal to stagnation entropy by definition of the stagnation state
    Real s_in, ds_dv, ds_de;
    _fp.s_from_v_e(v_in, e_in, s_in, ds_dv, ds_de);
    const Real ds_drhoA = ds_dv * dvin_drhoA + ds_de * dein_drhoA;
    const Real ds_drhouA = ds_de * dein_drhouA;
    const Real ds_drhoEA = ds_de * dein_drhoEA;

    const Real s_out = s_in;

    // isentropic: dH/m = Vdp/m
    // h0, T0, and c0 are constant for adiabatic flows

    const Real h0_in = e_in + p_in * v_in + 0.5 * vel_in * vel_in;
    const Real dh0_drhoA =
        dein_drhoA + dpin_drhoA * v_in + p_in * dvin_drhoA + vel_in * dvel_in_drhoA;
    const Real dh0_drhouA = dein_drhouA + dpin_drhouA * v_in + vel_in * dvel_in_drhouA;
    const Real dh0_drhoEA = dein_drhoEA + dpin_drhoEA * v_in;

    Real p0_in, dp0_dh0, dp0_ds;
    _fp.p_from_h_s(h0_in, s_in, p0_in, dp0_dh0, dp0_ds);
    const Real dp0_drhoA = dp0_dh0 * dh0_drhoA + dp0_ds * ds_drhoA;
    const Real dp0_drhouA = dp0_dh0 * dh0_drhouA + dp0_ds * ds_drhouA;
    const Real dp0_drhoEA = dp0_dh0 * dh0_drhoEA + dp0_ds * ds_drhoEA;

    Real rho0_in, drho0_dp0, drho0_ds;
    _fp.rho_from_p_s(p0_in, s_in, rho0_in, drho0_dp0, drho0_ds);
    const Real drho0_drhoA = drho0_dp0 * dp0_drhoA + drho0_ds * ds_drhoA;
    const Real drho0_drhouA = drho0_dp0 * dp0_drhouA + drho0_ds * ds_drhouA;
    const Real drho0_drhoEA = drho0_dp0 * dp0_drhoEA + drho0_ds * ds_drhoEA;

    const Real v0_in = 1.0 / rho0_in;
    const Real dv0_drho0 = -1.0 / rho0_in / rho0_in;
    const Real dv0_drhoA = dv0_drho0 * drho0_drhoA;
    const Real dv0_drhouA = dv0_drho0 * drho0_drhouA;
    const Real dv0_drhoEA = dv0_drho0 * drho0_drhoEA;

    Real e0_in, de0_dp0, de0_drho0;
    _fp.e_from_p_rho(p0_in, rho0_in, e0_in, de0_dp0, de0_drho0);
    const Real de0_drhoA = de0_dp0 * dp0_drhoA + de0_drho0 * drho0_drhoA;
    const Real de0_drhouA = de0_dp0 * dp0_drhouA + de0_drho0 * drho0_drhouA;
    const Real de0_drhoEA = de0_dp0 * dp0_drhoEA + de0_drho0 * drho0_drhoEA;

    Real c0_in, dc0_dv0, dc0_de0;
    _fp.c_from_v_e(v0_in, e0_in, c0_in, dc0_dv0, dc0_de0);
    const Real dc0_drhoA = dc0_dv0 * dv0_drhoA + dc0_de0 * de0_drhoA;
    const Real dc0_drhouA = dc0_dv0 * dv0_drhouA + dc0_de0 * de0_drhouA;
    const Real dc0_drhoEA = dc0_dv0 * dv0_drhoEA + dc0_de0 * de0_drhoEA;

    const Real flow_A = rhouA_in * _rho0_rated * _c0_rated;
    const Real dA_drhouA = _rho0_rated * _c0_rated;

    const Real flow_B = _mdot_rated * rho0_in * c0_in;
    const Real dB_drhoA = _mdot_rated * (drho0_drhoA * c0_in + rho0_in * dc0_drhoA);
    const Real dB_drhouA = _mdot_rated * (drho0_drhouA * c0_in + rho0_in * dc0_drhouA);
    const Real dB_drhoEA = _mdot_rated * (drho0_drhoEA * c0_in + rho0_in * dc0_drhoEA);

    const Real flow_rel_corr = flow_A / flow_B;
    const Real dflow_drhoA = -flow_A * dB_drhoA / flow_B / flow_B;
    const Real dflow_drhouA = (dA_drhouA * flow_B - flow_A * dB_drhouA) / (flow_B * flow_B);
    const Real dflow_drhoEA = -flow_A * dB_drhoEA / flow_B / flow_B;

    const Real speed_rel_corr = (_omega[0] / _omega_rated) * (_c0_rated / c0_in);
    const Real dspeed_dc0 = -(_omega[0] / _omega_rated) * (_c0_rated / c0_in / c0_in);
    const Real dspeed_drhoA = dspeed_dc0 * dc0_drhoA;
    const Real dspeed_drhouA = dspeed_dc0 * dc0_drhouA;
    const Real dspeed_drhoEA = dspeed_dc0 * dc0_drhoEA;
    const Real dspeed_domega = (1.0 / _omega_rated) * (_c0_rated / c0_in);

    if (speed_rel_corr < _speeds[0])
      mooseError(_compressor_name,
                 ": is attempting to operate at speeds less than allowed by functions ",
                 _Rp_function_names[0],
                 " and ",
                 _eff_function_names[0],
                 ".");

    if (speed_rel_corr > _speeds.back())
      mooseError(_compressor_name,
                 ": is attempting to operate at speeds greater than allowed by functions ",
                 _Rp_function_names.back(),
                 " and ",
                 _eff_function_names.back(),
                 ".");

    auto x1_iter = std::lower_bound(_speeds.begin(), _speeds.end(), speed_rel_corr);
    auto x2_iter = std::upper_bound(_speeds.begin(), _speeds.end(), speed_rel_corr);

    auto x1 = (x1_iter - _speeds.begin()) - 1; // _speeds index for entry <= speed_rel_corr
    auto x2 = (x2_iter - _speeds.begin());     // _speeds index for entry > speed_rel_corr
    const Real x1_spd = _speeds[x1];
    const Real x2_spd = _speeds[x2];

    const Real y1_Rp = _Rp_functions[x1]->value(flow_rel_corr, Point());
    const Real dy1Rp_dflow = _Rp_functions[x1]->timeDerivative(flow_rel_corr, Point());
    const Real dy1Rp_drhoA = dy1Rp_dflow * dflow_drhoA;
    const Real dy1Rp_drhouA = dy1Rp_dflow * dflow_drhouA;
    const Real dy1Rp_drhoEA = dy1Rp_dflow * dflow_drhoEA;

    const Real y2_Rp = _Rp_functions[x2]->value(flow_rel_corr, Point());
    const Real dy2Rp_dflow = _Rp_functions[x2]->timeDerivative(flow_rel_corr, Point());
    const Real dy2Rp_drhoA = dy2Rp_dflow * dflow_drhoA;
    const Real dy2Rp_drhouA = dy2Rp_dflow * dflow_drhouA;
    const Real dy2Rp_drhoEA = dy2Rp_dflow * dflow_drhoEA;

    const Real Rp_m = (y2_Rp - y1_Rp) / (x2_spd - x1_spd);
    const Real dRpm_drhoA = (dy2Rp_drhoA - dy1Rp_drhoA) / (x2_spd - x1_spd);
    const Real dRpm_drhouA = (dy2Rp_drhouA - dy1Rp_drhouA) / (x2_spd - x1_spd);
    const Real dRpm_drhoEA = (dy2Rp_drhoEA - dy1Rp_drhoEA) / (x2_spd - x1_spd);

    const Real Rp = y1_Rp + (speed_rel_corr - x1_spd) * Rp_m;
    const Real dRp_drhoA =
        dy1Rp_drhoA + (speed_rel_corr - x1_spd) * dRpm_drhoA + dspeed_drhoA * Rp_m;
    const Real dRp_drhouA =
        dy1Rp_drhouA + (speed_rel_corr - x1_spd) * dRpm_drhouA + dspeed_drhouA * Rp_m;
    const Real dRp_drhoEA =
        dy1Rp_drhoEA + (speed_rel_corr - x1_spd) * dRpm_drhoEA + dspeed_drhoEA * Rp_m;
    const Real dRp_domega = dspeed_domega * Rp_m;

    const Real y1_eff = _eff_functions[x1]->value(flow_rel_corr, Point());
    const Real dy1eff_dflow = _eff_functions[x1]->timeDerivative(flow_rel_corr, Point());
    const Real dy1eff_drhoA = dy1eff_dflow * dflow_drhoA;
    const Real dy1eff_drhouA = dy1eff_dflow * dflow_drhouA;
    const Real dy1eff_drhoEA = dy1eff_dflow * dflow_drhoEA;

    const Real y2_eff = _eff_functions[x2]->value(flow_rel_corr, Point());
    const Real dy2eff_dflow = _eff_functions[x2]->timeDerivative(flow_rel_corr, Point());
    const Real dy2eff_drhoA = dy2eff_dflow * dflow_drhoA;
    const Real dy2eff_drhouA = dy2eff_dflow * dflow_drhouA;
    const Real dy2eff_drhoEA = dy2eff_dflow * dflow_drhoEA;

    const Real eff_m = (y2_eff - y1_eff) / (x2_spd - x1_spd);
    const Real deffm_drhoA = (dy2eff_drhoA - dy1eff_drhoA) / (x2_spd - x1_spd);
    const Real deffm_drhouA = (dy2eff_drhouA - dy1eff_drhouA) / (x2_spd - x1_spd);
    const Real deffm_drhoEA = (dy2eff_drhoEA - dy1eff_drhoEA) / (x2_spd - x1_spd);

    const Real eff = y1_eff + (speed_rel_corr - x1_spd) * eff_m;

    const Real deff_drhoA =
        dy1eff_drhoA + (speed_rel_corr - x1_spd) * deffm_drhoA + dspeed_drhoA * eff_m;
    const Real deff_drhouA =
        dy1eff_drhouA + (speed_rel_corr - x1_spd) * deffm_drhouA + dspeed_drhouA * eff_m;
    const Real deff_drhoEA =
        dy1eff_drhoEA + (speed_rel_corr - x1_spd) * deffm_drhoEA + dspeed_drhoEA * eff_m;
    const Real deff_domega = dspeed_domega * eff_m;

    const Real p0_out = p0_in * Rp;
    const Real dp0out_drhoA = dp0_drhoA * Rp + p0_in * dRp_drhoA;
    const Real dp0out_drhouA = dp0_drhouA * Rp + p0_in * dRp_drhouA;
    const Real dp0out_drhoEA = dp0_drhoEA * Rp + p0_in * dRp_drhoEA;
    const Real dp0out_domega = p0_in * dRp_domega;

    Real rho0_out_isen, drho_out_s_dp0, drho_out_s_ds;
    _fp.rho_from_p_s(p0_out, s_out, rho0_out_isen, drho_out_s_dp0, drho_out_s_ds);
    const Real drho_out_s_drhoA = drho_out_s_dp0 * dp0out_drhoA + drho_out_s_ds * ds_drhoA;
    const Real drho_out_s_drhouA = drho_out_s_dp0 * dp0out_drhouA + drho_out_s_ds * ds_drhouA;
    const Real drho_out_s_drhoEA = drho_out_s_dp0 * dp0out_drhoEA + drho_out_s_ds * ds_drhoEA;
    const Real drho_out_s_domega = drho_out_s_dp0 * dp0out_domega;

    Real e0_out_isen, de0_out_s_dp0, de0_out_s_drho0;
    _fp.e_from_p_rho(p0_out, rho0_out_isen, e0_out_isen, de0_out_s_dp0, de0_out_s_drho0);
    const Real de0_out_s_drhoA = de0_out_s_dp0 * dp0out_drhoA + de0_out_s_drho0 * drho_out_s_drhoA;
    const Real de0_out_s_drhouA =
        de0_out_s_dp0 * dp0out_drhouA + de0_out_s_drho0 * drho_out_s_drhouA;
    const Real de0_out_s_drhoEA =
        de0_out_s_dp0 * dp0out_drhoEA + de0_out_s_drho0 * drho_out_s_drhoEA;
    const Real de0_out_s_domega =
        de0_out_s_dp0 * dp0out_domega + de0_out_s_drho0 * drho_out_s_domega;

    _delta_p = p0_in * (Rp - 1.0);
    const Real ddeltap_drhoA = dp0_drhoA * (Rp - 1.0) + p0_in * dRp_drhoA;
    const Real ddeltap_drhouA = dp0_drhouA * (Rp - 1.0) + p0_in * dRp_drhouA;
    const Real ddeltap_drhoEA = dp0_drhoEA * (Rp - 1.0) + p0_in * dRp_drhoEA;
    const Real ddeltap_domega = p0_in * dRp_domega;

    Real h0_out_isen, dh_de, dh_dp, dh_drho;
    THM::h_from_e_p_rho(e0_out_isen, p0_out, rho0_out_isen, h0_out_isen, dh_de, dh_dp, dh_drho);
    const Real dh0out_s_drhoA =
        dh_de * de0_out_s_drhoA + dh_dp * dp0out_drhoA + dh_drho * drho_out_s_drhoA;
    const Real dh0out_s_drhouA =
        dh_de * de0_out_s_drhouA + dh_dp * dp0out_drhouA + dh_drho * drho_out_s_drhouA;
    const Real dh0out_s_drhoEA =
        dh_de * de0_out_s_drhoEA + dh_dp * dp0out_drhoEA + dh_drho * drho_out_s_drhoEA;
    const Real dh0out_s_domega =
        dh_de * de0_out_s_domega + dh_dp * dp0out_domega + dh_drho * drho_out_s_domega;

    _isentropic_torque = -(rhouA_in / _omega[0]) * (h0_out_isen - h0_in); // tau_isen
    const Real dtau_isen_drhoA = -(rhouA_in / _omega[0]) * (dh0out_s_drhoA - dh0_drhoA);
    const Real dtau_isen_drhouA = -(1 / _omega[0]) * (h0_out_isen - h0_in) -
                                  (rhouA_in / _omega[0]) * (dh0out_s_drhouA - dh0_drhouA);
    const Real dtau_isen_drhoEA = -(rhouA_in / _omega[0]) * (dh0out_s_drhoEA - dh0_drhoEA);
    const Real dtau_isen_domega = (rhouA_in / _omega[0] / _omega[0]) * (h0_out_isen - h0_in) -
                                  (rhouA_in / _omega[0]) * dh0out_s_domega;

    // f(x) = g(x) / h(x)
    // f'(x) = (g'(x)*h(x) - g(x)*h'(x)) / (h(x)*h(x))
    const Real g_x = h0_out_isen - h0_in + h0_in * eff;
    const Real dgx_drhoA = dh0out_s_drhoA - dh0_drhoA + dh0_drhoA * eff + h0_in * deff_drhoA;
    const Real dgx_drhouA = dh0out_s_drhouA - dh0_drhouA + dh0_drhouA * eff + h0_in * deff_drhouA;
    const Real dgx_drhoEA = dh0out_s_drhoEA - dh0_drhoEA + dh0_drhoEA * eff + h0_in * deff_drhoEA;
    const Real dgx_domega = dh0out_s_domega + h0_in * deff_domega;

    const Real h0_out = g_x / eff;
    const Real dh0out_drhoA = (dgx_drhoA * eff - g_x * deff_drhoA) / (eff * eff);
    const Real dh0out_drhouA = (dgx_drhouA * eff - g_x * deff_drhouA) / (eff * eff);
    const Real dh0out_drhoEA = (dgx_drhoEA * eff - g_x * deff_drhoEA) / (eff * eff);
    const Real dh0out_domega = (dgx_domega * eff - g_x * deff_domega) / (eff * eff);

    _dissipation_torque = -(rhouA_in / _omega[0]) * (h0_out - h0_out_isen);
    const Real dtau_diss_drhoA = -(rhouA_in / _omega[0]) * (dh0out_drhoA - dh0out_s_drhoA);
    const Real dtau_diss_drhouA = -(rhouA_in / _omega[0]) * (dh0out_drhouA - dh0out_s_drhouA) -
                                  (1 / _omega[0]) * (h0_out - h0_out_isen);
    const Real dtau_diss_drhoEA = -(rhouA_in / _omega[0]) * (dh0out_drhoEA - dh0out_s_drhoEA);
    const Real dtau_diss_domega = (rhouA_in / _omega[0] / _omega[0]) * (h0_out - h0_out_isen) -
                                  (rhouA_in / _omega[0]) * (dh0out_domega - dh0out_s_domega);

    Real alpha = _omega[0] / _omega_rated;
    Real dalpha_domega = 1 / _omega_rated;

    Real dmoi_domega;
    if (alpha < _speed_cr_I)
    {
      _moment_of_inertia += _inertia_const;
      dmoi_domega = 0;
    }
    else
    {
      _moment_of_inertia += _inertia_coeff[0] + _inertia_coeff[1] * std::abs(alpha) +
                            _inertia_coeff[2] * alpha * alpha +
                            _inertia_coeff[3] * std::abs(alpha * alpha * alpha);
      dmoi_domega = _inertia_coeff[1] * dalpha_domega +
                    2 * _inertia_coeff[2] * dalpha_domega * alpha +
                    3 * _inertia_coeff[3] * alpha * alpha * dalpha_domega;
    }

    // friction torque
    Real sign;
    if (_omega[0] >= 0)
      sign = -1;
    else
      sign = 1;
    Real dtau_fr_domega;
    if (alpha < _speed_cr_fr)
    {
      _friction_torque = sign * _tau_fr_const;
      dtau_fr_domega = 0;
    }
    else
    {
      _friction_torque = sign * (_tau_fr_coeff[0] + _tau_fr_coeff[1] * std::abs(alpha) +
                                 _tau_fr_coeff[2] * alpha * alpha +
                                 _tau_fr_coeff[3] * std::abs(alpha * alpha * alpha));
      dtau_fr_domega =
          sign * (_tau_fr_coeff[1] * dalpha_domega + 2 * _tau_fr_coeff[2] * dalpha_domega * alpha +
                  3 * _tau_fr_coeff[3] * alpha * alpha * dalpha_domega);
    }

    _torque += _isentropic_torque + _dissipation_torque + _friction_torque;

    const Real dtorque_drhoA = dtau_isen_drhoA + dtau_diss_drhoA;
    const Real dtorque_drhouA = dtau_isen_drhouA + dtau_diss_drhouA;
    const Real dtorque_drhoEA = dtau_isen_drhoEA + dtau_diss_drhoEA;
    const Real dtorque_domega = dtau_isen_domega + dtau_diss_domega + dtau_fr_domega;

    // compute momentum and energy source terms
    // a negative torque value results in a positive S_energy
    const Real S_energy = -_torque * _omega[0];

    const Real dS_energy_drhoA = -dtorque_drhoA * _omega[0];
    const Real dS_energy_drhouA = -dtorque_drhouA * _omega[0];
    const Real dS_energy_drhoEA = -dtorque_drhoEA * _omega[0];
    const Real dS_energy_domega = -_torque - dtorque_domega * _omega[0];

    const RealVectorValue S_momentum = _delta_p * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhoA = ddeltap_drhoA * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhouA = ddeltap_drhouA * _A_ref * _di_out;
    const RealVectorValue dS_momentum_drhoEA = ddeltap_drhoEA * _A_ref * _di_out;
    const RealVectorValue dS_momentum_domega = ddeltap_domega * _A_ref * _di_out;

    _residual[VolumeJunction1Phase::RHOEV_INDEX] -= S_energy;

    _residual[VolumeJunction1Phase::RHOUV_INDEX] -= S_momentum(0);
    _residual[VolumeJunction1Phase::RHOVV_INDEX] -= S_momentum(1);
    _residual[VolumeJunction1Phase::RHOWV_INDEX] -= S_momentum(2);

    _residual_jacobian_omega_var[VolumeJunction1Phase::RHOEV_INDEX](0, 0) -= dS_energy_domega;

    const RealVectorValue rhovelV_index(VolumeJunction1Phase::RHOUV_INDEX,
                                        VolumeJunction1Phase::RHOVV_INDEX,
                                        VolumeJunction1Phase::RHOWV_INDEX);
    for (unsigned int i = 0; i < LIBMESH_DIM; i++)
    {
      _residual_jacobian_omega_var[rhovelV_index(i)](0, 0) -= dS_momentum_domega(i);
    }

    {
      DenseMatrix<Real> jac(_n_scalar_eq, _n_flux_eq);
      jac(VolumeJunction1Phase::RHOEV_INDEX, 0) = -dS_energy_drhoA;
      jac(VolumeJunction1Phase::RHOEV_INDEX, 1) = -dS_energy_drhouA;
      jac(VolumeJunction1Phase::RHOEV_INDEX, 2) = -dS_energy_drhoEA;
      jac(VolumeJunction1Phase::RHOUV_INDEX, 0) = -dS_momentum_drhoA(0);
      jac(VolumeJunction1Phase::RHOUV_INDEX, 1) = -dS_momentum_drhouA(0);
      jac(VolumeJunction1Phase::RHOUV_INDEX, 2) = -dS_momentum_drhoEA(0);
      jac(VolumeJunction1Phase::RHOVV_INDEX, 0) = -dS_momentum_drhoA(1);
      jac(VolumeJunction1Phase::RHOVV_INDEX, 1) = -dS_momentum_drhouA(1);
      jac(VolumeJunction1Phase::RHOVV_INDEX, 2) = -dS_momentum_drhoEA(1);
      jac(VolumeJunction1Phase::RHOWV_INDEX, 0) = -dS_momentum_drhoA(2);
      jac(VolumeJunction1Phase::RHOWV_INDEX, 1) = -dS_momentum_drhouA(2);
      jac(VolumeJunction1Phase::RHOWV_INDEX, 2) = -dS_momentum_drhoEA(2);
      for (unsigned int i = 0; i < _n_scalar_eq; i++)
      {
        unsigned int jk = 0;
        for (unsigned int j = 0; j < _n_flux_eq; j++)
        {
          for (unsigned int k = 0; k < VolumeJunctionBaseUserObject::_phi_face_values[c][j].size();
               k++)
          {
            _residual_jacobian_flow_channel_vars[c][i](0, jk) +=
                jac(i, j) * VolumeJunctionBaseUserObject::_phi_face_values[c][j][k];
            jk++;
          }
        }
      }
    }

    _torque_jacobian_omega_var(0, 0) = dtorque_domega;
    _moi_jacobian_omega_var(0, 0) = dmoi_domega;

    // dtau_dUi (i.e. wrt flow variables)
    {
      DenseMatrix<Real> jac(1, THM3Eqn::N_EQ);
      jac.zero();
      jac(0, THM3Eqn::EQ_MASS) = dtorque_drhoA;
      jac(0, THM3Eqn::EQ_MOMENTUM) = dtorque_drhouA;
      jac(0, THM3Eqn::EQ_ENERGY) = dtorque_drhoEA;
      computeTorqueScalarJacobianWRTFlowDofs(jac, c);
    }
  }
  else
  {
    // dtau_dUi (i.e. wrt flow variables)
    {
      DenseMatrix<Real> jac(1, THM3Eqn::N_EQ);
      jac.zero();
      computeTorqueScalarJacobianWRTFlowDofs(jac, c);
    }
  }

  // dmoi_dUi (i.e. wrt flow variables)
  {
    DenseMatrix<Real> jac(1, THM3Eqn::N_EQ);
    jac.zero();
    computeMomentOfInertiaScalarJacobianWRTFlowDofs(jac, c);
  }
}

Real
ShaftConnectedCompressor1PhaseUserObject::getIsentropicTorque() const
{
  return _isentropic_torque;
}

Real
ShaftConnectedCompressor1PhaseUserObject::getDissipationTorque() const
{
  return _dissipation_torque;
}

Real
ShaftConnectedCompressor1PhaseUserObject::getFrictionTorque() const
{
  return _friction_torque;
}

Real
ShaftConnectedCompressor1PhaseUserObject::getCompressorDeltaP() const
{
  return _delta_p;
}

void
ShaftConnectedCompressor1PhaseUserObject::finalize()
{
  VolumeJunctionBaseUserObject::finalize();

  ShaftConnectableUserObjectInterface::setupJunctionData(
      VolumeJunctionBaseUserObject::_scalar_dofs);
  ShaftConnectableUserObjectInterface::setOmegaDofs(getScalarVar("omega", 0));
}

void
ShaftConnectedCompressor1PhaseUserObject::threadJoin(const UserObject & uo)
{
  VolumeJunctionBaseUserObject::threadJoin(uo);
  ShaftConnectableUserObjectInterface::threadJoin(uo);

  const ShaftConnectedCompressor1PhaseUserObject & scpuo =
      dynamic_cast<const ShaftConnectedCompressor1PhaseUserObject &>(uo);
  _isentropic_torque += scpuo._isentropic_torque;
  _dissipation_torque += scpuo._dissipation_torque;
  _friction_torque += scpuo._friction_torque;
  _delta_p += scpuo._delta_p;

  _moi_jacobian_omega_var(0, 0) += scpuo._moi_jacobian_omega_var(0, 0);

  _torque_jacobian_omega_var(0, 0) += scpuo._torque_jacobian_omega_var(0, 0);

  for (unsigned int i = 0; i < _n_scalar_eq; i++)
  {
    for (unsigned int j = 0; j < _omega_dof.size(); j++)
    {
      _residual_jacobian_omega_var[i](0, j) += scpuo._residual_jacobian_omega_var[i](0, j);
    }
  }
}

void
ShaftConnectedCompressor1PhaseUserObject::getScalarEquationJacobianData(
    const unsigned int & equation_index,
    DenseMatrix<Real> & jacobian_block,
    std::vector<dof_id_type> & dofs_i,
    std::vector<dof_id_type> & dofs_j) const
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

  dofs_i = {VolumeJunctionBaseUserObject::_scalar_dofs[equation_index]};

  // number of DoFs for the dof_j array
  unsigned int n_dofs = VolumeJunctionBaseUserObject::_scalar_dofs.size() + _omega_dof.size();
  for (unsigned int c = 0; c < VolumeJunctionBaseUserObject::_n_connections; c++)
    n_dofs += _residual_jacobian_flow_channel_vars[c][equation_index].n();

  jacobian_block.resize(1, n_dofs);
  dofs_j.resize(n_dofs);

  unsigned int k = 0;
  // Store Jacobian entries w.r.t. scalar variables
  for (unsigned int j = 0; j < VolumeJunctionBaseUserObject::_scalar_dofs.size(); j++, k++)
  {
    jacobian_block(0, k) = _residual_jacobian_scalar_vars[equation_index](0, j);
    dofs_j[k] = VolumeJunctionBaseUserObject::_scalar_dofs[j];
  }
  // Store Jacobian entries w.r.t. shaft variables
  for (unsigned int j = 0; j < _omega_dof.size(); j++, k++)
  {
    jacobian_block(0, k) = _residual_jacobian_omega_var[equation_index](0, j);
    dofs_j[k] = _omega_dof[j];
  }
  // Store Jacobian entries w.r.t. flow variables
  for (unsigned int c = 0; c < VolumeJunctionBaseUserObject::_n_connections; c++)
  {
    for (unsigned int j = 0; j < _residual_jacobian_flow_channel_vars[c][equation_index].n();
         j++, k++)
    {
      jacobian_block(0, k) = _residual_jacobian_flow_channel_vars[c][equation_index](0, j);
      dofs_j[k] = VolumeJunctionBaseUserObject::_flow_channel_dofs[c][j];
    }
  }
}
