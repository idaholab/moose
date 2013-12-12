//  This post processor returns the mobility (density*relperm)
//
#include "RichardsMobility.h"

template<>
InputParameters validParams<RichardsMobility>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("pressure_vars", "List of variables that represent the pressure");
  params.addRequiredParam<int>("p_num", "Which of these pressure variables to use in the density (ie, which phase you want the mobility for)");
  params.addRequiredParam<UserObjectName>("density_UO", "Name of user object that defines fluid density.");
  params.addRequiredParam<UserObjectName>("seff_UO", "Name of user object that defines effective saturation.");
  params.addRequiredParam<UserObjectName>("relperm_UO", "Name of user object that defines relative permeability");
  params.addClassDescription("User object that defines the mobility (=density*relperm) for a phase");
  return params;
}

RichardsMobility::RichardsMobility(const std::string & name, InputParameters parameters) :
  AuxKernel(name, parameters),
  _p_num(getParam<int>("p_num")),
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
  if (_p_num < 0 || _p_num >= n)
    mooseError("You asked for the mobility for the " << _p_num << " variable.  But there are only " << n << " pressure variables");
}

Real
RichardsMobility::computeValue()
{
  Real density = _density_UO.density((*_pressure_vals[_p_num])[_qp]);

  Real seff = _seff_UO.seff(_pressure_vals, _qp);

  Real relperm = _relperm_UO.relperm(seff);

  return density*relperm;
}
