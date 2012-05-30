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

#include "Convection.h"

template<>
InputParameters validParams<Convection>()
{
  InputParameters params = validParams<Kernel>();
  return params;
}

Convection::Convection(const std::string & name,
                       InputParameters parameters) :
    Kernel(name, parameters),

    // Retrieve a gradient material property to use for the convection
    // velocity
    _velocity(getMaterialProperty<RealGradient>("convection_velocity"))
{}

Real Convection::computeQpResidual()
{
  return _test[_i][_qp]*(_velocity[_qp]*_grad_u[_qp]);
}

Real Convection::computeQpJacobian()
{
  return _test[_i][_qp]*(_velocity[_qp]*_grad_phi[_j][_qp]);
}
