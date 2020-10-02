//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BndsCalcAux.h"
#include "BndsCalculator.h"

registerMooseObject("PhaseFieldApp", BndsCalcAux);

InputParameters
BndsCalcAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculate location of grain boundaries in a polycrystalline sample");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

BndsCalcAux::BndsCalcAux(const InputParameters & parameters)
  : AuxKernel(parameters), _op_num(coupledComponents("v")), _vals(coupledValues("v"))
{
}

Real
BndsCalcAux::computeValue()
{
  return BndsCalculator::computeBndsVariable(_vals, _qp);
}
