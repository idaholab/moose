//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxKernel.h"

template <>
InputParameters
validParams<AuxKernel>()
{
  InputParameters params = auxKernelBaseValidParams();
  params.registerBase("AuxKernel");
  return params;
}

AuxKernel::AuxKernel(const InputParameters & parameters)
  : AuxKernelBase<Real>(parameters),
    _u(_nodal ? _var.dofValues() : _var.sln()),
    _u_old(_nodal ? _var.dofValuesOld() : _var.slnOld()),
    _u_older(_nodal ? _var.dofValuesOlder() : _var.slnOlder())
{
}
