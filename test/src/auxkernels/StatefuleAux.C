//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StatefulAux.h"

registerMooseObject("MooseTestApp", StatefulAux);

InputParameters
StatefulAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("coupled", "Coupled Value for Calculation");
  return params;
}

StatefulAux::StatefulAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _coupled(coupled("coupled")),
    _coupled_val_old(coupledValueOld("coupled"))
{
}

Real
StatefulAux::computeValue()
{
  return _coupled_val_old[_qp] * 2;
}
