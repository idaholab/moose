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

#include "ArrayDiffusion.h"


template<>
InputParameters validParams<ArrayDiffusion>()
{
  InputParameters params = validParams<ArrayKernel>();
  params.addClassDescription("The Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

ArrayDiffusion::ArrayDiffusion(const InputParameters & parameters) :
    ArrayKernel(parameters)
{
}

void
ArrayDiffusion::computeQpResidual()
{
  _residual.noalias() = _grad_u[_qp] * _grad_test[_i][_qp];
}

void
ArrayDiffusion::computeQpJacobian()
{
//  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
