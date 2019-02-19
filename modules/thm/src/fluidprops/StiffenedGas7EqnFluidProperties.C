#include "StiffenedGas7EqnFluidProperties.h"
#include "StiffenedGasFluidProperties.h"

const Real StiffenedGas7EqnFluidProperties::_P_critical = 22.09E+6;

registerMooseObject("THMApp", StiffenedGas7EqnFluidProperties);

template <>
InputParameters
validParams<StiffenedGas7EqnFluidProperties>()
{
  InputParameters params = validParams<TwoPhaseFluidProperties>();

  // Default parameters for Stiffened Gas EOS (liquid phase)
  params.addParam<Real>("gamma_liquid", 2.35, "");
  params.addParam<Real>("cv_liquid", 1816.0, "");
  params.addParam<Real>("q_liquid", -1.167e6, "");
  params.addParam<Real>("p_inf_liquid", 1.0e9, "");
  params.addParam<Real>("q_prime_liquid", 0, "");
  params.addParam<Real>("k_liquid", 0.5, "");
  params.addParam<Real>("mu_liquid", 281.8e-6, "");

  params.addParam<Real>("gamma_vapor", 1.43, "");
  params.addParam<Real>("cv_vapor", 1040.0, "");
  params.addParam<Real>("q_vapor", 2.03e6, "");
  params.addParam<Real>("p_inf_vapor", 0.0, "");
  params.addParam<Real>("q_prime_vapor", -2.3e4, "");
  params.addParam<Real>("k_vapor", 0.026, "");
  params.addParam<Real>("mu_vapor", 134.4e-7, "");

  return params;
}

StiffenedGas7EqnFluidProperties::StiffenedGas7EqnFluidProperties(const InputParameters & parameters)
  : TwoPhaseFluidProperties(parameters),

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

    _A((_cp_liquid - _cp_vapor + _q_prime_vapor - _q_prime_liquid) / (_cp_vapor - _cv_vapor)),
    _B((_q_liquid - _q_vapor) / (_cp_vapor - _cv_vapor)),
    _C((_cp_vapor - _cp_liquid) / (_cp_vapor - _cv_vapor)),
    _D((_cp_liquid - _cv_liquid) / (_cp_vapor - _cv_vapor)),

    _newton_tol(1.0e-8),
    _newton_max_iter(200)
{
  if (_tid == 0)
  {
    std::string class_name = "StiffenedGasLiquidFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<Real>("gamma") = _gamma_liquid;
    params.set<Real>("cv") = _cv_liquid;
    params.set<Real>("q") = _q_liquid;
    params.set<Real>("p_inf") = _p_inf_liquid;
    params.set<Real>("q_prime") = _q_prime_liquid;
    params.set<Real>("k") = getParam<Real>("k_liquid");
    params.set<Real>("mu") = getParam<Real>("mu_liquid");
    _fe_problem.addUserObject(class_name, _liquid_name, params);
  }
  _fp_liquid = &_fe_problem.getUserObject<SinglePhaseFluidProperties>(_liquid_name, _tid);

  if (_tid == 0)
  {
    std::string class_name = "StiffenedGasFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<Real>("gamma") = _gamma_vapor;
    params.set<Real>("cv") = _cv_vapor;
    params.set<Real>("q") = _q_vapor;
    params.set<Real>("p_inf") = _p_inf_vapor;
    params.set<Real>("q_prime") = _q_prime_vapor;
    params.set<Real>("k") = getParam<Real>("k_vapor");
    params.set<Real>("mu") = getParam<Real>("mu_vapor");
    _fe_problem.addUserObject(class_name, _vapor_name, params);
  }
  _fp_vapor = &_fe_problem.getUserObject<SinglePhaseFluidProperties>(_vapor_name, _tid);

  // calculate the saturation curve p(T) and store the data in two vectors for re-use
  {
    // sample temperatures from freezing point to critical point, in increments of 1 K
    const Real T_min = 0.0 + 274.0;   // slightly above freezing point
    const Real T_max = 374.0 + 273.0; // slightly below critical point
    const Real dT = 1.0;
    const unsigned int n_samples = std::round((T_max - T_min) / dT) + 1;
    _T_vec.resize(n_samples);
    _p_sat_vec.resize(n_samples);

    // loop over sample temperatures, starting with the minimum
    Real T = T_min;
    for (unsigned int i = 0; i < n_samples; i++)
    {
      _T_vec[i] = T;
      _p_sat_vec[i] = compute_p_sat(T);

      // increment sample temperature
      T += dT;
    }
  }

  _ipol_pressure.setData(_T_vec, _p_sat_vec);
  _ipol_temp.setData(_p_sat_vec, _T_vec);
}

Real
StiffenedGas7EqnFluidProperties::p_critical() const
{
  return _P_critical;
}

Real
StiffenedGas7EqnFluidProperties::T_sat(Real pressure) const
{
  return _ipol_temp.sample(pressure);
}

Real
StiffenedGas7EqnFluidProperties::p_sat(Real temperature) const
{
  return _ipol_pressure.sample(temperature);
}

Real
StiffenedGas7EqnFluidProperties::compute_p_sat(const Real & T) const
{
  // start from 0 C saturation pressure; starting from large value sometimes diverges
  Real p_sat = 611.0;
  Real f_norm = 1.0e5;
  unsigned int iter = 1;
  while (std::fabs(f_norm / p_sat) > _newton_tol)
  {
    // check for maximum iteration
    if (iter > _newton_max_iter)
      mooseError("StiffenedGas7EqnFluidProperties::compute_p_sat: ",
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
StiffenedGas7EqnFluidProperties::dT_sat_dp(Real pressure) const
{
  return _ipol_temp.sampleDerivative(pressure);
}
