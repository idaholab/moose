/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MatDiffusion.h"

template<>
InputParameters validParams<MatDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  params.addClassDescription("Diffusion equation Kernel that takes teh Diffusivity from a material property");
  params.addParam<MaterialPropertyName>("D_name", "D", "The name of the diffusivity");
  return params;
}

MatDiffusion::MatDiffusion(const InputParameters & parameters) :
    Diffusion(parameters),
    _D(getMaterialProperty<Real>("D_name"))
{}

Real
MatDiffusion::computeQpResidual()
{
  return _D[_qp] * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
MatDiffusion::computeQpJacobian()
{
  return _D[_qp] * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}


// DEPRECATED CONSTRUCTOR
MatDiffusion::MatDiffusion(const std::string & deprecated_name, InputParameters parameters) :
    Diffusion(deprecated_name, parameters),
    _D(getMaterialProperty<Real>("D_name"))
{}
