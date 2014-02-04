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

#include "FDAdvection.h"

template<>
InputParameters validParams<FDAdvection>()
{
  InputParameters params = validParams<FDKernel>();

  params.addRequiredCoupledVar("advector", "The gradient of this variable will be used as the velocity vector.");
  return params;
}

FDAdvection::FDAdvection(const std::string & name,
                       InputParameters parameters) :
    FDKernel(name, parameters),
    _grad_advector(coupledGradient("advector"))
{}

Real FDAdvection::computeQpResidual()
{
  return _test[_i][_qp]*(_grad_advector[_qp]*_grad_u[_qp]);
}


