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

#include "FluxJumpIndicator.h"

template<>
InputParameters validParams<FluxJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  params.addRequiredParam<std::string>("property", "The name of the material property to used as the 'diffusivity'");
  return params;
}


FluxJumpIndicator::FluxJumpIndicator(const std::string & name, InputParameters parameters) :
    JumpIndicator(name, parameters),
    _property_name(parameters.get<std::string>("property")),
    _property(getMaterialProperty<Real>(_property_name)),
    _property_neighbor(getNeighborMaterialProperty<Real>(_property_name))
{
}


Real
FluxJumpIndicator::computeQpIntegral()
{
  Real jump = (_property[_qp]*_grad_u[_qp] - _property_neighbor[_qp]*_grad_u_neighbor[_qp])*_normals[_qp];

  return jump*jump;
}

