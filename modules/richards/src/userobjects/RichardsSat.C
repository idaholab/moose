/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

//  saturation as a function of effective saturation, and its derivs wrt effective saturation
//
#include "RichardsSat.h"

template<>
InputParameters validParams<RichardsSat>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("User object yielding saturation as a function of effective saturation");
  params.addRequiredParam<Real>("s_res", "Residual fluid saturation.  0 <= s_res < 1.");
  params.addRequiredParam<Real>("sum_s_res", "Sum of s_res over all phases.  s_res <= sum_s_res < 1.  It is up to you to ensure the sum is done correctly.");
  return params;
}

RichardsSat::RichardsSat(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters),
  _s_res(getParam<Real>("s_res")),
  _sum_s_res(getParam<Real>("sum_s_res"))
{
  if (_s_res < 0 || _s_res >= 1)
    mooseError("Residual saturation set to " << _s_res << " but it must obey 0 <= s_res < 1");
  if (_sum_s_res < _s_res || _sum_s_res >= 1)
    mooseError("sum_s_res set to " << _sum_s_res << " but it must obey s_res <= sum_s_res < 1");
}

void
RichardsSat::initialize()
{}

void
RichardsSat::execute()
{}

void RichardsSat::finalize()
{}


Real
RichardsSat::sat(Real seff) const
{
  return _s_res + seff*(1.0 - _sum_s_res);
}

Real
RichardsSat::dsat(Real /*seff*/) const
{
  return 1.0 - _sum_s_res;
}

Real
RichardsSat::d2sat(Real /*seff*/) const
{
  return 0.0;
}

