//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CopyValueAux.h"

registerMooseObject("ThermalHydraulicsApp", CopyValueAux);

InputParameters
CopyValueAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("source", "Source variable to be copied.");

  return params;
}

CopyValueAux::CopyValueAux(const InputParameters & parameters)
  : AuxKernel(parameters), _source_var(coupledValue("source"))
{
}

Real
CopyValueAux::computeValue()
{
  return _source_var[_qp];
}
