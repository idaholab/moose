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

// Module includes
#include "FEBoundaryFluxUserObject.h"

template <>
InputParameters
validParams<FEBoundaryFluxUserObject>()
{
  InputParameters params = validParams<FEBoundaryBaseUserObject>();

  params.addClassDescription("Generates an FE representation for a boundary flux condition using a "
                             "'FunctionSeries'-type Function");

  params.addRequiredParam<std::string>("diffusivity",
                                       "The name of the material diffusivity "
                                       "property, or raw value, that will be used "
                                       "in the flux computation.");

  return params;
}

FEBoundaryFluxUserObject::FEBoundaryFluxUserObject(const InputParameters & parameters)
  : FEBoundaryBaseUserObject(parameters),
    _diffusivity_name(parameters.get<std::string>("diffusivity")),
    _diffusivity(getMaterialProperty<Real>(_diffusivity_name))
{
  // Nothing here
}

Real
FEBoundaryFluxUserObject::computeQpIntegral()
{
  return -_diffusivity[_qp] * _grad_u[_qp] * _normals[_qp];
}
