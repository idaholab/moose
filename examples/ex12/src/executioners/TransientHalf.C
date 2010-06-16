#include "TransientHalf.h"

//Moose includes

template<>
InputParameters validParams<TransientHalf>()
{
  InputParameters params = validParams<TransientExecutioner>();
  params.addParam<Real>("ratio", 0.5, "The ratio used to calculate the next timestep");
  params.addParam<Real>("min_dt", 0.01, "The smallest timestep we will allow");
  return params;
}

TransientHalf::TransientHalf(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :TransientExecutioner(name, moose_system, parameters),
   _ratio(_parameters.get<Real>("ratio")),
   _min_dt(_parameters.get<Real>("min_dt"))
{}

Real
TransientHalf::computeDT()
{
  /**
   * We won't grow timesteps with this example so if the ratio > 1.0 we'll just
   * return the current dt
   */
  if (_ratio > 1.0)
    return _dt;

  /**
   * Shrink our timestep by the specified ratio or return the min if it's too small
   */
  if (_time == 0.0)
    return _dt;
  else
    return std::max(_dt*_ratio, _min_dt);
}
