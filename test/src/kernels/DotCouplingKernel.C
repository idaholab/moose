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

#include "DotCouplingKernel.h"

template<>
InputParameters validParams<DotCouplingKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar("v", "Variable being coupled");
  return params;
}

DotCouplingKernel::DotCouplingKernel(const std::string & name, InputParameters parameters) :
    Kernel(name, parameters),
    _v_dot(coupledDot("v"))
{
}

DotCouplingKernel::~DotCouplingKernel()
{
}

Real
DotCouplingKernel::computeQpResidual()
{
  return _v_dot[_qp];
}
