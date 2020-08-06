//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BndsCalcIC.h"
#include "BndsCalculator.h"

registerMooseObject("PhaseFieldApp", BndsCalcIC);

InputParameters
BndsCalcIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "Initialize the location of grain boundaries in a polycrystalline sample");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

BndsCalcIC::BndsCalcIC(const InputParameters & parameters)
  : InitialCondition(parameters), _op_num(coupledComponents("v")), _vals(coupledValues("v"))
{
}

Real
BndsCalcIC::value(const Point & /*p*/)
{
  return BndsCalculator::computeBndsVariable(_vals, _qp);
}
