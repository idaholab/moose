/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ExplicitCriticalDT.h"

// Moose includes

template <>
InputParameters
validParams<ExplicitCriticalDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("dt", 1., "The initial time step size.");
  params.addParam<Real>("mesh_size", 1.0, "size of smallest element");
  params.addParam<Real>("min_dt", 0.01, "The smallest timestep we will allow");
  params.addParam<Real>("bulk_modulus", 0.0, "Material property name with Bulk Modulus");
  params.addParam<Real>("youngs_modulus", 0.0, "Material property name with Young's Modulus");
  params.addParam<Real>("poissons_ratio", 0.0, "Material property name with Possion's ratio");
  params.addParam<Real>("density", 0.0, "Material property name with density");
  params.addParam<Real>("wave_speed", 0.0, "Material property name with wave speed");

  return params;
}

ExplicitCriticalDT::ExplicitCriticalDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _bulk_mod(getParam<Real>("bulk_modulus")),
    _Emod(getParam<Real>("youngs_modulus")),
    _vratio(getParam<Real>("poissons_ratio")),
    _rho(getParam<Real>("density")),
    _cwave(getParam<Real>("wave_speed")),
    _mesh_size(getParam<Real>("mesh_size")),
    _min_dt(getParam<Real>("min_dt"))

{
}

Real
ExplicitCriticalDT::computeInitialDT()
{
  return getParam<Real>("dt");
}

Real
ExplicitCriticalDT::computeDT()
{
  /**
   * We won't grow timesteps with this example so if the ratio > 1.0 we'll just
   * leave current_dt alone.
   */
  Real cspeed = 0.0;

  if (_cwave > 0.0)
    cspeed = _cwave;
  else if (_bulk_mod > 0.0)
    cspeed = std::sqrt(_bulk_mod / _rho);
  else
    cspeed = std::sqrt(_Emod * (1 - _vratio) / (_rho * (1 + _vratio) * (1 - 2 * _vratio)));

  Real _t_crit = _mesh_size / cspeed;

  if (_t_crit < getCurrentDT())
    return std::max(_t_crit, _min_dt);
  // Shrink our timestep to satisfy the critical timestep
  else
    return getCurrentDT();
}
