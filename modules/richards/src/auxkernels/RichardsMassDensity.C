//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "RichardsMassDensity.h"

template<>
InputParameters validParams<RichardsMassDensity>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addRequiredParam<int>("p_num", "Which of the pressure variables to use in the density (ie, which phase to calculate mass for)");
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("sat_UO", "Name of user object that defines saturation");
  return params;
}

RichardsMassDensity::RichardsMassDensity(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _p_num(getParam<int>("p_num")),
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
  if (_p_num < 0 || _p_num >= n)
      mooseError("You asked for the " << _p_num << " mass density.  But there are only " << n << " pressure variables");
}



Real
RichardsMassDensity::computeValue()
{
  Real density = _density_UO.density((*_pressure_vals[_p_num])[_qp]);

  Real seff = _seff_UO.seff(_pressure_vals, _qp);

  Real sat = _sat_UO.sat(seff);

  return density*sat;
}
