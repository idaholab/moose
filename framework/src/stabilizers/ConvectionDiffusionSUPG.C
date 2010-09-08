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

#include "ConvectionDiffusionSUPG.h"

#include "quadrature_gauss.h"

template<>
InputParameters validParams<ConvectionDiffusionSUPG>()
{
  InputParameters params = validParams<Stabilizer>();
  params.addRequiredParam<Real>("coef", "The diffusion coefficient.");
  params.addRequiredParam<Real>("x", "Component of velocity in the x direction");
  params.addParam<Real>("y", 0.0, "Component of velocity in the y direction");
  params.addParam<Real>("z", 0.0, "Component of velocity in the z direction");
  return params;
}

ConvectionDiffusionSUPG::ConvectionDiffusionSUPG(const std::string & name,
                                                 MooseSystem & moose_system,
                                                 InputParameters parameters)
  :SUPGBase(name, moose_system, parameters),
   _coef(getParam<Real>("coef")),
   _x(getParam<Real>("x")),
   _y(getParam<Real>("y")),
   _z(getParam<Real>("z"))
{
  _my_velocity(0)=_x;
  _my_velocity(1)=_y;
  _my_velocity(2)=_z;

  _vel_mag = _my_velocity.size();
}

void
ConvectionDiffusionSUPG::computeTausAndVelocities()
{
  Real h = _current_elem->hmin();
  
  Real pec = (_vel_mag*h) / (2.0*_coef);

  Real tau = (h/(2.0*_vel_mag))*(coth(pec - (1.0/pec)));

  if(tau != tau)
    return;

  unsigned int num_q_points = _qrule->n_points();

  for(unsigned int qp=0; qp<num_q_points; qp++)
  {
    _tau[qp] = tau;
    _velocity[qp] = _my_velocity;
  }
}

Real
ConvectionDiffusionSUPG::coth(Real x)
{
  Real exp_cache = exp(2.0*x);
  return (exp_cache - 1) / (exp_cache + 1);
}
