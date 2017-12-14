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

  params.addClassDescription("Generates an FE representation for a boundary flux"
                             " condition using a 'FunctionSeries'-type Function");

  params.addRequiredParam<std::string>(
      "diffusion_coefficient",
      "The name of the material diffusivity property that will be used in the flux computation.");

  return params;
}

FEBoundaryFluxUserObject::FEBoundaryFluxUserObject(const InputParameters & parameters)
  : FEBoundaryBaseUserObject(parameters),
    _diffusion_coefficient_name(parameters.get<std::string>("diffusion_coefficient")),
    _diffusion_coefficient(getMaterialProperty<Real>(_diffusion_coefficient_name))
{
  // Nothing here
}

Real
FEBoundaryFluxUserObject::computeQpIntegral()
{
  return -_diffusion_coefficient[_qp] * _grad_u[_qp] * _normals[_qp];
}

FEBoundaryFluxUserObject::~FEBoundaryFluxUserObject()
{
  // Nothing here
}
