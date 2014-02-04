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

#include "FDDiffusion.h"


template<>
InputParameters validParams<FDDiffusion>()
{
  InputParameters p = validParams<FDKernel>();
  return p;
}


FDDiffusion::FDDiffusion(const std::string & name, InputParameters parameters) :
    FDKernel(name, parameters)
{

}

FDDiffusion::~FDDiffusion()
{

}

Real
FDDiffusion::computeQpResidual()
{
  return _grad_u[_qp] * _grad_test[_i][_qp];
}

