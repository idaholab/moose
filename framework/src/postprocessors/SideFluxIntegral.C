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

#include "SideFluxIntegral.h"

template<>
InputParameters validParams<SideFluxIntegral>()
{
  InputParameters params = validParams<SidePostprocessor>();
  params.addRequiredParam<std::string>("diffusivity", "The name of the diffusivity material property that will be used in the flux computation.");
  return params;
}

SideFluxIntegral::SideFluxIntegral(const std::string & name,
                                           InputParameters parameters)
  :SideIntegral(name, parameters),
   _diffusivity(getParam<std::string>("diffusivity")),
   _diffusion_coef(getMaterialProperty<Real>(_diffusivity))
{}

Real
SideFluxIntegral::computeQpIntegral()
{
  return -_diffusion_coef[_qp]*_grad_u[_qp]*_normals[_qp];
}
