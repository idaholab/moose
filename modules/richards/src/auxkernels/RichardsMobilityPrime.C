//  This post processor returns the mobility d(density*relperm)/dP
//
#include "RichardsMobilityPrime.h"

template<>
InputParameters validParams<RichardsMobilityPrime>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addRequiredParam<int>("p_num", "Which of these pressure variables to use in the density (ie, which phase you want the mobility for)");
  params.addRequiredParam<int>("dp_num", "Which of these pressure variables you want to take the derivative with respect to");
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("relperm_UO", "Name of user object that defines relative permeability");
  params.addClassDescription("auxillary variable which is d(density*relative_permeability)/dP");
  return params;
}

RichardsMobilityPrime::RichardsMobilityPrime(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _p_num(getParam<int>("p_num")),
  _dp_num(getParam<int>("dp_num")),
  _density_UO(getUserObject<RichardsDensity>("density_UO")),
  _seff_UO(getUserObject<RichardsSeff>("seff_UO")),
  _relperm_UO(getUserObject<RichardsRelPerm>("relperm_UO"))
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
      mooseError("You asked for the derivative of the " << _p_num << " mobility wrt the " << _dp_num << " variable.  But there are only " << n << " pressure variables");
}


Real
RichardsMobilityPrime::computeValue()
{
  Real density = _density_UO.density((*_pressure_vals[_p_num])[_qp]);
  Real ddensity_dp = _density_UO.ddensity((*_pressure_vals[_p_num])[_qp]);

  Real seff = _seff_UO.seff(_pressure_vals, _qp);
  std::vector<Real> dseff_dp = _seff_UO.dseff(_pressure_vals, _qp); 

  Real relperm = _relperm_UO.relperm(seff);
  Real drelperm_dseff = _relperm_UO.drelperm(seff);

  std::vector<Real> dmob_dp(dseff_dp.size());
  dmob_dp[_p_num] = ddensity_dp*relperm;
  for (unsigned int i=0 ; i<dmob_dp.size(); ++i)
    {
      dmob_dp[i] += density*drelperm_dseff*dseff_dp[i];
    }

  return dmob_dp[_dp_num];
}
