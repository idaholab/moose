#include "StiffenedGasTwoPhaseFluidProperties.h"
#include "StiffenedGasFluidProperties.h"

registerMooseObject("FluidPropertiesApp", StiffenedGasTwoPhaseFluidProperties);

InputParameters
StiffenedGasTwoPhaseFluidProperties::validParams()
{
  InputParameters params = TwoPhaseFluidProperties::validParams();
  params += NaNInterface::validParams();

  params.addParam<Real>("gamma_liquid", 2.35, "Liquid heat capacity ratio");
  params.addParam<Real>("cv_liquid", 1816.0, "Liquid isochoric specific heat capacity");
  params.addParam<Real>("q_liquid", -1.167e6, "Liquid reference specific internal energy");
  params.addParam<Real>("p_inf_liquid", 1.0e9, "Liquid stiffness pressure");
  params.addParam<Real>("q_prime_liquid", 0, "Liquid reference specific entropy");
  params.addParam<Real>("k_liquid", 0.5, "Liquid thermal conductivity");
  params.addParam<Real>("mu_liquid", 281.8e-6, "Liquid dynamic viscosity");
  params.addParam<Real>("M_liquid", 0.01801488, "Liquid molar mass");

  params.addParam<Real>("gamma_vapor", 1.43, "Vapor heat capacity ratio");
  params.addParam<Real>("cv_vapor", 1040.0, "Vapor isochoric specific heat capacity");
  params.addParam<Real>("q_vapor", 2.03e6, "Vapor reference specific internal energy");
  params.addParam<Real>("p_inf_vapor", 0.0, "Vapor stiffness pressure");
  params.addParam<Real>("q_prime_vapor", -2.3e4, "Vapor reference specific entropy");
  params.addParam<Real>("k_vapor", 0.026, "Vapor thermal conductivity");
  params.addParam<Real>("mu_vapor", 134.4e-7, "Vapor dynamic viscosity");
  params.addParam<Real>("M_vapor", 0.01801488, "Vapor molar mass");

  params.addParam<Real>("T_c", 647.096, "Critical temperature [K]");
  params.addParam<Real>("p_c", 22.09e6, "Critical pressure [Pa]");
  params.addParam<Real>("rho_c", 322.0, "Critical density [kg/m^3]");
  params.addParam<Real>("e_c", 2702979.84310559, "Critical specific internal energy [J/kg]");
  params.addParam<Real>("T_triple", 273.16, "Triple-point temperature [K]");
  params.addParam<Real>("L_fusion", 0.334, "Latent heat of fusion [J/kg]");

  params.addParam<Real>(
      "sigma_A", 0.2358, "'A' constant used in surface tension correlation [N/m]");
  params.addParam<Real>("sigma_B", 1.256, "'B' constant used in surface tension correlation");
  params.addParam<Real>("sigma_C", 0.625, "'C' constant used in surface tension correlation");

  // Default values correspond to water, from the freezing point to critical point, with 1 K
  // increments
  params.addParam<Real>("T_sat_min", 274.0, "Minimum temperature value in saturation curve [K]");
  params.addParam<Real>("T_sat_max", 647.0, "Maximum temperature value in saturation curve [K]");
  params.addParam<Real>(
      "p_sat_guess", 611.0, "Initial guess for saturation pressure Newton solve [Pa]");
  params.addParam<unsigned int>(
      "n_sat_samples", 374, "Number of samples to take in saturation curve");

  params.addClassDescription("Two-phase stiffened gas fluid properties");

  return params;
}

StiffenedGasTwoPhaseFluidProperties::StiffenedGasTwoPhaseFluidProperties(
    const InputParameters & parameters)
  : TwoPhaseFluidProperties(parameters),
    NaNInterface(this),

    _gamma_liquid(getParam<Real>("gamma_liquid")),
    _cv_liquid(getParam<Real>("cv_liquid")),
    _cp_liquid(_gamma_liquid * _cv_liquid),
    _q_liquid(getParam<Real>("q_liquid")),
    _p_inf_liquid(getParam<Real>("p_inf_liquid")),
    _q_prime_liquid(getParam<Real>("q_prime_liquid")),

    _gamma_vapor(getParam<Real>("gamma_vapor")),
    _cv_vapor(getParam<Real>("cv_vapor")),
    _cp_vapor(_gamma_vapor * _cv_vapor),
    _q_vapor(getParam<Real>("q_vapor")),
    _p_inf_vapor(getParam<Real>("p_inf_vapor")),
    _q_prime_vapor(getParam<Real>("q_prime_vapor")),

    _T_c(getParam<Real>("T_c")),
    _p_c(getParam<Real>("p_c")),
    _T_triple(getParam<Real>("T_triple")),
    _L_fusion(getParam<Real>("L_fusion")),

    _sigma_A(getParam<Real>("sigma_A")),
    _sigma_B(getParam<Real>("sigma_B")),
    _sigma_C(getParam<Real>("sigma_C")),

    _T_sat_min(getParam<Real>("T_sat_min")),
    _T_sat_max(getParam<Real>("T_sat_max")),
    _p_sat_guess(getParam<Real>("p_sat_guess")),
    _n_sat_samples(getParam<unsigned int>("n_sat_samples")),
    _dT_sat((_T_sat_max - _T_sat_min) / (_n_sat_samples - 1)),

    _A((_cp_liquid - _cp_vapor + _q_prime_vapor - _q_prime_liquid) / (_cp_vapor - _cv_vapor)),
    _B((_q_liquid - _q_vapor) / (_cp_vapor - _cv_vapor)),
    _C((_cp_vapor - _cp_liquid) / (_cp_vapor - _cv_vapor)),
    _D((_cp_liquid - _cv_liquid) / (_cp_vapor - _cv_vapor)),

    _newton_tol(1.0e-8),
    _newton_max_iter(200)
{
  if (_tid == 0)
  {
    std::string class_name = "StiffenedGasFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<MooseEnum>("emit_on_nan") = getParam<MooseEnum>("emit_on_nan");
    params.set<bool>("allow_nonphysical_states") = false;
    params.set<Real>("gamma") = _gamma_liquid;
    params.set<Real>("cv") = _cv_liquid;
    params.set<Real>("q") = _q_liquid;
    params.set<Real>("p_inf") = _p_inf_liquid;
    params.set<Real>("q_prime") = _q_prime_liquid;
    params.set<Real>("k") = getParam<Real>("k_liquid");
    params.set<Real>("mu") = getParam<Real>("mu_liquid");
    params.set<Real>("M") = getParam<Real>("M_liquid");
    _fe_problem.addUserObject(class_name, _liquid_name, params);
  }
  _fp_liquid = &_fe_problem.getUserObject<SinglePhaseFluidProperties>(_liquid_name, _tid);

  if (_tid == 0)
  {
    std::string class_name = "StiffenedGasFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<MooseEnum>("emit_on_nan") = getParam<MooseEnum>("emit_on_nan");
    params.set<bool>("allow_nonphysical_states") = false;
    params.set<Real>("gamma") = _gamma_vapor;
    params.set<Real>("cv") = _cv_vapor;
    params.set<Real>("q") = _q_vapor;
    params.set<Real>("p_inf") = _p_inf_vapor;
    params.set<Real>("q_prime") = _q_prime_vapor;
    params.set<Real>("k") = getParam<Real>("k_vapor");
    params.set<Real>("mu") = getParam<Real>("mu_vapor");
    params.set<Real>("M") = getParam<Real>("M_vapor");
    params.set<Real>("T_c") = getParam<Real>("T_c");
    params.set<Real>("rho_c") = getParam<Real>("rho_c");
    params.set<Real>("e_c") = getParam<Real>("e_c");
    _fe_problem.addUserObject(class_name, _vapor_name, params);
  }
  _fp_vapor = &_fe_problem.getUserObject<SinglePhaseFluidProperties>(_vapor_name, _tid);

  // calculate the saturation curve p(T) and store the data in two vectors for re-use
  {
    _T_vec.resize(_n_sat_samples);
    _p_sat_vec.resize(_n_sat_samples);

    // loop over sample temperatures, starting with the minimum
    Real T = _T_sat_min;
    for (unsigned int i = 0; i < _n_sat_samples; i++)
    {
      _T_vec[i] = T;
      _p_sat_vec[i] = compute_p_sat(T);

      // increment sample temperature
      T += _dT_sat;
    }
  }

  _ipol_pressure.setData(_T_vec, _p_sat_vec);
  _ipol_temp.setData(_p_sat_vec, _T_vec);
}

Real
StiffenedGasTwoPhaseFluidProperties::p_critical() const
{
  return _p_c;
}

Real
StiffenedGasTwoPhaseFluidProperties::T_triple() const
{
  return _T_triple;
}

Real
StiffenedGasTwoPhaseFluidProperties::L_fusion() const
{
  return _L_fusion;
}

Real
StiffenedGasTwoPhaseFluidProperties::T_sat(Real pressure) const
{
  return _ipol_temp.sample(pressure);
}

Real
StiffenedGasTwoPhaseFluidProperties::p_sat(Real temperature) const
{
  return _ipol_pressure.sample(temperature);
}

Real
StiffenedGasTwoPhaseFluidProperties::compute_p_sat(const Real & T) const
{
  Real p_sat = _p_sat_guess;
  Real f_norm = 1.0e5;
  unsigned int iter = 1;
  while (std::fabs(f_norm / p_sat) > _newton_tol)
  {
    // check for maximum iteration
    if (iter > _newton_max_iter)
      mooseError("StiffenedGasTwoPhaseFluidProperties::compute_p_sat: ",
                 "Newton solve did not converge after ",
                 _newton_max_iter,
                 " iterations.");

    // evaluate nonlinear residual and its derivative w.r.t. p_sat
    Real f = std::log(p_sat + _p_inf_vapor) - _A - _B / T - _C * std::log(T) -
             _D * std::log(p_sat + _p_inf_liquid);
    Real f_prime = 1.0 / (p_sat + _p_inf_vapor) - _D / (p_sat + _p_inf_liquid);

    // take Newton step
    f_norm = f / f_prime;
    p_sat -= f_norm;

    iter++;
  }
  return p_sat;
}

Real
StiffenedGasTwoPhaseFluidProperties::dT_sat_dp(Real pressure) const
{
  return _ipol_temp.sampleDerivative(pressure);
}

Real
StiffenedGasTwoPhaseFluidProperties::sigma_from_T(Real T) const
{
  const Real aux = 1.0 - T / _T_c;
  return _sigma_A * std::pow(aux, _sigma_B) * (1.0 - _sigma_C * aux);
}

Real
StiffenedGasTwoPhaseFluidProperties::dsigma_dT_from_T(Real T) const
{
  const Real aux = 1.0 - T / _T_c;
  const Real daux_dT = -1.0 / _T_c;
  const Real dsigma_daux =
      _sigma_A * (_sigma_B * std::pow(aux, _sigma_B - 1.0) * (1.0 - _sigma_C * aux) -
                  _sigma_C * std::pow(aux, _sigma_B));
  return dsigma_daux * daux_dT;
}
