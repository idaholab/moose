//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseError.h"
#include "SolutionAuxMisorientationBoundary.h"
#include "SolutionUserObject.h"
#include "BndsCalculator.h"

registerMooseObject("PhaseFieldApp", SolutionAuxMisorientationBoundary);

InputParameters
SolutionAuxMisorientationBoundary::validParams()
{
  InputParameters params = SolutionAux::validParams();
  params.addClassDescription(
      "Calculate location of grain boundaries by using information from a SolutionUserObject.");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addRequiredParam<Real>("gb_type_order",
                                "The grain boundary type to calculate bnds parameter");
  return params;
}

SolutionAuxMisorientationBoundary::SolutionAuxMisorientationBoundary(
    const InputParameters & parameters)
  : SolutionAux(parameters),
    _gb_type_order(getParam<Real>("gb_type_order")),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v"))
{
}

Real
SolutionAuxMisorientationBoundary::computeValue()
{
  // The value to output
  Real output_gb_type;

  // _direct=true, extract the values using the dof
  if (_direct)
  {
    if (isNodal())
      output_gb_type = _solution_object.directValue(_current_node, _var_name);
    else
      output_gb_type = _solution_object.directValue(_current_elem, _var_name);
  }
  // _direct=false, extract the values using time and point
  else
  {
    if (isNodal())
      output_gb_type = _solution_object.pointValue(_t, *_current_node, _var_name);
    else
      output_gb_type = _solution_object.pointValue(_t, _current_elem->vertex_average(), _var_name);
  }

  // generate different GB boundary parameters
  auto gb_type_shift = abs(output_gb_type - _gb_type_order);
  if (abs(output_gb_type - 0) < 1e-3)
    return 1.0;
  else if (gb_type_shift < 1e-3)
    return BndsCalculator::computeBndsVariable(_vals, _qp);
  else
    return gb_type_shift + (1 - gb_type_shift) * BndsCalculator::computeBndsVariable(_vals, _qp);
}
