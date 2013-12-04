//  saturation as a function of effective saturation, and its derivs wrt effective saturation
//
#include "RichardsSat.h"

template<>
InputParameters validParams<RichardsSat>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("User object yielding saturation as a function of effective saturation");
  params.addRequiredParam<Real>("s_res", "Residual fluid saturation.  Should be between 0 and 1.  Eg, 0.1");
  params.addRequiredParam<Real>("s_res_air", "Residual air saturation.  Should be between 0 and 1 - s_res.  Eg, 0.0");
  return params;
}

RichardsSat::RichardsSat(const std::string & name, InputParameters parameters) :
  GeneralUserObject(name, parameters),
  _s_res(getParam<Real>("s_res")),
  _s_res_air(getParam<Real>("s_res_air"))
{
  if (_s_res < 0 || _s_res > 1)
    mooseError("Residual saturation set to " << _s_res << " but it must not be less than zero or greater than 1");
  if (_s_res_air < 0 || _s_res_air > 1)
    mooseError("Residual air saturation set to " << _s_res_air << " but it must not be less than zero or greater than 1");
  if (_s_res >= 1 - _s_res_air)
    mooseError("Residual saturations set to " << _s_res << " and " << _s_res_air << " but they must satisfy S_res < 1 - S_res_air");
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
  return _s_res + seff*(1.0 - _s_res - _s_res_air);
}

Real
RichardsSat::dsat(Real seff) const
{
  return 1.0 - _s_res - _s_res_air;
}

Real
RichardsSat::d2sat(Real seff) const
{
  return 0.0;
}

