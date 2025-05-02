//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCoupledOldAux.h"

registerMooseObject("MooseApp", VectorCoupledOldAux);

InputParameters
VectorCoupledOldAux::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();

  params.addClassDescription("OldValueTestAux that return old value.");

  params.addRequiredCoupledVar("v", "Variable vector");

  return params;
}

VectorCoupledOldAux::VectorCoupledOldAux(const InputParameters & parameters)
  : VectorAuxKernel(parameters), _vector(coupledVectorValueOld("v"))
{
}

RealVectorValue
VectorCoupledOldAux::computeValue()
{
  return _vector[_qp];
}
