//  This post processor returns the derivative of mass wrt to pressure in a region.
//
#include "RichardsMassDensityPrime.h"

template<>
InputParameters validParams<RichardsMassDensityPrime>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addRequiredParam<int>("p_num", "Which of the pressure variables to use in the density (ie, which phase to calculate mass for)");
  params.addRequiredParam<int>("dp_num", "Which of the pressure variables to take the derivative with respect to");
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("sat_UO", "Name of user object that defines saturation");
  params.addClassDescription("auxillary variable which is d(density*saturation)/dP");
  return params;
}

RichardsMassDensityPrime::RichardsMassDensityPrime(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _p_num(getParam<int>("p_num")),
  _dp_num(getParam<int>("dp_num")),
  _density_UO(getUserObject<RichardsDensity>("density_UO")),
  _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
  _sat_UO(getUserObject<RichardsSat>("sat_UO"))
{
  int n = coupledComponents("pressure_vars");
  _pressure_vars.resize(n);
  _pressure_vals.resize(n);

  for (int i=0 ; i<n; ++i)
    {
      _pressure_vars[i] = coupled("pressure_vars", i);
      _pressure_vals[i] = &coupledValue("pressure_vars", i);
    }
  if (_p_num < 0 || _p_num >= n || _dp_num < 0 || _dp_num >= n)
      mooseError("You asked for the derivative of the " << _p_num << " mass density wrt the " << _dp_num << " variable.  But there are only " << n << " pressure variables");
}



Real
RichardsMassDensityPrime::computeValue()
{
  Real density = _density_UO.density((*_pressure_vals[_p_num])[_qp]);
  Real ddensity_dp = _density_UO.ddensity((*_pressure_vals[_p_num])[_qp]);

  Real seff = _seff_UO.seff(_pressure_vals, _qp);
  std::vector<Real> dseff_dp = _seff_UO.dseff(_pressure_vals, _qp);

  Real sat = _sat_UO.sat(seff);
  Real dsat_dseff = _sat_UO.dsat(seff);

  std::vector<Real> dmass_dp(dseff_dp.size(), 0);
  dmass_dp[_p_num] = ddensity_dp*sat;

  for (unsigned int i=0 ; i<dseff_dp.size() ; ++i)
    {
      dmass_dp[i] += density*dsat_dseff*dseff_dp[i];
    }

  return dmass_dp[_dp_num];
}
